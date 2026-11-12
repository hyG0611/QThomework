#ifndef ATTENDANCEDAOH
#define ATTENDANCEDAOH

#include <QList>
#include <QSqlDatabase>
#include <QString>
#include <QVariant>

class AttendanceDao
{
public:
    explicit AttendanceDao(QSqlDatabase database);

    bool addAttendanceRecord(const QString &studentNo,
                             const QString &attendanceTime,
                             int attendanceType = 0,
                             int checkMode = 0,
                             const QString &remark = QString());

    QList<QVariantMap> dailyStatistics(const QString &dateText);

    // Return students who attended on the given date (with attendance time).
    QList<QVariantMap> attendedStudents(const QString &dateText);

    // Return students who did NOT attend on the given date.
    QList<QVariantMap> absentStudents(const QString &dateText);

private:
    QSqlDatabase m_database;
};

#endif
