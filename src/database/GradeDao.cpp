#include "GradeDao.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <utility>

GradeDao::GradeDao(QSqlDatabase database)
    : m_database(std::move(database))
{
}

QString GradeDao::lastErrorText() const
{
    return m_lastError;
}

bool GradeDao::addGrade(const Grade &grade)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO grades (student_no, course_no, score)
        VALUES (?, ?, ?)
    )");
    query.addBindValue(grade.studentNo());
    query.addBindValue(grade.courseNo());
    query.addBindValue(grade.score());
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    m_lastError.clear();
    return true;
}

bool GradeDao::updateGrade(const Grade &grade)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE grades
        SET student_no = ?, course_no = ?, score = ?, record_time = CURRENT_TIMESTAMP
        WHERE id = ?
    )");
    query.addBindValue(grade.studentNo());
    query.addBindValue(grade.courseNo());
    query.addBindValue(grade.score());
    query.addBindValue(grade.id());
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    m_lastError.clear();
    return true;
}

bool GradeDao::deleteGrade(qint64 id)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM grades WHERE id = ?");
    query.addBindValue(id);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    m_lastError.clear();
    return true;
}

QList<Grade> GradeDao::listGrades(const QString &studentNo, const QString &courseKeyword, const QString &semester)
{
    QSqlQuery query(m_database);
    QString sql = R"(
        SELECT g.id,
               g.student_no,
               s.name AS student_name,
               g.course_no,
               c.course_name,
               c.credit,
               c.semester,
               g.score,
               g.record_time
        FROM grades g
        JOIN students s ON g.student_no = s.student_no
        JOIN courses c ON g.course_no = c.course_no
    )";

    QStringList conditions;
    QList<QVariant> bindValues;

    if (!studentNo.isEmpty()) {
        conditions.append("g.student_no LIKE ?");
        bindValues.append(QString("%%%1%").arg(studentNo));
    }
    if (!courseKeyword.isEmpty()) {
        conditions.append("(g.course_no LIKE ? OR c.course_name LIKE ?)");
        const QString pattern = QString("%%%1%").arg(courseKeyword);
        bindValues.append(pattern);
        bindValues.append(pattern);
    }
    if (!semester.isEmpty()) {
        conditions.append("c.semester LIKE ?");
        bindValues.append(QString("%%%1%").arg(semester));
    }

    if (!conditions.isEmpty()) {
        sql += " WHERE " + conditions.join(" AND ");
    }
    sql += " ORDER BY g.record_time DESC";

    query.prepare(sql);
    for (const QVariant &value : bindValues) {
        query.addBindValue(value);
    }

    QList<Grade> grades;
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return grades;
    }

    while (query.next()) {
        Grade grade;
        grade.setId(query.value("id").toLongLong());
        grade.setStudentNo(query.value("student_no").toString());
        grade.setStudentName(query.value("student_name").toString());
        grade.setCourseNo(query.value("course_no").toString());
        grade.setCourseName(query.value("course_name").toString());
        grade.setCredit(query.value("credit").toFloat());
        grade.setSemester(query.value("semester").toString());
        grade.setScore(query.value("score").toFloat());
        grade.setRecordTime(query.value("record_time").toString());
        grades.append(grade);
    }
    return grades;
}

Grade GradeDao::findById(qint64 id)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT g.id,
               g.student_no,
               s.name AS student_name,
               g.course_no,
               c.course_name,
               c.credit,
               c.semester,
               g.score,
               g.record_time
        FROM grades g
        JOIN students s ON g.student_no = s.student_no
        JOIN courses c ON g.course_no = c.course_no
        WHERE g.id = ?
    )");
    query.addBindValue(id);
    if (!query.exec() || !query.next()) {
        return Grade();
    }

    Grade grade;
    grade.setId(query.value("id").toLongLong());
    grade.setStudentNo(query.value("student_no").toString());
    grade.setStudentName(query.value("student_name").toString());
    grade.setCourseNo(query.value("course_no").toString());
    grade.setCourseName(query.value("course_name").toString());
    grade.setCredit(query.value("credit").toFloat());
    grade.setSemester(query.value("semester").toString());
    grade.setScore(query.value("score").toFloat());
    grade.setRecordTime(query.value("record_time").toString());
    return grade;
}
