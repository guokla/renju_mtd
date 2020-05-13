#include "mainwindow.h"
#include <QApplication>

QVector<Pos> rec;
uint32_t rec_pos;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
