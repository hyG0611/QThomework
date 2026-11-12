#include "Grade.h"

Grade::Grade()
    : m_id(0), m_credit(0.0f), m_score(0.0f)
{
}

qint64 Grade::id() const { return m_id; }
void Grade::setId(qint64 newId) { m_id = newId; }

QString Grade::studentNo() const { return m_studentNo; }
void Grade::setStudentNo(const QString &newStudentNo) { m_studentNo = newStudentNo; }

QString Grade::studentName() const { return m_studentName; }
void Grade::setStudentName(const QString &newStudentName) { m_studentName = newStudentName; }

QString Grade::courseNo() const { return m_courseNo; }
void Grade::setCourseNo(const QString &newCourseNo) { m_courseNo = newCourseNo; }

QString Grade::courseName() const { return m_courseName; }
void Grade::setCourseName(const QString &newCourseName) { m_courseName = newCourseName; }

float Grade::credit() const { return m_credit; }
void Grade::setCredit(float newCredit) { m_credit = newCredit; }

float Grade::score() const { return m_score; }
void Grade::setScore(float newScore) { m_score = newScore; }

QString Grade::semester() const { return m_semester; }
void Grade::setSemester(const QString &newSemester) { m_semester = newSemester; }

QString Grade::recordTime() const { return m_recordTime; }
void Grade::setRecordTime(const QString &newRecordTime) { m_recordTime = newRecordTime; }
