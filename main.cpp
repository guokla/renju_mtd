#include "mainwindow.h"
#include <QApplication>

QVector<Pos> root_mtd;
QVector<Pos> root_kill;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
