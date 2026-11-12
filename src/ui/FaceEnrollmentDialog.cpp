#include "FaceEnrollmentDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCameraDevice>
#include <QMediaDevices>
#include <QPixmap>
#include <QApplication>
#include <QDebug>

FaceEnrollmentDialog::FaceEnrollmentDialog(QWidget *parent)
    : QDialog(parent)
    , m_camera(nullptr)
    , m_imageCapture(nullptr)
    , m_captureSession(nullptr)
    , m_captureTimer(nullptr)
    , m_faceEngine(nullptr)
    , m_faceFeatureSize(0)
    , m_cameraStarted(false)
{
    setupUi();
    setWindowTitle("人脸信息录入");
    resize(640, 540);

    // Initialize face engine
    m_faceEngine = new FaceEngine();
    if (!m_faceEngine->initialize(QApplication::applicationDirPath())) {
        m_statusLabel->setText(QString("人脸引擎初始化失败：%1").arg(m_faceEngine->lastError()));
        m_captureButton->setEnabled(false);
    } else {
        m_statusLabel->setText("人脸引擎已就绪，请点击「拍照录入」采集人脸信息。");
    }
}

FaceEnrollmentDialog::~FaceEnrollmentDialog()
{
    stopCamera();
}

void FaceEnrollmentDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    // Preview area
    m_previewLabel = new QLabel("摄像头预览区", this);
    m_previewLabel->setMinimumHeight(300);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setStyleSheet("background:#111; color:white; border:1px solid #555;");
    mainLayout->addWidget(m_previewLabel);

    // Status label
    m_statusLabel = new QLabel("正在初始化摄像头...", this);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet("font-size:14px; margin-top:6px;");
    mainLayout->addWidget(m_statusLabel);

    // Button row
    auto *buttonLayout = new QHBoxLayout();
    m_captureButton = new QPushButton("拍照录入", this);
    m_okButton = new QPushButton("确认录入", this);
    m_okButton->setEnabled(false);
    m_cancelButton = new QPushButton("取消", this);

    buttonLayout->addWidget(m_captureButton);
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_captureButton, &QPushButton::clicked, this, &FaceEnrollmentDialog::onCapture);
    connect(m_okButton, &QPushButton::clicked, this, &FaceEnrollmentDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void FaceEnrollmentDialog::startCamera()
{
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        m_statusLabel->setText("未检测到可用摄像头，无法进行人脸录入。");
        m_captureButton->setEnabled(false);
        return;
    }

    m_camera = new QCamera(cameras.first(), this);
    m_imageCapture = new QImageCapture(this);
    m_captureSession = new QMediaCaptureSession(this);
    m_captureSession->setCamera(m_camera);
    m_captureSession->setImageCapture(m_imageCapture);

    connect(m_imageCapture, &QImageCapture::imageCaptured,
            this, [this](int, const QImage &preview) {
                m_latestFrame = preview;
                if (!preview.isNull()) {
                    m_previewLabel->setPixmap(QPixmap::fromImage(
                        preview.scaled(m_previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
                }
            });

    m_camera->start();
    m_cameraStarted = true;
    m_previewLabel->setText("摄像头已启动，等待画面...");
}

void FaceEnrollmentDialog::stopCamera()
{
    if (m_captureTimer) {
        m_captureTimer->stop();
    }
    if (m_camera) {
        m_camera->stop();
    }
    if (m_captureSession) {
        m_captureSession->setCamera(nullptr);
        m_captureSession->setImageCapture(nullptr);
    }
    m_cameraStarted = false;
}

void FaceEnrollmentDialog::onCapture()
{
    if (!m_cameraStarted) {
        startCamera();
        // Give the camera a moment to start producing frames before the first capture
        QTimer::singleShot(800, this, &FaceEnrollmentDialog::onCapture);
        return;
    }

    if (!m_imageCapture) {
        m_statusLabel->setText("摄像头未就绪，请重试。");
        return;
    }

    // Trigger a single capture
    m_imageCapture->capture();

    // Process the captured frame after a short delay to allow the frame to arrive
    QTimer::singleShot(300, this, [this]() {
        if (m_latestFrame.isNull()) {
            m_statusLabel->setText("未能获取摄像头画面，请确认摄像头工作正常后重试。");
            return;
        }

        if (!m_faceEngine || !m_faceEngine->isInitialized()) {
            m_statusLabel->setText("人脸引擎未初始化，无法提取特征。");
            return;
        }

        // Detect face
        QRect faceRect;
        if (!m_faceEngine->detect(m_latestFrame, faceRect)) {
            m_statusLabel->setText(QString("未检测到人脸，请正对摄像头后重新拍照。(%1)")
                                       .arg(m_faceEngine->lastError()));
            m_okButton->setEnabled(false);
            return;
        }

        // Extract face feature
        m_faceFeature = m_faceEngine->extractFeature(m_latestFrame);
        if (m_faceFeature.isEmpty()) {
            m_statusLabel->setText(QString("人脸特征提取失败：%1").arg(m_faceEngine->lastError()));
            m_okButton->setEnabled(false);
            return;
        }

        m_faceFeatureSize = m_faceFeature.size();
        m_statusLabel->setText(QString("人脸特征提取成功！特征数据大小：%1 字节。"
                                       "请点击「确认录入」保存。")
                                   .arg(m_faceFeatureSize));
        m_statusLabel->setStyleSheet("font-size:14px; margin-top:6px; color:green;");
        m_okButton->setEnabled(true);
    });
}

void FaceEnrollmentDialog::onAccept()
{
    if (m_faceFeature.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先拍照录入人脸信息后再确认。");
        return;
    }
    accept();
}

QByteArray FaceEnrollmentDialog::faceFeature() const
{
    return m_faceFeature;
}

int FaceEnrollmentDialog::faceFeatureSize() const
{
    return m_faceFeatureSize;
}
