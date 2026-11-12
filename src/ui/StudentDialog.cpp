#include "StudentDialog.h"

#include <QVBoxLayout>
#include <QLabel>

StudentDialog::StudentDialog(DialogMode mode, const Student &student, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_student(student)
{
    setupUi();
    setWindowTitle(mode == AddMode ? "新增学生" : "编辑学生");
}

StudentDialog *StudentDialog::addDialog(QWidget *parent)
{
    return new StudentDialog(AddMode, Student(), parent);
}

StudentDialog *StudentDialog::editDialog(const Student &student, QWidget *parent)
{
    return new StudentDialog(EditMode, student, parent);
}

void StudentDialog::setupUi()
{
    auto *layout = new QVBoxLayout(this);

    auto *formLayout = new QFormLayout();
    m_studentNoEdit = new QLineEdit(this);
    m_studentNoEdit->setText(m_student.studentNo());
    m_studentNoEdit->setReadOnly(m_mode == EditMode);
    if (m_mode == EditMode) {
        m_studentNoEdit->setPlaceholderText("学号不可修改");
    }

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setText(m_student.name());

    m_genderBox = new QComboBox(this);
    m_genderBox->addItems({"未知", "男", "女"});
    m_genderBox->setCurrentIndex(m_student.gender());

    m_ethnicityEdit = new QLineEdit(this);
    m_ethnicityEdit->setText(m_student.ethnicity());

    m_idCardEdit = new QLineEdit(this);
    m_idCardEdit->setText(m_student.idCard());

    m_classEdit = new QLineEdit(this);
    m_classEdit->setText(m_student.className());

    m_collegeEdit = new QLineEdit(this);
    m_collegeEdit->setText(m_student.college());

    formLayout->addRow("学号 *", m_studentNoEdit);
    formLayout->addRow("姓名 *", m_nameEdit);
    formLayout->addRow("性别", m_genderBox);
    formLayout->addRow("民族", m_ethnicityEdit);
    formLayout->addRow("身份证号", m_idCardEdit);
    formLayout->addRow("班级", m_classEdit);
    formLayout->addRow("学院", m_collegeEdit);

    layout->addLayout(formLayout);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(m_buttonBox);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &StudentDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &StudentDialog::reject);
}

Student StudentDialog::student() const
{
    Student result;
    result.setStudentNo(m_studentNoEdit->text().trimmed());
    result.setName(m_nameEdit->text().trimmed());
    result.setGender(m_genderBox->currentIndex());
    result.setEthnicity(m_ethnicityEdit->text().trimmed());
    result.setIdCard(m_idCardEdit->text().trimmed());
    result.setClassName(m_classEdit->text().trimmed());
    result.setCollege(m_collegeEdit->text().trimmed());

    // Preserve existing face feature data when editing.
    if (m_mode == EditMode) {
        result.setFaceFeature(m_student.faceFeature());
        result.setFaceFeatureSize(m_student.faceFeatureSize());
    }

    return result;
}
