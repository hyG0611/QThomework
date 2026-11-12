#ifndef FACEENGINE_H
#define FACEENGINE_H

#include <QImage>
#include <QByteArray>
#include <QString>
#include <QList>
#include <QRect>

#include "../models/Student.h"

struct FaceRecognitionResult
{
    bool matched = false;
    QString studentNo;
    QString name;
    float score = 0.0f;
    QString message;
};

class FaceEngine
{
public:
    FaceEngine();
    ~FaceEngine();

    bool initialize(const QString &modelDir = QString());
    bool isInitialized() const;

    // Detect the primary face in the image. Returns false if no face is found.
    bool detect(const QImage &image, QRect &faceRect);

    // Extract a face feature vector from the image. Returns empty bytes on failure.
    QByteArray extractFeature(const QImage &image);

    // Compare current image against all registered students.
    FaceRecognitionResult recognize(const QImage &image, const QList<Student> &students, float threshold = 0.65f);

    float compareFeatures(const QByteArray &featureA, const QByteArray &featureB);
    QString lastError() const;

private:
    static QByteArray imageToRGB888(const QImage &image);

    bool m_initialized;
    QString m_modelDir;
    QString m_lastError;
    int m_featureSize;

    // Opaque pointers — cast to actual SeetaFace types in .cpp with #ifdef HAVE_SEETA_FACE6
    void *m_faceDetector;
    void *m_faceLandmarker;
    void *m_faceRecognizer;
};

#endif
