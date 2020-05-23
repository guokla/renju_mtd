#include "mainwindow.h"
#include <QApplication>

QVector<Pos> root;
int Count, ABcut, tag, sto;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
