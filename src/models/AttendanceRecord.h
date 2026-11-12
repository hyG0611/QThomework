#ifndef ATTENDANCERECORD_H
#define ATTENDANCERECORD_H

#include <QString>

class AttendanceRecord
{
public:
    AttendanceRecord();

    int id() const;
    void setId(int newId);

    QString studentNo() const;
    void setStudentNo(const QString &newStudentNo);

    QString attendanceTime() const;
    void setAttendanceTime(const QString &newAttendanceTime);

    int attendanceType() const;
    void setAttendanceType(int newAttendanceType);

    int checkMode() const;
    void setCheckMode(int newCheckMode);

    QString remark() const;
    void setRemark(const QString &newRemark);

private:
    int m_id;
    QString m_studentNo;
    QString m_attendanceTime;
    int m_attendanceType;
    int m_checkMode;
    QString m_remark;
};

#endif
