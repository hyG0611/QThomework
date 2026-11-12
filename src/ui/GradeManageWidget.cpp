#include "GradeManageWidget.h"

#include "CourseDialog.h"
#include "GradeDialog.h"
#include "../database/DatabaseManager.h"
#include "../database/CourseDao.h"
#include "../database/GradeDao.h"
#include "../database/StudentDao.h"
#include "../models/Course.h"
#include "../models/Grade.h"

#include <QMessageBox>
#include <QHeaderView>
#include <QLabel>

GradeManageWidget::GradeManageWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadCourses();
    loadGrades();
}

void GradeManageWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    m_tabWidget = new QTabWidget(this);

    auto *courseTab = new QWidget();
    setupCourseTab(courseTab);
    m_tabWidget->addTab(courseTab, "课程管理");

    auto *gradeTab = new QWidget();
    setupGradeTab(gradeTab);
    m_tabWidget->addTab(gradeTab, "成绩管理");

    mainLayout->addWidget(m_tabWidget);
}

// ============================================================================
// Course Tab
// ============================================================================
void GradeManageWidget::setupCourseTab(QWidget *tab)
{
    auto *layout = new QVBoxLayout(tab);

    auto *toolbar = new QHBoxLayout();
    m_addCourseBtn = new QPushButton("新增课程", tab);
    m_editCourseBtn = new QPushButton("编辑课程", tab);
    m_deleteCourseBtn = new QPushButton("删除课程", tab);
    m_refreshCourseBtn = new QPushButton("刷新", tab);
    toolbar->addWidget(m_addCourseBtn);
    toolbar->addWidget(m_editCourseBtn);
    toolbar->addWidget(m_deleteCourseBtn);
    toolbar->addStretch();
    toolbar->addWidget(m_refreshCourseBtn);
    layout->addLayout(toolbar);

    m_courseTable = new QTableWidget(tab);
    m_courseTable->setColumnCount(4);
    m_courseTable->setHorizontalHeaderLabels({"课程号", "课程名", "学分", "学期"});
    m_courseTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_courseTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_courseTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(m_courseTable);

    connect(m_addCourseBtn, &QPushButton::clicked, this, &GradeManageWidget::onAddCourse);
    connect(m_editCourseBtn, &QPushButton::clicked, this, &GradeManageWidget::onEditCourse);
    connect(m_deleteCourseBtn, &QPushButton::clicked, this, &GradeManageWidget::onDeleteCourse);
    connect(m_refreshCourseBtn, &QPushButton::clicked, this, [this]() { loadCourses(); });
    connect(m_courseTable, &QTableWidget::itemSelectionChanged, this, &GradeManageWidget::updateCourseButtonStates);

    updateCourseButtonStates();
}

void GradeManageWidget::updateCourseButtonStates()
{
    const bool hasSelection = m_courseTable->currentRow() >= 0;
    m_editCourseBtn->setEnabled(hasSelection);
    m_deleteCourseBtn->setEnabled(hasSelection);
}

QString GradeManageWidget::currentSelectedCourseNo() const
{
    const int row = m_courseTable->currentRow();
    if (row < 0) return QString();
    const QTableWidgetItem *item = m_courseTable->item(row, 0);
    return item ? item->text() : QString();
}

void GradeManageWidget::loadCourses()
{
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        return;
    }

    CourseDao dao(db.database());
    const QList<Course> courses = dao.listCourses();

    m_courseTable->setRowCount(courses.size());
    for (int i = 0; i < courses.size(); ++i) {
        const Course &c = courses.at(i);
        m_courseTable->setItem(i, 0, new QTableWidgetItem(c.courseNo()));
        m_courseTable->setItem(i, 1, new QTableWidgetItem(c.courseName()));
        m_courseTable->setItem(i, 2, new QTableWidgetItem(QString::number(c.credit(), 'f', 1)));
        m_courseTable->setItem(i, 3, new QTableWidgetItem(c.semester()));
    }
    updateCourseButtonStates();
}

void GradeManageWidget::onAddCourse()
{
    QScopedPointer<CourseDialog> dialog(CourseDialog::addDialog(this));
    if (dialog->exec() != QDialog::Accepted) return;

    const Course course = dialog->course();
    if (course.courseNo().isEmpty() || course.courseName().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请填写课程号和课程名。");
        return;
    }

    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        QMessageBox::critical(this, "数据库错误", db.lastErrorText());
        return;
    }

    CourseDao dao(db.database());
    if (dao.addCourse(course)) {
        loadCourses();
    } else {
        QMessageBox::critical(this, "失败", "添加课程失败：" + dao.lastErrorText());
    }
}

void GradeManageWidget::onEditCourse()
{
    const QString courseNo = currentSelectedCourseNo();
    if (courseNo.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择一门课程。");
        return;
    }

    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        QMessageBox::critical(this, "数据库错误", db.lastErrorText());
        return;
    }

    CourseDao dao(db.database());
    Course course = dao.findByCourseNo(courseNo);
    if (course.courseNo().isEmpty()) {
        QMessageBox::warning(this, "错误", "未找到该课程。");
        return;
    }

    QScopedPointer<CourseDialog> dialog(CourseDialog::editDialog(course, this));
    if (dialog->exec() != QDialog::Accepted) return;

    const Course updated = dialog->course();
    if (updated.courseName().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "课程名不能为空。");
        return;
    }

    if (dao.updateCourse(updated)) {
        loadCourses();
    } else {
        QMessageBox::critical(this, "失败", "更新课程失败：" + dao.lastErrorText());
    }
}

void GradeManageWidget::onDeleteCourse()
{
    const QString courseNo = currentSelectedCourseNo();
    if (courseNo.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择一门课程。");
        return;
    }

    const int row = m_courseTable->currentRow();
    const QString name = m_courseTable->item(row, 1)->text();

    if (QMessageBox::question(this, "确认删除",
            QString("确定要删除课程 %1（%2）吗？\n注意：该课程的所有成绩也会被删除。").arg(name, courseNo),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        QMessageBox::critical(this, "数据库错误", db.lastErrorText());
        return;
    }

    CourseDao dao(db.database());
    if (dao.deleteCourse(courseNo)) {
        loadCourses();
        loadGrades();
    } else {
        QMessageBox::critical(this, "失败", "删除课程失败：" + dao.lastErrorText());
    }
}

// ============================================================================
// Grade Tab
// ============================================================================
void GradeManageWidget::setupGradeTab(QWidget *tab)
{
    auto *layout = new QVBoxLayout(tab);

    // Toolbar
    auto *toolbar = new QHBoxLayout();
    m_addGradeBtn = new QPushButton("新增成绩", tab);
    m_editGradeBtn = new QPushButton("编辑成绩", tab);
    m_deleteGradeBtn = new QPushButton("删除成绩", tab);
    m_refreshGradeBtn = new QPushButton("刷新", tab);
    toolbar->addWidget(m_addGradeBtn);
    toolbar->addWidget(m_editGradeBtn);
    toolbar->addWidget(m_deleteGradeBtn);
    toolbar->addStretch();
    toolbar->addWidget(m_refreshGradeBtn);
    layout->addLayout(toolbar);

    // Search area
    auto *searchLayout = new QHBoxLayout();
    m_searchStudentNoEdit = new QLineEdit(tab);
    m_searchStudentNoEdit->setPlaceholderText("学号");
    m_searchCourseEdit = new QLineEdit(tab);
    m_searchCourseEdit->setPlaceholderText("课程号/课程名");
    m_searchSemesterEdit = new QLineEdit(tab);
    m_searchSemesterEdit->setPlaceholderText("学期");
    m_searchButton = new QPushButton("搜索", tab);

    searchLayout->addWidget(new QLabel("学号", tab));
    searchLayout->addWidget(m_searchStudentNoEdit);
    searchLayout->addWidget(new QLabel("课程", tab));
    searchLayout->addWidget(m_searchCourseEdit);
    searchLayout->addWidget(new QLabel("学期", tab));
    searchLayout->addWidget(m_searchSemesterEdit);
    searchLayout->addWidget(m_searchButton);
    searchLayout->addStretch();
    layout->addLayout(searchLayout);

    // Table
    m_gradeTable = new QTableWidget(tab);
    m_gradeTable->setColumnCount(7);
    m_gradeTable->setHorizontalHeaderLabels({"学号", "姓名", "课程号", "课程名", "学分", "学期", "成绩"});
    m_gradeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_gradeTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_gradeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(m_gradeTable);

    connect(m_addGradeBtn, &QPushButton::clicked, this, &GradeManageWidget::onAddGrade);
    connect(m_editGradeBtn, &QPushButton::clicked, this, &GradeManageWidget::onEditGrade);
    connect(m_deleteGradeBtn, &QPushButton::clicked, this, &GradeManageWidget::onDeleteGrade);
    connect(m_refreshGradeBtn, &QPushButton::clicked, this, [this]() { loadGrades(); });
    connect(m_searchButton, &QPushButton::clicked, this, &GradeManageWidget::onSearchGrades);
    connect(m_gradeTable, &QTableWidget::itemSelectionChanged, this, &GradeManageWidget::updateGradeButtonStates);

    updateGradeButtonStates();
}

void GradeManageWidget::updateGradeButtonStates()
{
    const bool hasSelection = m_gradeTable->currentRow() >= 0;
    m_editGradeBtn->setEnabled(hasSelection);
    m_deleteGradeBtn->setEnabled(hasSelection);
}

qint64 GradeManageWidget::currentSelectedGradeId() const
{
    const int row = m_gradeTable->currentRow();
    if (row < 0) return -1;
    const QTableWidgetItem *item = m_gradeTable->item(row, 0);
    if (!item) return -1;
    return item->data(Qt::UserRole).toLongLong();
}

void GradeManageWidget::loadGrades()
{
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) return;

    GradeDao dao(db.database());
    QList<Grade> grades = dao.listGrades(m_searchStudentNoEdit->text().trimmed(),
                                         m_searchCourseEdit->text().trimmed(),
                                         m_searchSemesterEdit->text().trimmed());

    m_gradeTable->setRowCount(grades.size());
    for (int i = 0; i < grades.size(); ++i) {
        const Grade &g = grades.at(i);
        auto *noItem = new QTableWidgetItem(g.studentNo());
        noItem->setData(Qt::UserRole, g.id());
        m_gradeTable->setItem(i, 0, noItem);
        m_gradeTable->setItem(i, 1, new QTableWidgetItem(g.studentName()));
        m_gradeTable->setItem(i, 2, new QTableWidgetItem(g.courseNo()));
        m_gradeTable->setItem(i, 3, new QTableWidgetItem(g.courseName()));
        m_gradeTable->setItem(i, 4, new QTableWidgetItem(QString::number(g.credit(), 'f', 1)));
        m_gradeTable->setItem(i, 5, new QTableWidgetItem(g.semester()));
        m_gradeTable->setItem(i, 6, new QTableWidgetItem(QString::number(g.score(), 'f', 1)));
    }
    updateGradeButtonStates();
}

void GradeManageWidget::onAddGrade()
{
    QScopedPointer<GradeDialog> dialog(GradeDialog::addDialog(this));
    if (dialog->exec() != QDialog::Accepted) return;

    Grade grade = dialog->grade();
    if (grade.studentNo().isEmpty() || grade.courseNo().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请填写学号和课程号。");
        return;
    }

    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        QMessageBox::critical(this, "数据库错误", db.lastErrorText());
        return;
    }

    StudentDao studentDao(db.database());
    if (studentDao.findByStudentNo(grade.studentNo()).studentNo().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "学号不存在。");
        return;
    }
    CourseDao courseDao(db.database());
    if (courseDao.findByCourseNo(grade.courseNo()).courseNo().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "课程号不存在，请先在「课程管理」中添加课程。");
        return;
    }

    GradeDao dao(db.database());
    if (dao.addGrade(grade)) {
        loadGrades();
    } else {
        QMessageBox::critical(this, "失败", "添加成绩失败：" + dao.lastErrorText());
    }
}

void GradeManageWidget::onEditGrade()
{
    const qint64 id = currentSelectedGradeId();
    if (id < 0) {
        QMessageBox::information(this, "提示", "请先选择一行成绩记录。");
        return;
    }

    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        QMessageBox::critical(this, "数据库错误", db.lastErrorText());
        return;
    }

    GradeDao dao(db.database());
    Grade grade = dao.findById(id);
    if (grade.id() <= 0) {
        QMessageBox::warning(this, "错误", "未找到该成绩记录。");
        return;
    }

    QScopedPointer<GradeDialog> dialog(GradeDialog::editDialog(grade, this));
    if (dialog->exec() != QDialog::Accepted) return;

    Grade updated = dialog->grade();
    updated.setId(id);
    if (updated.studentNo().isEmpty() || updated.courseNo().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请填写学号和课程号。");
        return;
    }

    if (dao.updateGrade(updated)) {
        loadGrades();
    } else {
        QMessageBox::critical(this, "失败", "更新成绩失败：" + dao.lastErrorText());
    }
}

void GradeManageWidget::onDeleteGrade()
{
    const qint64 id = currentSelectedGradeId();
    if (id < 0) {
        QMessageBox::information(this, "提示", "请先选择一行成绩记录。");
        return;
    }

    if (QMessageBox::question(this, "确认删除", "确定要删除这条成绩记录吗？",
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        QMessageBox::critical(this, "数据库错误", db.lastErrorText());
        return;
    }

    GradeDao dao(db.database());
    if (dao.deleteGrade(id)) {
        loadGrades();
    } else {
        QMessageBox::critical(this, "失败", "删除成绩失败：" + dao.lastErrorText());
    }
}

void GradeManageWidget::onSearchGrades()
{
    loadGrades();
}
