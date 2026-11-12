#include "AttendanceRecord.h"

AttendanceRecord::AttendanceRecord()
    : m_id(0), m_attendanceType(0), m_checkMode(0)
{
}

int AttendanceRecord::id() const { return m_id; }
void AttendanceRecord::setId(int newId) { m_id = newId; }

QString AttendanceRecord::studentNo() const { return m_studentNo; }
void AttendanceRecord::setStudentNo(const QString &newStudentNo) { m_studentNo = newStudentNo; }

QString AttendanceRecord::attendanceTime() const { return m_attendanceTime; }
void AttendanceRecord::setAttendanceTime(const QString &newAttendanceTime) { m_attendanceTime = newAttendanceTime; }

int AttendanceRecord::attendanceType() const { return m_attendanceType; }
void AttendanceRecord::setAttendanceType(int newAttendanceType) { m_attendanceType = newAttendanceType; }

int AttendanceRecord::checkMode() const { return m_checkMode; }
void AttendanceRecord::setCheckMode(int newCheckMode) { m_checkMode = newCheckMode; }

QString AttendanceRecord::remark() const { return m_remark; }
void AttendanceRecord::setRemark(const QString &newRemark) { m_remark = newRemark; }
