#include <QApplication>
#include <QMessageBox>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("AttendanceSystem");
    app.setOrganizationName("QThomework");

    MainWindow window;
    window.show();

    return app.exec();
}
