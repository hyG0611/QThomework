#ifndef STUDENTMANAGEWIDGET_H
#define STUDENTMANAGEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class Student;

class StudentManageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StudentManageWidget(QWidget *parent = nullptr);

public slots:
    void onRefresh();
    void onAdd();
    void onEdit();
    void onDelete();
    void onSearch();
    void onEnrollFace();

private:
    void setupUi();
    void loadStudents();
    void updateButtonStates();
    Student currentSelectedStudent() const;

    QLineEdit *m_searchStudentNoEdit;
    QLineEdit *m_searchNameEdit;
    QLineEdit *m_searchCollegeEdit;
    QPushButton *m_searchButton;
    QPushButton *m_refreshButton;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    QPushButton *m_enrollFaceButton;
    QTableWidget *m_table;
};

#endif
