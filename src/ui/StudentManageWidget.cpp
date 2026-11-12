#include "StudentManageWidget.h"

#include "StudentDialog.h"
#include "FaceEnrollmentDialog.h"
#include "../database/DatabaseManager.h"
#include "../database/StudentDao.h"
#include "../models/Student.h"

#include <QMessageBox>
#include <QHeaderView>
#include <QLabel>

StudentManageWidget::StudentManageWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    onRefresh();
}

void StudentManageWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    // Toolbar
    auto *toolbarLayout = new QHBoxLayout();
    m_addButton = new QPushButton("新增", this);
    m_editButton = new QPushButton("编辑", this);
    m_deleteButton = new QPushButton("删除", this);
    m_enrollFaceButton = new QPushButton("人脸录入", this);
    m_refreshButton = new QPushButton("刷新", this);
    toolbarLayout->addWidget(m_addButton);
    toolbarLayout->addWidget(m_editButton);
    toolbarLayout->addWidget(m_deleteButton);
    toolbarLayout->addWidget(m_enrollFaceButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_refreshButton);
    mainLayout->addLayout(toolbarLayout);

    // Search area
    auto *searchLayout = new QHBoxLayout();
    m_searchStudentNoEdit = new QLineEdit(this);
    m_searchStudentNoEdit->setPlaceholderText("学号");
    m_searchNameEdit = new QLineEdit(this);
    m_searchNameEdit->setPlaceholderText("姓名");
    m_searchCollegeEdit = new QLineEdit(this);
    m_searchCollegeEdit->setPlaceholderText("学院");
    m_searchButton = new QPushButton("搜索", this);

    searchLayout->addWidget(new QLabel("学号", this));
    searchLayout->addWidget(m_searchStudentNoEdit);
    searchLayout->addWidget(new QLabel("姓名", this));
    searchLayout->addWidget(m_searchNameEdit);
    searchLayout->addWidget(new QLabel("学院", this));
    searchLayout->addWidget(m_searchCollegeEdit);
    searchLayout->addWidget(m_searchButton);
    searchLayout->addStretch();
    mainLayout->addLayout(searchLayout);

    // Table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"学号", "姓名", "性别", "班级", "学院", "人脸特征"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mainLayout->addWidget(m_table);

    connect(m_refreshButton, &QPushButton::clicked, this, &StudentManageWidget::onRefresh);
    connect(m_addButton, &QPushButton::clicked, this, &StudentManageWidget::onAdd);
    connect(m_editButton, &QPushButton::clicked, this, &StudentManageWidget::onEdit);
    connect(m_deleteButton, &QPushButton::clicked, this, &StudentManageWidget::onDelete);
    connect(m_enrollFaceButton, &QPushButton::clicked, this, &StudentManageWidget::onEnrollFace);
    connect(m_searchButton, &QPushButton::clicked, this, &StudentManageWidget::onSearch);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &StudentManageWidget::updateButtonStates);

    updateButtonStates();
}

void StudentManageWidget::updateButtonStates()
{
    const bool hasSelection = m_table->currentRow() >= 0;
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
    m_enrollFaceButton->setEnabled(hasSelection);
}

void StudentManageWidget::loadStudents()
{
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        QMessageBox::critical(this, "数据库错误", db.lastErrorText().isEmpty() ? "数据库初始化失败" : db.lastErrorText());
        return;
    }

    StudentDao dao(db.database());
    QList<Student> students = dao.listStudents(m_searchStudentNoEdit->text().trimmed(),
                                               m_searchNameEdit->text().trimmed(),
                                               m_searchCollegeEdit->text().trimmed());

    m_table->setRowCount(students.size());
    for (int i = 0; i < students.size(); ++i) {
        const Student &s = students.at(i);
        m_table->setItem(i, 0, new QTableWidgetItem(s.studentNo()));
        m_table->setItem(i, 1, new QTableWidgetItem(s.name()));
        m_table->setItem(i, 2, new QTableWidgetItem(s.gender() == 1 ? "男" : s.gender() == 2 ? "女" : "未知"));
        m_table->setItem(i, 3, new QTableWidgetItem(s.className()));
        m_table->setItem(i, 4, new QTableWidgetItem(s.college()));
        m_table->setItem(i, 5, new QTableWidgetItem(s.faceFeatureSize() > 0 ? "已录入" : "未录入"));
    }

    updateButtonStates();
}

Student StudentManageWidget::currentSelectedStudent() const
{
    const int row = m_table->currentRow();
    if (row < 0) {
        return Student();
    }

    Student student;
    student.setStudentNo(m_table->item(row, 0)->text());
    student.setName(m_table->item(row, 1)->text());
    const QString genderText = m_table->item(row, 2)->text();
    student.setGender(genderText == "男" ? 1 : genderText == "女" ? 2 : 0);
    student.setClassName(m_table->item(row, 3)->text());
    student.setCollege(m_table->item(row, 4)->text());
    return student;
}

void StudentManageWidget::onEnrollFace()
{
    const int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "提示", "请先选择一名学生。");
        return;
    }

    const QString studentNo = m_table->item(row, 0)->text();
    const QString studentName = m_table->item(row, 1)->text();

    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        QMessageBox::critical(this, "数据库错误",
                              db.lastErrorText().isEmpty() ? "数据库初始化失败" : db.lastErrorText());
        return;
    }

    StudentDao dao(db.database());
    Student student = dao.findByStudentNo(studentNo);
    if (student.studentNo().isEmpty()) {
        QMessageBox::warning(this, "错误", "未找到该学生记录。");
        return;
    }

    // Open face enrollment dialog
    FaceEnrollmentDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QByteArray feature = dialog.faceFeature();
    if (feature.isEmpty()) {
        QMessageBox::warning(this, "录入失败", "未获取到有效的人脸特征数据。");
        return;
    }

    // Save face feature to the student record
    student.setFaceFeature(feature);
    student.setFaceFeatureSize(feature.size());

    if (dao.updateStudent(student)) {
        QMessageBox::information(this, "录入成功",
                                 QString("学生 %1（%2）的人脸信息已录入成功！")
                                     .arg(studentName, studentNo));
        onRefresh();
    } else {
        const QString errorText = dao.lastErrorText();
        QMessageBox::critical(this, "录入失败",
                              QString("人脸特征保存失败：%1")
                                  .arg(errorText.isEmpty() ? "未知错误" : errorText));
    }
}

void StudentManageWidget::onRefresh()
{
    loadStudents();
}

void StudentManageWidget::onSearch()
{
    loadStudents();
}

void StudentManageWidget::onAdd()
{
    QScopedPointer<StudentDialog> dialog(StudentDialog::addDialog(this));
    if (dialog->exec() != QDialog::Accepted) {
        return;
    }

    Student student = dialog->student();
    if (student.studentNo().isEmpty() || student.name().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请至少填写学号和姓名。");
        return;
    }

    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        QMessageBox::critical(this, "数据库错误", db.lastErrorText().isEmpty() ? "数据库初始化失败" : db.lastErrorText());
        return;
    }

    StudentDao dao(db.database());
    if (dao.addStudent(student)) {
        QMessageBox::information(this, "成功", "学生信息已添加。");
        onRefresh();
    } else {
        const QString errorText = dao.lastErrorText();
        QMessageBox::critical(this, "失败", QString("添加学生失败：%1").arg(errorText.isEmpty() ? "未知错误" : errorText));
    }
}

void StudentManageWidget::onEdit()
{
    const int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "提示", "请先选择一行学生记录。");
        return;
    }

    const QString studentNo = m_table->item(row, 0)->text();
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        QMessageBox::critical(this, "数据库错误", db.lastErrorText().isEmpty() ? "数据库初始化失败" : db.lastErrorText());
        return;
    }

    StudentDao dao(db.database());
    Student student = dao.findByStudentNo(studentNo);
    if (student.studentNo().isEmpty()) {
        QMessageBox::warning(this, "错误", "未找到该学生记录。");
        return;
    }

    QScopedPointer<StudentDialog> dialog(StudentDialog::editDialog(student, this));
    if (dialog->exec() != QDialog::Accepted) {
        return;
    }

    Student updated = dialog->student();
    if (updated.name().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "姓名不能为空。");
        return;
    }

    if (dao.updateStudent(updated)) {
        QMessageBox::information(this, "成功", "学生信息已更新。");
        onRefresh();
    } else {
        const QString errorText = dao.lastErrorText();
        QMessageBox::critical(this, "失败", QString("更新学生失败：%1").arg(errorText.isEmpty() ? "未知错误" : errorText));
    }
}

void StudentManageWidget::onDelete()
{
    const int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "提示", "请先选择一行学生记录。");
        return;
    }

    const QString studentNo = m_table->item(row, 0)->text();
    const QString name = m_table->item(row, 1)->text();
    const int ret = QMessageBox::question(this, "确认删除",
                                          QString("确定要删除学生 %1（%2）吗？").arg(name, studentNo),
                                          QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) {
        return;
    }

    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        QMessageBox::critical(this, "数据库错误", db.lastErrorText().isEmpty() ? "数据库初始化失败" : db.lastErrorText());
        return;
    }

    StudentDao dao(db.database());
    if (dao.deleteStudent(studentNo)) {
        QMessageBox::information(this, "成功", "学生已删除。");
        onRefresh();
    } else {
        const QString errorText = dao.lastErrorText();
        QMessageBox::critical(this, "失败", QString("删除学生失败：%1").arg(errorText.isEmpty() ? "未知错误" : errorText));
    }
}
