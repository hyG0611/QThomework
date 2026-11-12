#include "Student.h"

Student::Student()
    : m_id(0), m_gender(0), m_faceFeatureSize(0)
{
}

int Student::id() const { return m_id; }
void Student::setId(int newId) { m_id = newId; }

QString Student::studentNo() const { return m_studentNo; }
void Student::setStudentNo(const QString &newStudentNo) { m_studentNo = newStudentNo; }

QString Student::name() const { return m_name; }
void Student::setName(const QString &newName) { m_name = newName; }

int Student::gender() const { return m_gender; }
void Student::setGender(int newGender) { m_gender = newGender; }

QString Student::ethnicity() const { return m_ethnicity; }
void Student::setEthnicity(const QString &newEthnicity) { m_ethnicity = newEthnicity; }

QString Student::idCard() const { return m_idCard; }
void Student::setIdCard(const QString &newIdCard) { m_idCard = newIdCard; }

QString Student::className() const { return m_className; }
void Student::setClassName(const QString &newClassName) { m_className = newClassName; }

QString Student::college() const { return m_college; }
void Student::setCollege(const QString &newCollege) { m_college = newCollege; }

QByteArray Student::faceFeature() const { return m_faceFeature; }
void Student::setFaceFeature(const QByteArray &newFaceFeature) { m_faceFeature = newFaceFeature; }

int Student::faceFeatureSize() const { return m_faceFeatureSize; }
void Student::setFaceFeatureSize(int newFaceFeatureSize) { m_faceFeatureSize = newFaceFeatureSize; }

QString Student::createdAt() const { return m_createdAt; }
void Student::setCreatedAt(const QString &newCreatedAt) { m_createdAt = newCreatedAt; }

QString Student::updatedAt() const { return m_updatedAt; }
void Student::setUpdatedAt(const QString &newUpdatedAt) { m_updatedAt = newUpdatedAt; }
