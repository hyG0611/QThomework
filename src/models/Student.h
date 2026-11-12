#ifndef STUDENT_H
#define STUDENT_H

#include <QString>
#include <QByteArray>

class Student
{
public:
    Student();

    int id() const;
    void setId(int newId);

    QString studentNo() const;
    void setStudentNo(const QString &newStudentNo);

    QString name() const;
    void setName(const QString &newName);

    int gender() const;
    void setGender(int newGender);

    QString ethnicity() const;
    void setEthnicity(const QString &newEthnicity);

    QString idCard() const;
    void setIdCard(const QString &newIdCard);

    QString className() const;
    void setClassName(const QString &newClassName);

    QString college() const;
    void setCollege(const QString &newCollege);

    QByteArray faceFeature() const;
    void setFaceFeature(const QByteArray &newFaceFeature);

    int faceFeatureSize() const;
    void setFaceFeatureSize(int newFaceFeatureSize);

    QString createdAt() const;
    void setCreatedAt(const QString &newCreatedAt);

    QString updatedAt() const;
    void setUpdatedAt(const QString &newUpdatedAt);

private:
    int m_id;
    QString m_studentNo;
    QString m_name;
    int m_gender;
    QString m_ethnicity;
    QString m_idCard;
    QString m_className;
    QString m_college;
    QByteArray m_faceFeature;
    int m_faceFeatureSize;
    QString m_createdAt;
    QString m_updatedAt;
};

#endif
