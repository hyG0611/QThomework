#include "StudentDao.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <utility>

StudentDao::StudentDao(QSqlDatabase database)
    : m_database(std::move(database))
{
}

QString StudentDao::lastErrorText() const
{
    return m_lastError;
}

bool StudentDao::addStudent(const Student &student)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO students (student_no, name, gender, ethnicity, id_card, face_feature, face_feature_size, class_name, college)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    query.addBindValue(student.studentNo());
    query.addBindValue(student.name());
    query.addBindValue(student.gender());
    query.addBindValue(student.ethnicity());
    query.addBindValue(student.idCard());
    query.addBindValue(student.faceFeature());
    query.addBindValue(student.faceFeatureSize());
    query.addBindValue(student.className());
    query.addBindValue(student.college());
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    m_lastError.clear();
    return true;
}

bool StudentDao::updateStudent(const Student &student)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE students
        SET name = ?,
            gender = ?,
            ethnicity = ?,
            id_card = ?,
            face_feature = ?,
            face_feature_size = ?,
            class_name = ?,
            college = ?
        WHERE student_no = ?
    )");
    query.addBindValue(student.name());
    query.addBindValue(student.gender());
    query.addBindValue(student.ethnicity());
    query.addBindValue(student.idCard());
    query.addBindValue(student.faceFeature());
    query.addBindValue(student.faceFeatureSize());
    query.addBindValue(student.className());
    query.addBindValue(student.college());
    query.addBindValue(student.studentNo());
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    m_lastError.clear();
    return true;
}

bool StudentDao::deleteStudent(const QString &studentNo)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM students WHERE student_no = ?");
    query.addBindValue(studentNo);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    m_lastError.clear();
    return true;
}

QList<Student> StudentDao::listStudents(const QString &studentNo, const QString &name, const QString &college)
{
    QSqlQuery query(m_database);
    QString sql = "SELECT id, student_no, name, gender, ethnicity, id_card, face_feature, face_feature_size, class_name, college, created_at, updated_at FROM students";
    QStringList conditions;
    QList<QVariant> bindValues;

    if (!studentNo.isEmpty()) {
        conditions.append("student_no LIKE ?");
        bindValues.append(QString("%%%1%").arg(studentNo));
    }
    if (!name.isEmpty()) {
        conditions.append("name LIKE ?");
        bindValues.append(QString("%%%1%").arg(name));
    }
    if (!college.isEmpty()) {
        conditions.append("college LIKE ?");
        bindValues.append(QString("%%%1%").arg(college));
    }

    if (!conditions.isEmpty()) {
        sql += " WHERE " + conditions.join(" AND ");
    }
    query.prepare(sql);
    for (const QVariant &value : bindValues) {
        query.addBindValue(value);
    }

    QList<Student> students;
    if (!query.exec()) {
        return students;
    }

    while (query.next()) {
        Student student;
        student.setId(query.value("id").toInt());
        student.setStudentNo(query.value("student_no").toString());
        student.setName(query.value("name").toString());
        student.setGender(query.value("gender").toInt());
        student.setEthnicity(query.value("ethnicity").toString());
        student.setIdCard(query.value("id_card").toString());
        student.setFaceFeature(query.value("face_feature").toByteArray());
        student.setFaceFeatureSize(query.value("face_feature_size").toInt());
        student.setClassName(query.value("class_name").toString());
        student.setCollege(query.value("college").toString());
        student.setCreatedAt(query.value("created_at").toString());
        student.setUpdatedAt(query.value("updated_at").toString());
        students.append(student);
    }
    return students;
}

Student StudentDao::findByStudentNo(const QString &studentNo)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT id, student_no, name, gender, ethnicity, id_card, face_feature, face_feature_size, class_name, college, created_at, updated_at FROM students WHERE student_no = ?");
    query.addBindValue(studentNo);
    if (!query.exec() || !query.next()) {
        return Student();
    }

    Student student;
    student.setId(query.value("id").toInt());
    student.setStudentNo(query.value("student_no").toString());
    student.setName(query.value("name").toString());
    student.setGender(query.value("gender").toInt());
    student.setEthnicity(query.value("ethnicity").toString());
    student.setIdCard(query.value("id_card").toString());
    student.setFaceFeature(query.value("face_feature").toByteArray());
    student.setFaceFeatureSize(query.value("face_feature_size").toInt());
    student.setClassName(query.value("class_name").toString());
    student.setCollege(query.value("college").toString());
    student.setCreatedAt(query.value("created_at").toString());
    student.setUpdatedAt(query.value("updated_at").toString());
    return student;
}
