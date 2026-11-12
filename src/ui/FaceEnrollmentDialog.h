#ifndef FACEENROLLMENTDIALOG_H
#define FACEENROLLMENTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QCamera>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QImage>
#include <QByteArray>
#include <QTimer>

#include "../face/FaceEngine.h"

class FaceEnrollmentDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FaceEnrollmentDialog(QWidget *parent = nullptr);
    ~FaceEnrollmentDialog() override;

    // Returns the extracted face feature data (empty if enrollment failed).
    QByteArray faceFeature() const;
    int faceFeatureSize() const;

private slots:
    void onCapture();
    void onAccept();

private:
    void setupUi();
    void startCamera();
    void stopCamera();

    QLabel *m_previewLabel;
    QLabel *m_statusLabel;
    QPushButton *m_captureButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    QCamera *m_camera;
    QImageCapture *m_imageCapture;
    QMediaCaptureSession *m_captureSession;
    QTimer *m_captureTimer;

    FaceEngine *m_faceEngine;
    QImage m_latestFrame;
    QByteArray m_faceFeature;
    int m_faceFeatureSize;
    bool m_cameraStarted;
};

#endif
