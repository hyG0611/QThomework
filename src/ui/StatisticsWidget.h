#ifndef STATISTICSWIDGET_H
#define STATISTICSWIDGET_H

#include <QWidget>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>

class StatisticsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StatisticsWidget(QWidget *parent = nullptr);

public slots:
    void onRefresh();

private:
    void setupUi();
    void refreshData();

    QDateEdit *dateEdit;
    QPushButton *refreshButton;
    QChartView *chartView;
    QLabel *m_attendedLabel;
    QTableWidget *m_attendedTable;
    QLabel *m_absentLabel;
    QTableWidget *m_absentTable;
};

#endif
