#include "AttendanceDao.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

AttendanceDao::AttendanceDao(QSqlDatabase database)
    : m_database(std::move(database))
{
}

bool AttendanceDao::addAttendanceRecord(const QString &studentNo,
                                        const QString &attendanceTime,
                                        int attendanceType,
                                        int checkMode,
                                        const QString &remark)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO attendance_records (student_no, attendance_time, attendance_type, check_mode, remark)
        VALUES (?, ?, ?, ?, ?)
    )");
    query.addBindValue(studentNo);
    query.addBindValue(attendanceTime);
    query.addBindValue(attendanceType);
    query.addBindValue(checkMode);
    query.addBindValue(remark);
    return query.exec();
}

QList<QVariantMap> AttendanceDao::dailyStatistics(const QString &dateText)
{
    QSqlQuery query(m_database);
    // Use LIKE prefix match instead of DATE() for cross-database compatibility
    query.prepare(R"(
        SELECT
            COUNT(DISTINCT ar.student_no) AS attended_count,
            (SELECT COUNT(*) FROM students) - COUNT(DISTINCT ar.student_no) AS absent_count
        FROM attendance_records ar
        WHERE ar.attendance_time LIKE ?
    )");
    query.addBindValue(dateText + "%");
    QList<QVariantMap> result;
    if (!query.exec()) {
        qWarning() << "dailyStatistics query failed:" << query.lastError().text();
        return result;
    }
    if (!query.next()) {
        // No attendance records at all for this date — return counts manually
        QSqlQuery countQuery(m_database);
        countQuery.prepare("SELECT COUNT(*) FROM students");
        QVariantMap row;
        if (countQuery.exec() && countQuery.next()) {
            row["attended_count"] = 0;
            row["absent_count"] = countQuery.value(0);
        } else {
            row["attended_count"] = 0;
            row["absent_count"] = 0;
        }
        result.append(row);
        return result;
    }

    QVariantMap row;
    row["attended_count"] = query.value("attended_count");
    row["absent_count"] = query.value("absent_count");
    result.append(row);
    return result;
}

QList<QVariantMap> AttendanceDao::attendedStudents(const QString &dateText)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT DISTINCT s.student_no, s.name, s.gender, s.class_name, s.college,
               MIN(ar.attendance_time) AS first_check_time
        FROM students s
        JOIN attendance_records ar ON s.student_no = ar.student_no
        WHERE ar.attendance_time LIKE ?
        GROUP BY s.student_no, s.name, s.gender, s.class_name, s.college
        ORDER BY s.student_no
    )");
    query.addBindValue(dateText + "%");

    QList<QVariantMap> result;
    if (!query.exec()) {
        qWarning() << "attendedStudents query failed:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        QVariantMap row;
        row["student_no"] = query.value("student_no");
        row["name"] = query.value("name");
        row["gender"] = query.value("gender");
        row["class_name"] = query.value("class_name");
        row["college"] = query.value("college");
        row["first_check_time"] = query.value("first_check_time");
        result.append(row);
    }
    return result;
}

QList<QVariantMap> AttendanceDao::absentStudents(const QString &dateText)
{
    QSqlQuery query(m_database);
    // Use LEFT JOIN instead of NOT IN for better cross-DB compatibility
    query.prepare(R"(
        SELECT s.student_no, s.name, s.gender, s.class_name, s.college
        FROM students s
        LEFT JOIN attendance_records ar
            ON s.student_no = ar.student_no AND ar.attendance_time LIKE ?
        WHERE ar.student_no IS NULL
        ORDER BY s.student_no
    )");
    query.addBindValue(dateText + "%");

    QList<QVariantMap> result;
    if (!query.exec()) {
        qWarning() << "absentStudents query failed:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        QVariantMap row;
        row["student_no"] = query.value("student_no");
        row["name"] = query.value("name");
        row["gender"] = query.value("gender");
        row["class_name"] = query.value("class_name");
        row["college"] = query.value("college");
        result.append(row);
    }
    return result;
}
