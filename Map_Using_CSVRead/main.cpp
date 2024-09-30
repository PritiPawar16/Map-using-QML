#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyleSheet("QMessageBox { font-family: 'Segoe UI Semibold'; font-size: 12pt; }");
    MainWindow w;
    w.show();
    return a.exec();
}
