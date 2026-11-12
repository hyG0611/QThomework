#include "DatabaseManager.h"

#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

DatabaseManager::DatabaseManager()
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager manager;
    return manager;
}

bool DatabaseManager::initialize(const QString &driverName, const QString &databasePath)
{
    if (m_database.isOpen()) {
        return true;
    }

    QString requestedDriver = driverName;
    if (requestedDriver.isEmpty()) {
        requestedDriver = qEnvironmentVariable("ATTENDANCE_DB_DRIVER", "QSQLITE");
    }

    QString effectiveDriver = requestedDriver;
    if (effectiveDriver.compare("QMYSQL", Qt::CaseInsensitive) == 0) {
        if (!QSqlDatabase::isDriverAvailable("QMYSQL")) {
            qWarning() << "QMYSQL driver is not available. Available drivers:" << QSqlDatabase::drivers();
            if (QSqlDatabase::isDriverAvailable("QSQLITE")) {
                effectiveDriver = "QSQLITE";
                qWarning() << "Falling back to QSQLITE.";
            } else {
                m_lastError = "QMYSQL driver is not available and QSQLITE is not available either.";
                qCritical() << m_lastError;
                return false;
            }
        }
    } else if (effectiveDriver.compare("QODBC", Qt::CaseInsensitive) == 0) {
        if (!QSqlDatabase::isDriverAvailable("QODBC")) {
            qWarning() << "QODBC driver is not available.";
            if (QSqlDatabase::isDriverAvailable("QMYSQL")) {
                effectiveDriver = "QMYSQL";
                qWarning() << "Falling back to QMYSQL.";
            } else if (QSqlDatabase::isDriverAvailable("QSQLITE")) {
                effectiveDriver = "QSQLITE";
                qWarning() << "Falling back to QSQLITE.";
            } else {
                m_lastError = "Neither QODBC, QMYSQL nor QSQLITE drivers are available.";
                qCritical() << m_lastError;
                return false;
            }
        }
    } else if (effectiveDriver.compare("QSQLITE", Qt::CaseInsensitive) == 0) {
        if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
            m_lastError = "QSQLITE driver is not available.";
            qCritical() << m_lastError;
            return false;
        }
    }

    if (!m_connectionName.isEmpty()) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    m_connectionName = "attendance_connection";
    m_database = QSqlDatabase::addDatabase(effectiveDriver, m_connectionName);
    qInfo() << "Database driver selected:" << effectiveDriver;

    if (effectiveDriver.compare("QODBC", Qt::CaseInsensitive) == 0) {
        m_database.setDatabaseName(qEnvironmentVariable("ATTENDANCE_DB_ODBC_DSN"));
        m_database.setUserName(qEnvironmentVariable("ATTENDANCE_DB_USER"));
        m_database.setPassword(qEnvironmentVariable("ATTENDANCE_DB_PASSWORD"));
    } else if (effectiveDriver.compare("QMYSQL", Qt::CaseInsensitive) == 0) {
        m_database.setHostName("127.0.0.1");
        m_database.setPort(3306);
        m_database.setDatabaseName("attendance_system");
        m_database.setUserName(qEnvironmentVariable("ATTENDANCE_DB_USER"));
        m_database.setPassword(qEnvironmentVariable("ATTENDANCE_DB_PASSWORD"));
    } else if (effectiveDriver.compare("QSQLITE", Qt::CaseInsensitive) == 0) {
        QString dbPath = databasePath;
        if (dbPath.isEmpty()) {
            dbPath = QCoreApplication::applicationDirPath() + "/attendance_db.sqlite";
        }
        m_database.setDatabaseName(dbPath);
    }

    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        qCritical() << "Database open failed:" << m_lastError;
        return false;
    }

    if (!ensureSchema()) {
        if (m_database.isOpen()) {
            m_database.close();
        }
        return false;
    }

    m_lastError.clear();
    return true;
}

bool DatabaseManager::ensureSchema()
{
    if (!m_database.isOpen()) {
        return false;
    }

    const bool useMySql = m_database.driverName().compare("QMYSQL", Qt::CaseInsensitive) == 0
        || m_database.driverName().compare("QODBC", Qt::CaseInsensitive) == 0;
    QSqlQuery query(m_database);
    QStringList statements;

    if (useMySql) {
        statements = {
            "CREATE TABLE IF NOT EXISTS classes ("
            "id INT AUTO_INCREMENT PRIMARY KEY,"
            "class_name VARCHAR(50) NOT NULL UNIQUE,"
            "department VARCHAR(50) DEFAULT NULL,"
            "grade VARCHAR(20) DEFAULT NULL,"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
            ")",
            "CREATE TABLE IF NOT EXISTS students ("
            "id INT AUTO_INCREMENT PRIMARY KEY,"
            "student_no VARCHAR(32) NOT NULL UNIQUE,"
            "name VARCHAR(50) NOT NULL,"
            "gender TINYINT DEFAULT 0,"
            "ethnicity VARCHAR(30) DEFAULT NULL,"
            "id_card VARCHAR(18) DEFAULT NULL UNIQUE,"
            "face_feature LONGBLOB DEFAULT NULL,"
            "face_feature_size INT DEFAULT 0,"
            "class_name VARCHAR(50) DEFAULT NULL,"
            "college VARCHAR(50) DEFAULT NULL,"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP"
            ")",
            "CREATE TABLE IF NOT EXISTS attendance_records ("
            "id BIGINT AUTO_INCREMENT PRIMARY KEY,"
            "student_no VARCHAR(32) NOT NULL,"
            "attendance_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "attendance_type TINYINT DEFAULT 0,"
            "check_mode TINYINT DEFAULT 0,"
            "remark VARCHAR(200) DEFAULT NULL"
            ")",
            "CREATE TABLE IF NOT EXISTS courses ("
            "id INT AUTO_INCREMENT PRIMARY KEY,"
            "course_no VARCHAR(32) NOT NULL UNIQUE,"
            "course_name VARCHAR(100) NOT NULL,"
            "credit FLOAT DEFAULT 0,"
            "semester VARCHAR(20) DEFAULT NULL,"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
            ")",
            "CREATE TABLE IF NOT EXISTS grades ("
            "id BIGINT AUTO_INCREMENT PRIMARY KEY,"
            "student_no VARCHAR(32) NOT NULL,"
            "course_no VARCHAR(32) NOT NULL,"
            "score FLOAT DEFAULT 0,"
            "record_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "UNIQUE KEY uk_student_course (student_no, course_no)"
            ")"
        };
    } else {
        statements = {
            "CREATE TABLE IF NOT EXISTS classes ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "class_name VARCHAR(50) NOT NULL UNIQUE,"
            "department VARCHAR(50) DEFAULT NULL,"
            "grade VARCHAR(20) DEFAULT NULL,"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
            ")",
            "CREATE TABLE IF NOT EXISTS students ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "student_no VARCHAR(32) NOT NULL UNIQUE,"
            "name VARCHAR(50) NOT NULL,"
            "gender TINYINT DEFAULT 0,"
            "ethnicity VARCHAR(30) DEFAULT NULL,"
            "id_card VARCHAR(18) DEFAULT NULL UNIQUE,"
            "face_feature BLOB DEFAULT NULL,"
            "face_feature_size INT DEFAULT 0,"
            "class_name VARCHAR(50) DEFAULT NULL,"
            "college VARCHAR(50) DEFAULT NULL,"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
            ")",
            "CREATE TABLE IF NOT EXISTS attendance_records ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "student_no VARCHAR(32) NOT NULL,"
            "attendance_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "attendance_type TINYINT DEFAULT 0,"
            "check_mode TINYINT DEFAULT 0,"
            "remark VARCHAR(200) DEFAULT NULL"
            ")",
            "CREATE TABLE IF NOT EXISTS courses ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "course_no VARCHAR(32) NOT NULL UNIQUE,"
            "course_name VARCHAR(100) NOT NULL,"
            "credit FLOAT DEFAULT 0,"
            "semester VARCHAR(20) DEFAULT NULL,"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
            ")",
            "CREATE TABLE IF NOT EXISTS grades ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "student_no VARCHAR(32) NOT NULL,"
            "course_no VARCHAR(32) NOT NULL,"
            "score FLOAT DEFAULT 0,"
            "record_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "UNIQUE(student_no, course_no)"
            ")"
        };
    }

    for (const QString &statement : statements) {
        if (!query.exec(statement)) {
            m_lastError = query.lastError().text();
            qCritical() << "Schema setup failed:" << statement << m_lastError;
            return false;
        }
    }

    // Schema migration for existing databases: add missing columns / tables.
    // Then insert sample data (best-effort — ignored if rows already exist or columns are missing).
    const QStringList migrations = useMySql
        ? QStringList{
            "ALTER TABLE students ADD COLUMN college VARCHAR(50) DEFAULT NULL",
            "CREATE TABLE IF NOT EXISTS courses ("
            "id INT AUTO_INCREMENT PRIMARY KEY,"
            "course_no VARCHAR(32) NOT NULL UNIQUE,"
            "course_name VARCHAR(100) NOT NULL,"
            "credit FLOAT DEFAULT 0,"
            "semester VARCHAR(20) DEFAULT NULL,"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
            ")",
            "CREATE TABLE IF NOT EXISTS grades ("
            "id BIGINT AUTO_INCREMENT PRIMARY KEY,"
            "student_no VARCHAR(32) NOT NULL,"
            "course_no VARCHAR(32) NOT NULL,"
            "score FLOAT DEFAULT 0,"
            "record_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "UNIQUE KEY uk_student_course (student_no, course_no)"
            ")",
            "INSERT IGNORE INTO classes (class_name, department, grade) VALUES ('计算机2201', '计算机学院', '2022')",
            "INSERT IGNORE INTO classes (class_name, department, grade) VALUES ('计算机2202', '计算机学院', '2022')",
            "INSERT IGNORE INTO students (student_no, name, gender, ethnicity, id_card, class_name, college) VALUES ('2022010001', '张三', 1, '汉族', 'DEMO-ID-001', '计算机2201', '计算机学院')",
            "INSERT IGNORE INTO students (student_no, name, gender, ethnicity, id_card, class_name, college) VALUES ('2022010002', '李四', 2, '汉族', 'DEMO-ID-002', '计算机2202', '计算机学院')",
            "INSERT IGNORE INTO courses (course_no, course_name, credit, semester) VALUES ('CS101', '数据结构', 4.0, '2023-2024-1')",
            "INSERT IGNORE INTO courses (course_no, course_name, credit, semester) VALUES ('CS102', '操作系统', 3.5, '2023-2024-1')",
            "INSERT IGNORE INTO courses (course_no, course_name, credit, semester) VALUES ('CS103', '计算机网络', 3.0, '2023-2024-2')",
            "INSERT IGNORE INTO courses (course_no, course_name, credit, semester) VALUES ('MATH201', '高等数学', 5.0, '2023-2024-1')",
            "INSERT IGNORE INTO courses (course_no, course_name, credit, semester) VALUES ('ENG101', '大学英语', 3.0, '2023-2024-1')",
            "INSERT IGNORE INTO grades (student_no, course_no, score) VALUES ('2022010001', 'CS101', 88.5)",
            "INSERT IGNORE INTO grades (student_no, course_no, score) VALUES ('2022010001', 'CS102', 92.0)",
            "INSERT IGNORE INTO grades (student_no, course_no, score) VALUES ('2022010002', 'CS101', 76.0)",
            "INSERT IGNORE INTO grades (student_no, course_no, score) VALUES ('2022010002', 'MATH201', 85.0)",
            "INSERT IGNORE INTO attendance_records (student_no, attendance_time, attendance_type, check_mode) VALUES ('2022010001', '2026-07-08 08:30:00', 0, 0)"
        }
        : QStringList{
            "ALTER TABLE students ADD COLUMN college VARCHAR(50) DEFAULT NULL",
            "CREATE TABLE IF NOT EXISTS courses ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "course_no VARCHAR(32) NOT NULL UNIQUE,"
            "course_name VARCHAR(100) NOT NULL,"
            "credit FLOAT DEFAULT 0,"
            "semester VARCHAR(20) DEFAULT NULL,"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
            ")",
            "CREATE TABLE IF NOT EXISTS grades ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "student_no VARCHAR(32) NOT NULL,"
            "course_no VARCHAR(32) NOT NULL,"
            "score FLOAT DEFAULT 0,"
            "record_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "UNIQUE(student_no, course_no)"
            ")",
            "INSERT OR IGNORE INTO classes (class_name, department, grade) VALUES ('计算机2201', '计算机学院', '2022')",
            "INSERT OR IGNORE INTO classes (class_name, department, grade) VALUES ('计算机2202', '计算机学院', '2022')",
            "INSERT OR IGNORE INTO students (student_no, name, gender, ethnicity, id_card, class_name, college) VALUES ('2022010001', '张三', 1, '汉族', 'DEMO-ID-001', '计算机2201', '计算机学院')",
            "INSERT OR IGNORE INTO students (student_no, name, gender, ethnicity, id_card, class_name, college) VALUES ('2022010002', '李四', 2, '汉族', 'DEMO-ID-002', '计算机2202', '计算机学院')",
            "INSERT OR IGNORE INTO courses (course_no, course_name, credit, semester) VALUES ('CS101', '数据结构', 4.0, '2023-2024-1')",
            "INSERT OR IGNORE INTO courses (course_no, course_name, credit, semester) VALUES ('CS102', '操作系统', 3.5, '2023-2024-1')",
            "INSERT OR IGNORE INTO courses (course_no, course_name, credit, semester) VALUES ('CS103', '计算机网络', 3.0, '2023-2024-2')",
            "INSERT OR IGNORE INTO courses (course_no, course_name, credit, semester) VALUES ('MATH201', '高等数学', 5.0, '2023-2024-1')",
            "INSERT OR IGNORE INTO courses (course_no, course_name, credit, semester) VALUES ('ENG101', '大学英语', 3.0, '2023-2024-1')",
            "INSERT OR IGNORE INTO grades (student_no, course_no, score) VALUES ('2022010001', 'CS101', 88.5)",
            "INSERT OR IGNORE INTO grades (student_no, course_no, score) VALUES ('2022010001', 'CS102', 92.0)",
            "INSERT OR IGNORE INTO grades (student_no, course_no, score) VALUES ('2022010002', 'CS101', 76.0)",
            "INSERT OR IGNORE INTO grades (student_no, course_no, score) VALUES ('2022010002', 'MATH201', 85.0)",
            "INSERT OR IGNORE INTO attendance_records (student_no, attendance_time, attendance_type, check_mode) VALUES ('2022010001', '2026-07-08 08:30:00', 0, 0)"
        };

    for (const QString &migration : migrations) {
        query.exec(migration); // best-effort — may fail if column/table already exists
    }

    m_lastError.clear();
    return true;
}

QSqlDatabase DatabaseManager::database() const
{
    return m_database;
}

QString DatabaseManager::lastErrorText() const
{
    return m_lastError;
}
