#include "widget.h"
#include <iostream>
#include <string>

#include <QApplication>

using namespace pxr;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OpenGLWidget w;

    w.makeCurrent();

    auto _hgi = Hgi::CreatePlatformDefaultHgi();
    HdDriver _hgiDriver;
    _hgiDriver.name = HgiTokens->renderDriver;
    _hgiDriver.driver = VtValue(_hgi.get());

    UsdImagingGLEngine glEngine(_hgiDriver);

    auto curRender = glEngine.GetCurrentRendererId();
    std::cout << curRender << '\n';

    w.show();
    return a.exec();
}
