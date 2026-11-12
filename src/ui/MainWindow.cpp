#include "database/DatabaseManager.h"
#include "MainWindow.h"

#include "StudentManageWidget.h"
#include "FaceAttendanceWidget.h"
#include "StatisticsWidget.h"
#include "GradeManageWidget.h"

#include <QApplication>
#include <QCameraDevice>
#include <QMediaDevices>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QListWidgetItem>
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    appendLog("系统已启动，准备检查数据库与摄像头环境。");
    switchPage("学生管理");
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(central);

    navList = new QListWidget(this);
    navList->addItem("学生管理");
    navList->addItem("成绩管理");
    navList->addItem("人脸识别");
    navList->addItem("考勤统计");
    navList->addItem("运行状态");
    navList->setMaximumWidth(180);
    mainLayout->addWidget(navList);

    auto *contentLayout = new QVBoxLayout();
    stack = new QStackedWidget(this);

    m_studentPage = new StudentManageWidget(this);
    auto *gradePage = new GradeManageWidget(this);
    m_facePage = new FaceAttendanceWidget(this);
    m_statisticsPage = new StatisticsWidget(this);

    auto *statusPage = new QWidget(this);
    auto *statusLayout = new QVBoxLayout(statusPage);
    QGroupBox *group = new QGroupBox("运行状态", this);
    QVBoxLayout *groupLayout = new QVBoxLayout(group);
    statusLabel = new QLabel("等待检测...", this);
    statusLabel->setWordWrap(true);
    groupLayout->addWidget(statusLabel);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    dbButton = new QPushButton("测试 MySQL", this);
    cameraButton = new QPushButton("测试摄像头", this);
    buttonLayout->addWidget(dbButton);
    buttonLayout->addWidget(cameraButton);
    groupLayout->addLayout(buttonLayout);
    logEdit = new QTextEdit(this);
    logEdit->setReadOnly(true);
    logEdit->setMinimumHeight(220);
    groupLayout->addWidget(logEdit);
    statusLayout->addWidget(group);

    stack->addWidget(m_studentPage);
    stack->addWidget(gradePage);
    stack->addWidget(m_facePage);
    stack->addWidget(m_statisticsPage);
    stack->addWidget(statusPage);
    contentLayout->addWidget(stack);
    mainLayout->addLayout(contentLayout);

    setCentralWidget(central);
    resize(1080, 720);
    setWindowTitle("AttendanceSystem");

    setupToolBar();

    connect(dbButton, &QPushButton::clicked, this, &MainWindow::onTestDatabase);
    connect(cameraButton, &QPushButton::clicked, this, &MainWindow::onTestCamera);
    connect(navList, &QListWidget::itemClicked, this, &MainWindow::onNavigationClicked);
    connect(stack, &QStackedWidget::currentChanged, this, &MainWindow::onPageChanged);
}

void MainWindow::setupToolBar()
{
    m_toolBar = addToolBar("操作");
    m_actAdd = m_toolBar->addAction("新增学生");
    m_actEdit = m_toolBar->addAction("编辑学生");
    m_actDelete = m_toolBar->addAction("删除学生");

    connect(m_actAdd, &QAction::triggered, m_studentPage, &StudentManageWidget::onAdd);
    connect(m_actEdit, &QAction::triggered, m_studentPage, &StudentManageWidget::onEdit);
    connect(m_actDelete, &QAction::triggered, m_studentPage, &StudentManageWidget::onDelete);

    onPageChanged(stack->currentIndex());
}

void MainWindow::onPageChanged(int index)
{
    const bool isStudentPage = (index == 0);
    if (m_actAdd) m_actAdd->setEnabled(isStudentPage);
    if (m_actEdit) m_actEdit->setEnabled(isStudentPage);
    if (m_actDelete) m_actDelete->setEnabled(isStudentPage);
}

void MainWindow::appendLog(const QString &message)
{
    const QString stamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logEdit->append(QString("[%1] %2").arg(stamp, message));
}

void MainWindow::switchPage(const QString &pageName)
{
    if (pageName == "学生管理") {
        stack->setCurrentIndex(0);
    } else if (pageName == "成绩管理") {
        stack->setCurrentIndex(1);
    } else if (pageName == "人脸识别") {
        stack->setCurrentIndex(2);
        // Refresh student list when navigating to face recognition
        if (m_facePage) {
            m_facePage->refreshStudents();
        }
    } else if (pageName == "考勤统计") {
        stack->setCurrentIndex(3);
        // Refresh statistics when navigating to this page
        if (m_statisticsPage) {
            m_statisticsPage->onRefresh();
        }
    } else {
        stack->setCurrentIndex(4);
    }
}

void MainWindow::onNavigationClicked(QListWidgetItem *item)
{
    if (item) {
        switchPage(item->text());
    }
}

void MainWindow::onTestDatabase()
{
    appendLog("正在测试数据库连接...");

    const QStringList drivers = QSqlDatabase::drivers();
    appendLog(QString("Qt 可用数据库驱动：%1").arg(drivers.join(", ")));

    auto &manager = DatabaseManager::instance();
    if (!manager.initialize()) {
        const QString error = manager.lastErrorText();
        appendLog(QString("数据库连接失败: %1").arg(error));
        statusLabel->setText("数据库连接失败");
        QMessageBox::critical(this, "数据库错误", error);
        return;
    }

    const QString driverName = manager.database().driverName();
    appendLog(QString("数据库连接成功，当前驱动: %1").arg(driverName));
    statusLabel->setText(QString("数据库连接成功 (%1)").arg(driverName));

}

void MainWindow::onTestCamera()
{
    appendLog("正在检测摄像头设备...");
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        appendLog("未检测到摄像头设备。");
        statusLabel->setText("未检测到摄像头");
        QMessageBox::information(this, "摄像头检测", "当前环境中没有检测到可用摄像头。" );
        return;
    }

    const QCameraDevice camera = cameras.first();
    appendLog(QString("检测到摄像头: %1").arg(camera.description()));
    statusLabel->setText(QString("摄像头可用: %1").arg(camera.description()));
}
