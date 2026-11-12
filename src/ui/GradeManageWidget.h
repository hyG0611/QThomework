#ifndef GRADEMANAGEWIDGET_H
#define GRADEMANAGEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>

class GradeManageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GradeManageWidget(QWidget *parent = nullptr);

private:
    void setupUi();

    // Course management
    void setupCourseTab(QWidget *tab);
    void loadCourses();
    void onAddCourse();
    void onEditCourse();
    void onDeleteCourse();
    QString currentSelectedCourseNo() const;
    void updateCourseButtonStates();

    // Grade management
    void setupGradeTab(QWidget *tab);
    void loadGrades();
    void onAddGrade();
    void onEditGrade();
    void onDeleteGrade();
    void onSearchGrades();
    qint64 currentSelectedGradeId() const;
    void updateGradeButtonStates();

    QTabWidget *m_tabWidget;

    // Course tab widgets
    QTableWidget *m_courseTable;
    QPushButton *m_addCourseBtn;
    QPushButton *m_editCourseBtn;
    QPushButton *m_deleteCourseBtn;
    QPushButton *m_refreshCourseBtn;

    // Grade tab widgets
    QLineEdit *m_searchStudentNoEdit;
    QLineEdit *m_searchCourseEdit;
    QLineEdit *m_searchSemesterEdit;
    QPushButton *m_searchButton;
    QPushButton *m_addGradeBtn;
    QPushButton *m_editGradeBtn;
    QPushButton *m_deleteGradeBtn;
    QPushButton *m_refreshGradeBtn;
    QTableWidget *m_gradeTable;
};

#endif
