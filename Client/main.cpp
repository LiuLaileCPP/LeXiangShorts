#include "maindlg.h"

#include <QApplication>

#undef main
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainDlg w;
    w.show();
    return a.exec();
}
