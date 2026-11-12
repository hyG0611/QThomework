#ifndef STUDENTDIALOG_H
#define STUDENTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>
#include <QDialogButtonBox>

#include "../models/Student.h"

class StudentDialog : public QDialog
{
    Q_OBJECT
public:
    enum DialogMode {
        AddMode,
        EditMode
    };

    explicit StudentDialog(DialogMode mode, const Student &student, QWidget *parent = nullptr);

    static StudentDialog *addDialog(QWidget *parent = nullptr);
    static StudentDialog *editDialog(const Student &student, QWidget *parent = nullptr);

    Student student() const;

private:
    void setupUi();

    DialogMode m_mode;
    Student m_student;

    QLineEdit *m_studentNoEdit;
    QLineEdit *m_nameEdit;
    QComboBox *m_genderBox;
    QLineEdit *m_ethnicityEdit;
    QLineEdit *m_idCardEdit;
    QLineEdit *m_classEdit;
    QLineEdit *m_collegeEdit;
    QDialogButtonBox *m_buttonBox;
};

#endif
