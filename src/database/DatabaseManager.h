#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager
{
public:
    static DatabaseManager &instance();

    bool initialize(const QString &driverName = QString(), const QString &databasePath = QString());
    bool ensureSchema();
    QSqlDatabase database() const;
    QString lastErrorText() const;

private:
    DatabaseManager();
    ~DatabaseManager();

    QSqlDatabase m_database;
    QString m_lastError;
    QString m_connectionName;
};

#endif
