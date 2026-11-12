#ifndef FACEATTENDANCEWIDGET_H
#define FACEATTENDANCEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCamera>
#include <QTimer>
#include <QImage>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QList>

#include "../database/DatabaseManager.h"
#include "../database/StudentDao.h"
#include "../database/AttendanceDao.h"
#include "../face/FaceEngine.h"

class FaceAttendanceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FaceAttendanceWidget(QWidget *parent = nullptr);
    ~FaceAttendanceWidget() override;

public slots:
    void refreshStudents();

private slots:
    void onStartCapture();
    void onStopCapture();
    void onFrameCaptured();
    void onImageCaptured(int id, const QImage &preview);

private:
    void setupUi();
    void loadStudents();
    void processFrame(const QImage &frame);

    QLabel *previewLabel;
    QLabel *resultLabel;
    QPushButton *startButton;
    QPushButton *stopButton;
    QCamera *camera;
    QImageCapture *imageCapture;
    QMediaCaptureSession *captureSession;
    QTimer *captureTimer;
    FaceEngine *faceEngine;
    QList<Student> students;
    QImage latestFrame;
    bool isRunning;
};

#endif
