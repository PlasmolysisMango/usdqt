#include "widget.h"
#include "pxr/imaging/hd/engine.h"

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
}

OpenGLWidget::~OpenGLWidget()
{
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f,0.5f,0.9f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shaderProgram.create();
    // shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,":/gl.vert");
    // shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,":/gl.frag");
    shaderProgram.link();
    shaderProgram.enableAttributeArray(0);

    using namespace pxr;

    auto _hgi = Hgi::CreatePlatformDefaultHgi();
    HdDriver _hgiDriver;
    _hgiDriver.name = HgiTokens->renderDriver;
    _hgiDriver.driver = VtValue(_hgi.get());

    

    UsdImagingGLEngine glEngine(_hgiDriver);

    auto curRender = glEngine.GetCurrentRendererId();
    std::cout << curRender << '\n';
}

void OpenGLWidget::paintGL()
{
    shaderProgram.bind();

    float vertices[]={
         0.5f,  0.5f, 0.0f,  
         0.5f, -0.5f, 0.0f,  
        -0.5f,  0.5f, 0.0f, 
        -0.5f, -0.5f, 0.0f, 
    };
    shaderProgram.setAttributeArray(0,GL_FLOAT,vertices,3);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

void OpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0,0,w,h);
}
