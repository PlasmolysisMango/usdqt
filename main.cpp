#include "widget.h"
#include <iostream>
#include <string>

#include <QApplication>

using namespace pxr;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OpenGLWidget w;

    // w.StartAnimating();
    w.show();
    return a.exec();
}
