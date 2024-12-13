#include "mainwindow.h"
#include <QApplication>
#define BACKWARD_HAS_BFD 1
#include "backward.hpp"
int main(int argc, char *argv[])
{
    backward::SignalHandling sh; // Install a signal handler
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
