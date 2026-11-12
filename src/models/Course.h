#ifndef COURSE_H
#define COURSE_H

#include <QString>

class Course
{
public:
    Course();

    int id() const;
    void setId(int newId);

    QString courseNo() const;
    void setCourseNo(const QString &newCourseNo);

    QString courseName() const;
    void setCourseName(const QString &newCourseName);

    float credit() const;
    void setCredit(float newCredit);

    QString semester() const;
    void setSemester(const QString &newSemester);

    QString createdAt() const;
    void setCreatedAt(const QString &newCreatedAt);

private:
    int m_id;
    QString m_courseNo;
    QString m_courseName;
    float m_credit;
    QString m_semester;
    QString m_createdAt;
};

#endif
