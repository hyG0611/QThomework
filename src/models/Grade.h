#ifndef GRADE_H
#define GRADE_H

#include <QString>

class Grade
{
public:
    Grade();

    qint64 id() const;
    void setId(qint64 newId);

    QString studentNo() const;
    void setStudentNo(const QString &newStudentNo);

    QString studentName() const;
    void setStudentName(const QString &newStudentName);

    QString courseNo() const;
    void setCourseNo(const QString &newCourseNo);

    QString courseName() const;
    void setCourseName(const QString &newCourseName);

    float credit() const;
    void setCredit(float newCredit);

    float score() const;
    void setScore(float newScore);

    QString semester() const;
    void setSemester(const QString &newSemester);

    QString recordTime() const;
    void setRecordTime(const QString &newRecordTime);

private:
    qint64 m_id;
    QString m_studentNo;
    QString m_studentName;
    QString m_courseNo;
    QString m_courseName;
    float m_credit;
    float m_score;
    QString m_semester;
    QString m_recordTime;
};

#endif
