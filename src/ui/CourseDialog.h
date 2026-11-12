#ifndef COURSEDIALOG_H
#define COURSEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>

#include "../models/Course.h"

class CourseDialog : public QDialog
{
    Q_OBJECT
public:
    enum DialogMode {
        AddMode,
        EditMode
    };

    explicit CourseDialog(DialogMode mode, const Course &course, QWidget *parent = nullptr);

    static CourseDialog *addDialog(QWidget *parent = nullptr);
    static CourseDialog *editDialog(const Course &course, QWidget *parent = nullptr);

    Course course() const;

private:
    void setupUi();

    DialogMode m_mode;
    Course m_course;

    QLineEdit *m_courseNoEdit;
    QLineEdit *m_courseNameEdit;
    QDoubleSpinBox *m_creditSpin;
    QLineEdit *m_semesterEdit;
    QDialogButtonBox *m_buttonBox;
};

#endif
