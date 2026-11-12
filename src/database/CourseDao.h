#ifndef COURSEDAO_H
#define COURSEDAO_H

#include "../models/Course.h"
#include <QList>
#include <QString>
#include <QSqlDatabase>

class CourseDao
{
public:
    explicit CourseDao(QSqlDatabase database);

    bool addCourse(const Course &course);
    bool updateCourse(const Course &course);
    bool deleteCourse(const QString &courseNo);
    QList<Course> listCourses(const QString &keyword = QString());
    Course findByCourseNo(const QString &courseNo);
    QString lastErrorText() const;

private:
    QSqlDatabase m_database;
    QString m_lastError;
};

#endif
