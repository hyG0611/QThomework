#ifndef GRADEDIALOG_H
#define GRADEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>

#include "../models/Grade.h"

class GradeDialog : public QDialog
{
    Q_OBJECT
public:
    enum DialogMode {
        AddMode,
        EditMode
    };

    explicit GradeDialog(DialogMode mode, const Grade &grade, QWidget *parent = nullptr);

    static GradeDialog *addDialog(QWidget *parent = nullptr);
    static GradeDialog *editDialog(const Grade &grade, QWidget *parent = nullptr);

    Grade grade() const;

private:
    void setupUi();

    DialogMode m_mode;
    Grade m_grade;

    QLineEdit *m_studentNoEdit;
    QLineEdit *m_courseNoEdit;
    QDoubleSpinBox *m_scoreSpin;
    QDialogButtonBox *m_buttonBox;
};

#endif
