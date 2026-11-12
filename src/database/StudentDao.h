#ifndef STUDENTDAO_H
#define STUDENTDAO_H

#include "../models/Student.h"
#include <QList>
#include <QString>
#include <QSqlDatabase>

class StudentDao
{
public:
    explicit StudentDao(QSqlDatabase database);

    bool addStudent(const Student &student);
    bool updateStudent(const Student &student);
    bool deleteStudent(const QString &studentNo);
    QList<Student> listStudents(const QString &studentNo = QString(),
                                const QString &name = QString(),
                                const QString &college = QString());
    Student findByStudentNo(const QString &studentNo);
    QString lastErrorText() const;

private:
    QSqlDatabase m_database;
    QString m_lastError;
};

#endif
