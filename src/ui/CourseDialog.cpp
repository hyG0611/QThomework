#include "CourseDialog.h"

#include <QVBoxLayout>

CourseDialog::CourseDialog(DialogMode mode, const Course &course, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_course(course)
{
    setupUi();
    setWindowTitle(mode == AddMode ? "新增课程" : "编辑课程");
}

CourseDialog *CourseDialog::addDialog(QWidget *parent)
{
    return new CourseDialog(AddMode, Course(), parent);
}

CourseDialog *CourseDialog::editDialog(const Course &course, QWidget *parent)
{
    return new CourseDialog(EditMode, course, parent);
}

void CourseDialog::setupUi()
{
    auto *layout = new QVBoxLayout(this);

    auto *formLayout = new QFormLayout();
    m_courseNoEdit = new QLineEdit(this);
    m_courseNoEdit->setText(m_course.courseNo());
    m_courseNoEdit->setReadOnly(m_mode == EditMode);

    m_courseNameEdit = new QLineEdit(this);
    m_courseNameEdit->setText(m_course.courseName());

    m_creditSpin = new QDoubleSpinBox(this);
    m_creditSpin->setRange(0.5, 10.0);
    m_creditSpin->setDecimals(1);
    m_creditSpin->setSingleStep(0.5);
    m_creditSpin->setValue(static_cast<double>(m_course.credit()));

    m_semesterEdit = new QLineEdit(this);
    m_semesterEdit->setText(m_course.semester());
    m_semesterEdit->setPlaceholderText("如: 2023-2024-1");

    formLayout->addRow("课程号 *", m_courseNoEdit);
    formLayout->addRow("课程名 *", m_courseNameEdit);
    formLayout->addRow("学分", m_creditSpin);
    formLayout->addRow("学期", m_semesterEdit);

    layout->addLayout(formLayout);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(m_buttonBox);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &CourseDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &CourseDialog::reject);
}

Course CourseDialog::course() const
{
    Course result;
    result.setCourseNo(m_courseNoEdit->text().trimmed());
    result.setCourseName(m_courseNameEdit->text().trimmed());
    result.setCredit(static_cast<float>(m_creditSpin->value()));
    result.setSemester(m_semesterEdit->text().trimmed());
    return result;
}
