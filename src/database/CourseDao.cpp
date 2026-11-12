#include "CourseDao.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <utility>

CourseDao::CourseDao(QSqlDatabase database)
    : m_database(std::move(database))
{
}

QString CourseDao::lastErrorText() const
{
    return m_lastError;
}

bool CourseDao::addCourse(const Course &course)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO courses (course_no, course_name, credit, semester)
        VALUES (?, ?, ?, ?)
    )");
    query.addBindValue(course.courseNo());
    query.addBindValue(course.courseName());
    query.addBindValue(course.credit());
    query.addBindValue(course.semester());
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    m_lastError.clear();
    return true;
}

bool CourseDao::updateCourse(const Course &course)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE courses
        SET course_name = ?, credit = ?, semester = ?
        WHERE course_no = ?
    )");
    query.addBindValue(course.courseName());
    query.addBindValue(course.credit());
    query.addBindValue(course.semester());
    query.addBindValue(course.courseNo());
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    m_lastError.clear();
    return true;
}

bool CourseDao::deleteCourse(const QString &courseNo)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM courses WHERE course_no = ?");
    query.addBindValue(courseNo);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    m_lastError.clear();
    return true;
}

QList<Course> CourseDao::listCourses(const QString &keyword)
{
    QSqlQuery query(m_database);
    QString sql = "SELECT id, course_no, course_name, credit, semester, created_at FROM courses";
    if (!keyword.isEmpty()) {
        sql += " WHERE course_no LIKE ? OR course_name LIKE ? OR semester LIKE ?";
        query.prepare(sql);
        const QString pattern = QString("%%%1%").arg(keyword);
        query.addBindValue(pattern);
        query.addBindValue(pattern);
        query.addBindValue(pattern);
    } else {
        query.prepare(sql);
    }

    QList<Course> courses;
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return courses;
    }

    while (query.next()) {
        Course course;
        course.setId(query.value("id").toInt());
        course.setCourseNo(query.value("course_no").toString());
        course.setCourseName(query.value("course_name").toString());
        course.setCredit(query.value("credit").toFloat());
        course.setSemester(query.value("semester").toString());
        course.setCreatedAt(query.value("created_at").toString());
        courses.append(course);
    }
    return courses;
}

Course CourseDao::findByCourseNo(const QString &courseNo)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT id, course_no, course_name, credit, semester, created_at FROM courses WHERE course_no = ?");
    query.addBindValue(courseNo);
    if (!query.exec() || !query.next()) {
        return Course();
    }

    Course course;
    course.setId(query.value("id").toInt());
    course.setCourseNo(query.value("course_no").toString());
    course.setCourseName(query.value("course_name").toString());
    course.setCredit(query.value("credit").toFloat());
    course.setSemester(query.value("semester").toString());
    course.setCreatedAt(query.value("created_at").toString());
    return course;
}
