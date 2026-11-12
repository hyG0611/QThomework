#include "FaceAttendanceWidget.h"

#include <QMessageBox>
#include <QDateTime>
#include <QPixmap>
#include <QCameraDevice>
#include <QMediaDevices>
#include <QDebug>
#include <QApplication>
#include <algorithm>

FaceAttendanceWidget::FaceAttendanceWidget(QWidget *parent)
    : QWidget(parent),
      camera(nullptr),
      imageCapture(nullptr),
      captureSession(nullptr),
      captureTimer(nullptr),
      faceEngine(nullptr),
      isRunning(false)
{
    setupUi();
    loadStudents();
}

FaceAttendanceWidget::~FaceAttendanceWidget()
{
    if (captureTimer) {
        captureTimer->stop();
    }
    if (camera) {
        camera->stop();
    }
    if (captureSession) {
        captureSession->setCamera(nullptr);
        captureSession->setImageCapture(nullptr);
    }
}

void FaceAttendanceWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    previewLabel = new QLabel("摄像头预览区", this);
    previewLabel->setMinimumHeight(280);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setStyleSheet("background:#111; color:white; border:1px solid #555;");
    mainLayout->addWidget(previewLabel);

    resultLabel = new QLabel("准备开始识别", this);
    resultLabel->setWordWrap(true);
    resultLabel->setStyleSheet("font-size:16px; margin-top:8px;");
    mainLayout->addWidget(resultLabel);

    auto *buttonLayout = new QHBoxLayout();
    startButton = new QPushButton("开始识别", this);
    stopButton = new QPushButton("停止识别", this);
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    mainLayout->addLayout(buttonLayout);

    connect(startButton, &QPushButton::clicked, this, &FaceAttendanceWidget::onStartCapture);
    connect(stopButton, &QPushButton::clicked, this, &FaceAttendanceWidget::onStopCapture);
}

void FaceAttendanceWidget::loadStudents()
{
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        resultLabel->setText(QString("数据库初始化失败：%1").arg(db.lastErrorText().isEmpty() ? "数据库不可用" : db.lastErrorText()));
        return;
    }

    StudentDao dao(db.database());
    students = dao.listStudents();
    if (students.isEmpty()) {
        resultLabel->setText("当前数据库中没有学生数据，无法进行识别。请先在学生管理页添加学生。" );
        return;
    }

    // Only create and initialize the face engine once
    if (!faceEngine) {
        faceEngine = new FaceEngine();
        if (!faceEngine->initialize(QApplication::applicationDirPath())) {
            resultLabel->setText(faceEngine->lastError());
        } else {
            resultLabel->setText(QString("已加载 %1 名学生，识别引擎已就绪。" ).arg(students.size()));
        }
    } else {
        resultLabel->setText(QString("已加载 %1 名学生，识别引擎已就绪。" ).arg(students.size()));
    }
}

void FaceAttendanceWidget::refreshStudents()
{
    // Reload students from database and update the display
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        return;
    }

    StudentDao dao(db.database());
    students = dao.listStudents();

    if (isRunning) {
        const int enrolledCount = std::count_if(students.begin(), students.end(), [](const Student &s) {
            return s.faceFeatureSize() > 0 && !s.faceFeature().isEmpty();
        });
        resultLabel->setText(QString("学生列表已刷新：共 %1 名学生，%2 名已录入人脸。")
                                 .arg(students.size())
                                 .arg(enrolledCount));
    } else if (!students.isEmpty()) {
        resultLabel->setText(QString("已加载 %1 名学生，识别引擎已就绪。" ).arg(students.size()));
    }
}

void FaceAttendanceWidget::onStartCapture()
{
    if (isRunning) {
        return;
    }

    // Reload students to get the latest data (including newly enrolled faces)
    loadStudents();

    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        QMessageBox::information(this, "摄像头检测", "当前环境中没有检测到可用摄像头。" );
        return;
    }

    camera = new QCamera(cameras.first(), this);
    imageCapture = new QImageCapture(this);
    captureSession = new QMediaCaptureSession(this);
    captureSession->setCamera(camera);
    captureSession->setImageCapture(imageCapture);

    // Connect imageCaptured signal to do recognition on the just-captured frame
    connect(imageCapture, &QImageCapture::imageCaptured,
            this, &FaceAttendanceWidget::onImageCaptured);

    previewLabel->setText("摄像头已启动，正在等待画面...");
    camera->start();

    // Timer fires every 1.5s to trigger a new capture
    captureTimer = new QTimer(this);
    connect(captureTimer, &QTimer::timeout, this, &FaceAttendanceWidget::onFrameCaptured);
    captureTimer->start(1500);
    isRunning = true;

    const int enrolledCount = std::count_if(students.begin(), students.end(), [](const Student &s) {
        return s.faceFeatureSize() > 0 && !s.faceFeature().isEmpty();
    });
    const QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    resultLabel->setText(QString("人脸识别已启动（%1 名学生，%2 名已录入），当前时间：%3")
                             .arg(students.size())
                             .arg(enrolledCount)
                             .arg(now));
}

void FaceAttendanceWidget::onStopCapture()
{
    if (captureTimer) {
        captureTimer->stop();
    }
    if (camera) {
        camera->stop();
    }
    if (captureSession) {
        captureSession->setCamera(nullptr);
        captureSession->setImageCapture(nullptr);
    }
    latestFrame = QImage();
    isRunning = false;
    resultLabel->setText("识别已停止。" );
    previewLabel->setText("摄像头预览已停止");
}

// Timer callback — only triggers an async capture, does NOT process the frame.
// Recognition happens in onImageCaptured() when the frame actually arrives.
void FaceAttendanceWidget::onFrameCaptured()
{
    if (!camera || !isRunning || !imageCapture) {
        return;
    }

    imageCapture->capture();
}

// Called when a frame is actually captured — do recognition on THIS frame.
void FaceAttendanceWidget::onImageCaptured(int id, const QImage &preview)
{
    Q_UNUSED(id)

    if (preview.isNull()) {
        return;
    }

    // Update preview
    latestFrame = preview;
    previewLabel->setPixmap(QPixmap::fromImage(
        preview.scaled(previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));

    // Run recognition on the just-captured frame
    processFrame(preview);
}

void FaceAttendanceWidget::processFrame(const QImage &frame)
{
    if (!faceEngine || !faceEngine->isInitialized()) {
        resultLabel->setText("识别引擎未就绪，请检查模型文件。");
        return;
    }

    if (students.isEmpty()) {
        resultLabel->setText("没有可识别的学生数据。");
        return;
    }

    const FaceRecognitionResult recognition = faceEngine->recognize(frame, students, 0.65f);
    if (recognition.matched) {
        DatabaseManager &db = DatabaseManager::instance();
        AttendanceDao dao(db.database());
        const QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        if (dao.addAttendanceRecord(recognition.studentNo, now, 0, 0, QString("识别成功:%1").arg(recognition.name))) {
            resultLabel->setText(QString("识别成功：%1 (%2) 考勤已写入 (匹配度: %3)")
                                     .arg(recognition.name, recognition.studentNo)
                                     .arg(static_cast<double>(recognition.score), 0, 'f', 2));
        } else {
            resultLabel->setText(QString("识别成功但写入考勤记录失败：%1").arg(recognition.studentNo));
        }
    } else {
        resultLabel->setText(recognition.message);
    }
}
