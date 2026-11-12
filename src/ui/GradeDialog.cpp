#include "GradeDialog.h"

#include <QVBoxLayout>

GradeDialog::GradeDialog(DialogMode mode, const Grade &grade, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_grade(grade)
{
    setupUi();
    setWindowTitle(mode == AddMode ? "新增成绩" : "编辑成绩");
}

GradeDialog *GradeDialog::addDialog(QWidget *parent)
{
    return new GradeDialog(AddMode, Grade(), parent);
}

GradeDialog *GradeDialog::editDialog(const Grade &grade, QWidget *parent)
{
    return new GradeDialog(EditMode, grade, parent);
}

void GradeDialog::setupUi()
{
    auto *layout = new QVBoxLayout(this);

    auto *formLayout = new QFormLayout();
    m_studentNoEdit = new QLineEdit(this);
    m_studentNoEdit->setText(m_grade.studentNo());

    m_courseNoEdit = new QLineEdit(this);
    m_courseNoEdit->setText(m_grade.courseNo());

    m_scoreSpin = new QDoubleSpinBox(this);
    m_scoreSpin->setRange(0.0, 100.0);
    m_scoreSpin->setDecimals(1);
    m_scoreSpin->setValue(static_cast<double>(m_grade.score()));

    formLayout->addRow("学号 *", m_studentNoEdit);
    formLayout->addRow("课程号 *", m_courseNoEdit);
    formLayout->addRow("成绩 *", m_scoreSpin);

    layout->addLayout(formLayout);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(m_buttonBox);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &GradeDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &GradeDialog::reject);
}

Grade GradeDialog::grade() const
{
    Grade result;
    result.setId(m_grade.id());
    result.setStudentNo(m_studentNoEdit->text().trimmed());
    result.setCourseNo(m_courseNoEdit->text().trimmed());
    result.setScore(static_cast<float>(m_scoreSpin->value()));
    return result;
}
