#include "Course.h"

Course::Course()
    : m_id(0), m_credit(0.0f)
{
}

int Course::id() const { return m_id; }
void Course::setId(int newId) { m_id = newId; }

QString Course::courseNo() const { return m_courseNo; }
void Course::setCourseNo(const QString &newCourseNo) { m_courseNo = newCourseNo; }

QString Course::courseName() const { return m_courseName; }
void Course::setCourseName(const QString &newCourseName) { m_courseName = newCourseName; }

float Course::credit() const { return m_credit; }
void Course::setCredit(float newCredit) { m_credit = newCredit; }

QString Course::semester() const { return m_semester; }
void Course::setSemester(const QString &newSemester) { m_semester = newSemester; }

QString Course::createdAt() const { return m_createdAt; }
void Course::setCreatedAt(const QString &newCreatedAt) { m_createdAt = newCreatedAt; }
