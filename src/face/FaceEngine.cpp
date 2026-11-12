#include "FaceEngine.h"

#include <QBuffer>
#include <QFile>
#include <QDebug>
#include <QImage>
#include <QCoreApplication>
#include <cmath>
#include <cstring>

#ifdef HAVE_SEETA_FACE6
#include <seeta/FaceDetector.h>
#include <seeta/FaceLandmarker.h>
#include <seeta/FaceRecognizer.h>
#include <seeta/Common/CStruct.h>
#include <seeta/CFaceInfo.h>
#endif

FaceEngine::FaceEngine()
    : m_initialized(false)
    , m_featureSize(0)
    , m_faceDetector(nullptr)
    , m_faceLandmarker(nullptr)
    , m_faceRecognizer(nullptr)
{
}

FaceEngine::~FaceEngine()
{
#ifdef HAVE_SEETA_FACE6
    delete static_cast<seeta::FaceDetector   *>(m_faceDetector);
    delete static_cast<seeta::FaceLandmarker *>(m_faceLandmarker);
    delete static_cast<seeta::FaceRecognizer *>(m_faceRecognizer);
#endif
}

bool FaceEngine::initialize(const QString &modelDir)
{
    QString resolvedDir = modelDir;
    if (resolvedDir.isEmpty()) {
        resolvedDir = QCoreApplication::applicationDirPath();
    }

    // Search candidate directories for model files.
    const QStringList candidateDirs = {
        resolvedDir,
        resolvedDir + "/sf6.0_windows",
        QCoreApplication::applicationDirPath() + "/sf6.0_windows",
        QCoreApplication::applicationDirPath() + "/../sf6.0_windows",
    };

    QString detectorPath;
    QString landmarkerPath;
    QString recognizerPath;
    for (const QString &dir : candidateDirs) {
        const QString d = dir + "/face_detector.csta";
        const QString l = dir + "/face_landmarker_pts5.csta";
        const QString r = dir + "/face_recognizer.csta";
        if (QFile::exists(d) && QFile::exists(l) && QFile::exists(r)) {
            detectorPath = d;
            landmarkerPath = l;
            recognizerPath = r;
            resolvedDir = dir;
            break;
        }
    }

    if (detectorPath.isEmpty()) {
        detectorPath = resolvedDir + "/face_detector.csta";
        landmarkerPath = resolvedDir + "/face_landmarker_pts5.csta";
        recognizerPath = resolvedDir + "/face_recognizer.csta";
    }

    if (!QFile::exists(detectorPath) || !QFile::exists(landmarkerPath) || !QFile::exists(recognizerPath)) {
        m_lastError = QString("缺少 SeetaFace6 模型文件，期待：%1、%2、%3")
                          .arg(detectorPath, landmarkerPath, recognizerPath);
        qWarning() << m_lastError;
        return false;
    }

#ifdef HAVE_SEETA_FACE6
    // Use seeta::ModelSetting (C++ class that properly manages model path strings).
    seeta::ModelSetting detSetting;
    detSetting.append(detectorPath.toStdString());
    detSetting.set_device(seeta::ModelSetting::CPU);

    seeta::ModelSetting lanSetting;
    lanSetting.append(landmarkerPath.toStdString());
    lanSetting.set_device(seeta::ModelSetting::CPU);

    seeta::ModelSetting recSetting;
    recSetting.append(recognizerPath.toStdString());
    recSetting.set_device(seeta::ModelSetting::CPU);

    auto *fd = new seeta::FaceDetector(detSetting);
    auto *fl = new seeta::FaceLandmarker(lanSetting);
    auto *fr = new seeta::FaceRecognizer(recSetting);

    m_faceDetector   = fd;
    m_faceLandmarker = fl;
    m_faceRecognizer = fr;
    m_featureSize    = fr->GetExtractFeatureSize();

    m_modelDir = resolvedDir;
    m_initialized = true;
    m_lastError.clear();
    qInfo() << "SeetaFace6 initialized successfully. Feature size:" << m_featureSize;
#else
    Q_UNUSED(detectorPath)
    Q_UNUSED(landmarkerPath)
    Q_UNUSED(recognizerPath)
    m_modelDir = resolvedDir;
    m_initialized = true;
    m_featureSize = 1024;
    qWarning() << "SeetaFace6 SDK not linked; running in placeholder mode.";
#endif
    return true;
}

bool FaceEngine::isInitialized() const
{
    return m_initialized;
}

QByteArray FaceEngine::imageToRGB888(const QImage &image)
{
    const QImage rgb = image.convertToFormat(QImage::Format_RGB888);
    QByteArray raw(rgb.width() * rgb.height() * 3, '\0');
    for (int y = 0; y < rgb.height(); ++y) {
        std::memcpy(raw.data() + y * rgb.bytesPerLine(),
                    rgb.constScanLine(y),
                    static_cast<size_t>(rgb.width()) * 3);
    }
    return raw;
}

bool FaceEngine::detect(const QImage &image, QRect &faceRect)
{
    faceRect = QRect();
    if (!m_initialized) {
        m_lastError = "SeetaFace6 模型尚未初始化";
        return false;
    }
    if (image.isNull()) {
        m_lastError = "输入图像为空";
        return false;
    }

#ifdef HAVE_SEETA_FACE6
    const QByteArray raw = imageToRGB888(image);
    SeetaImageData seetaImage;
    seetaImage.width    = image.width();
    seetaImage.height   = image.height();
    seetaImage.channels = 3;
    seetaImage.data     = const_cast<unsigned char *>(reinterpret_cast<const unsigned char *>(raw.constData()));

    auto *fd = static_cast<seeta::FaceDetector *>(m_faceDetector);
    const SeetaFaceInfoArray faces = fd->detect(seetaImage);
    if (faces.size == 0) {
        m_lastError = "未检测到人脸";
        return false;
    }

    int bestIdx = 0;
    for (int i = 1; i < faces.size; ++i) {
        if (faces.data[i].score > faces.data[bestIdx].score) {
            bestIdx = i;
        }
    }

    const SeetaRect &r = faces.data[bestIdx].pos;
    faceRect = QRect(r.x, r.y, r.width, r.height);
    return true;
#else
    Q_UNUSED(image)
    const int w = image.width();
    const int h = image.height();
    faceRect = QRect(w / 5, h / 5, w * 3 / 5, h * 3 / 5);
    return true;
#endif
}

QByteArray FaceEngine::extractFeature(const QImage &image)
{
    if (!m_initialized) {
        m_lastError = "SeetaFace6 模型尚未初始化";
        return {};
    }

#ifdef HAVE_SEETA_FACE6
    QRect faceRect;
    if (!detect(image, faceRect)) {
        return {};
    }

    const QByteArray raw = imageToRGB888(image);
    SeetaImageData seetaImage;
    seetaImage.width    = image.width();
    seetaImage.height   = image.height();
    seetaImage.channels = 3;
    seetaImage.data     = const_cast<unsigned char *>(reinterpret_cast<const unsigned char *>(raw.constData()));

    SeetaRect rc;
    rc.x      = faceRect.x();
    rc.y      = faceRect.y();
    rc.width  = faceRect.width();
    rc.height = faceRect.height();

    auto *fl = static_cast<seeta::FaceLandmarker *>(m_faceLandmarker);
    const int pointCount = fl->number();
    std::vector<SeetaPointF> points(static_cast<size_t>(pointCount));
    fl->mark(seetaImage, rc, points.data());

    auto *fr = static_cast<seeta::FaceRecognizer *>(m_faceRecognizer);
    std::vector<float> features(static_cast<size_t>(m_featureSize));
    fr->Extract(seetaImage, points.data(), features.data());

    const auto byteSize = static_cast<int>(features.size() * sizeof(float));
    return QByteArray(reinterpret_cast<const char *>(features.data()), byteSize);
#else
    Q_UNUSED(image)
    const int floatCount = m_featureSize > 0 ? m_featureSize : 1024;
    const int byteSize   = floatCount * static_cast<int>(sizeof(float));
    QByteArray bytes(byteSize, '\0');
    float *features = reinterpret_cast<float *>(bytes.data());
    const quint64 seed = static_cast<quint64>(image.width()) * 73856093u
                       ^ static_cast<quint64>(image.height()) * 19349663u;
    for (int i = 0; i < floatCount; ++i) {
        features[i] = static_cast<float>((seed + static_cast<quint64>(i) * 83492791u) % 1000) / 1000.0f;
    }
    return bytes;
#endif
}

FaceRecognitionResult FaceEngine::recognize(const QImage &image, const QList<Student> &students, float threshold)
{
    FaceRecognitionResult result;
    if (!m_initialized) {
        result.message = "SeetaFace6 模型尚未初始化";
        return result;
    }
    if (students.isEmpty()) {
        result.message = "数据库中没有可比对的学生";
        return result;
    }

    const QByteArray currentFeature = extractFeature(image);
    if (currentFeature.isEmpty()) {
        result.message = "当前帧特征提取失败：" + m_lastError;
        return result;
    }

#ifdef HAVE_SEETA_FACE6
    const float *current = reinterpret_cast<const float *>(currentFeature.constData());
    auto *fr = static_cast<seeta::FaceRecognizer *>(m_faceRecognizer);
    for (const Student &student : students) {
        if (student.faceFeatureSize() <= 0 || student.faceFeature().isEmpty()) {
            continue;
        }
        const int expectedBytes = m_featureSize * static_cast<int>(sizeof(float));
        if (student.faceFeature().size() != expectedBytes) {
            continue;
        }
        const float *reference = reinterpret_cast<const float *>(student.faceFeature().constData());
        const float similarity = fr->CalculateSimilarity(current, reference);
        if (similarity >= threshold && similarity > result.score) {
            result.matched   = true;
            result.studentNo = student.studentNo();
            result.name      = student.name();
            result.score     = similarity;
        }
    }
#else
    for (const Student &student : students) {
        if (student.faceFeatureSize() <= 0 || student.faceFeature().isEmpty()) {
            continue;
        }
        const float similarity = compareFeatures(currentFeature, student.faceFeature());
        if (similarity >= threshold && similarity > result.score) {
            result.matched   = true;
            result.studentNo = student.studentNo();
            result.name      = student.name();
            result.score     = similarity;
        }
    }
#endif

    if (!result.matched) {
        result.message = "未匹配到已注册学生";
    } else {
        result.message = QString("识别成功，匹配度 %1").arg(static_cast<double>(result.score), 0, 'f', 2);
    }
    return result;
}

float FaceEngine::compareFeatures(const QByteArray &featureA, const QByteArray &featureB)
{
    const int floatSize = static_cast<int>(sizeof(float));
    if (featureA.size() < floatSize || featureB.size() < floatSize) {
        return 0.0f;
    }

    const int countA = featureA.size() / floatSize;
    const int countB = featureB.size() / floatSize;
    const int count  = qMin(countA, countB);

    const float *a = reinterpret_cast<const float *>(featureA.constData());
    const float *b = reinterpret_cast<const float *>(featureB.constData());

#ifdef HAVE_SEETA_FACE6
    auto *fr = static_cast<seeta::FaceRecognizer *>(m_faceRecognizer);
    return fr->CalculateSimilarity(a, b);
#else
    float dot = 0.0f, normA = 0.0f, normB = 0.0f;
    for (int i = 0; i < count; ++i) {
        dot   += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }
    return dot / (std::sqrt(normA) * std::sqrt(normB) + 1e-6f);
#endif
}

QString FaceEngine::lastError() const
{
    return m_lastError;
}
