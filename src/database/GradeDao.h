#ifndef GRADEDAO_H
#define GRADEDAO_H

#include "../models/Grade.h"
#include <QList>
#include <QString>
#include <QSqlDatabase>

class GradeDao
{
public:
    explicit GradeDao(QSqlDatabase database);

    bool addGrade(const Grade &grade);
    bool updateGrade(const Grade &grade);
    bool deleteGrade(qint64 id);
    QList<Grade> listGrades(const QString &studentNo = QString(),
                            const QString &courseKeyword = QString(),
                            const QString &semester = QString());
    Grade findById(qint64 id);
    QString lastErrorText() const;

private:
    QSqlDatabase m_database;
    QString m_lastError;
};

#endif
