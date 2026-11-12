#include "StatisticsWidget.h"

#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>

#include "../database/AttendanceDao.h"
#include "../database/DatabaseManager.h"

#include <QMessageBox>
#include <QDate>
#include <QHeaderView>

StatisticsWidget::StatisticsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    onRefresh();
}

void StatisticsWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    // ---- Toolbar ----
    auto *toolbar = new QHBoxLayout();
    dateEdit = new QDateEdit(QDate::currentDate(), this);
    dateEdit->setCalendarPopup(true);
    refreshButton = new QPushButton("刷新统计", this);
    toolbar->addWidget(dateEdit);
    toolbar->addWidget(refreshButton);
    toolbar->addStretch();
    mainLayout->addLayout(toolbar);

    // ---- Pie chart (overview) ----
    chartView = new QChartView(this);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(220);
    chartView->setMaximumHeight(280);
    mainLayout->addWidget(chartView);

    // ---- Attended students ----
    m_attendedLabel = new QLabel("✓ 已出勤学生", this);
    m_attendedLabel->setStyleSheet("font-size:15px; font-weight:bold; color:#2e7d32; margin-top:4px;");
    mainLayout->addWidget(m_attendedLabel);

    m_attendedTable = new QTableWidget(this);
    m_attendedTable->setColumnCount(6);
    m_attendedTable->setHorizontalHeaderLabels({"学号", "姓名", "性别", "班级", "学院", "打卡时间"});
    m_attendedTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_attendedTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_attendedTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_attendedTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_attendedTable->setMinimumHeight(120);
    mainLayout->addWidget(m_attendedTable, 2);

    // ---- Absent students ----
    m_absentLabel = new QLabel("✗ 未出勤学生", this);
    m_absentLabel->setStyleSheet("font-size:15px; font-weight:bold; color:#c62828; margin-top:8px;");
    mainLayout->addWidget(m_absentLabel);

    m_absentTable = new QTableWidget(this);
    m_absentTable->setColumnCount(5);
    m_absentTable->setHorizontalHeaderLabels({"学号", "姓名", "性别", "班级", "学院"});
    m_absentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_absentTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_absentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_absentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_absentTable->setMinimumHeight(120);
    mainLayout->addWidget(m_absentTable, 2);

    connect(refreshButton, &QPushButton::clicked, this, &StatisticsWidget::onRefresh);
}

void StatisticsWidget::refreshData()
{
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        QMessageBox::critical(this, "数据库错误", db.lastErrorText());
        return;
    }

    AttendanceDao dao(db.database());
    const QString dateText = dateEdit->date().toString("yyyy-MM-dd");

    // --- Pie chart: overview ---
    auto *series = new QPieSeries(this);
    const QList<QVariantMap> statsRows = dao.dailyStatistics(dateText);
    int attendedCount = 0;
    int absentCount = 0;
    if (!statsRows.isEmpty()) {
        attendedCount = statsRows.first()["attended_count"].toInt();
        absentCount = statsRows.first()["absent_count"].toInt();
    }
    series->append("已出勤", attendedCount);
    series->append("未出勤", absentCount);

    // Fixed indices: 0 = 已出勤(green), 1 = 未出勤(red)
    series->slices().at(0)->setBrush(QColor("#43a047"));
    series->slices().at(1)->setBrush(QColor("#ef5350"));

    auto *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QString("%1 考勤统计（共 %2 名学生）")
                        .arg(dateText)
                        .arg(attendedCount + absentCount));
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // Delete old chart to avoid memory leak
    QChart *oldChart = chartView->chart();
    chartView->setChart(chart);
    if (oldChart) {
        delete oldChart;
    }

    // --- Attended table ---
    const QList<QVariantMap> attendedList = dao.attendedStudents(dateText);
    m_attendedLabel->setText(QString("✓ 已出勤学生（%1 人）").arg(attendedList.size()));
    m_attendedTable->setRowCount(attendedList.size());
    for (int i = 0; i < attendedList.size(); ++i) {
        const QVariantMap &row = attendedList.at(i);
        const int gender = row["gender"].toInt();
        const QString genderText = gender == 1 ? "男" : gender == 2 ? "女" : "未知";
        const QString checkTime = row["first_check_time"].toString();

        m_attendedTable->setItem(i, 0, new QTableWidgetItem(row["student_no"].toString()));
        m_attendedTable->setItem(i, 1, new QTableWidgetItem(row["name"].toString()));
        m_attendedTable->setItem(i, 2, new QTableWidgetItem(genderText));
        m_attendedTable->setItem(i, 3, new QTableWidgetItem(row["class_name"].toString()));
        m_attendedTable->setItem(i, 4, new QTableWidgetItem(row["college"].toString()));
        m_attendedTable->setItem(i, 5, new QTableWidgetItem(checkTime));

        // Color rows light green for attended
        for (int col = 0; col < 6; ++col) {
            if (m_attendedTable->item(i, col)) {
                m_attendedTable->item(i, col)->setBackground(QColor("#e8f5e9"));
            }
        }
    }

    // --- Absent table ---
    const QList<QVariantMap> absentList = dao.absentStudents(dateText);
    m_absentLabel->setText(QString("✗ 未出勤学生（%1 人）").arg(absentList.size()));
    m_absentTable->setRowCount(absentList.size());
    for (int i = 0; i < absentList.size(); ++i) {
        const QVariantMap &row = absentList.at(i);
        const int gender = row["gender"].toInt();
        const QString genderText = gender == 1 ? "男" : gender == 2 ? "女" : "未知";

        m_absentTable->setItem(i, 0, new QTableWidgetItem(row["student_no"].toString()));
        m_absentTable->setItem(i, 1, new QTableWidgetItem(row["name"].toString()));
        m_absentTable->setItem(i, 2, new QTableWidgetItem(genderText));
        m_absentTable->setItem(i, 3, new QTableWidgetItem(row["class_name"].toString()));
        m_absentTable->setItem(i, 4, new QTableWidgetItem(row["college"].toString()));

        // Color rows light red for absent
        for (int col = 0; col < 5; ++col) {
            if (m_absentTable->item(i, col)) {
                m_absentTable->item(i, col)->setBackground(QColor("#ffebee"));
            }
        }
    }
}

void StatisticsWidget::onRefresh()
{
    refreshData();
}
