#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QStackedWidget>
#include <QListWidget>
#include <QToolBar>
#include <QAction>

class StudentManageWidget;
class FaceAttendanceWidget;
class StatisticsWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onTestDatabase();
    void onTestCamera();
    void onNavigationClicked(QListWidgetItem *item);
    void onPageChanged(int index);

private:
    void setupUi();
    void setupToolBar();
    void appendLog(const QString &message);
    void switchPage(const QString &pageName);

    QTextEdit *logEdit;
    QLabel *statusLabel;
    QPushButton *dbButton;
    QPushButton *cameraButton;
    QStackedWidget *stack;
    QListWidget *navList;

    QToolBar *m_toolBar;
    QAction *m_actAdd;
    QAction *m_actEdit;
    QAction *m_actDelete;
    StudentManageWidget *m_studentPage;
    FaceAttendanceWidget *m_facePage;
    StatisticsWidget *m_statisticsPage;
};

#endif
