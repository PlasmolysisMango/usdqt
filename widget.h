#ifndef WIDGET_H
#define WIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include <pxr/pxr.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/imaging/hd/engine.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include "pxr/imaging/hgi/tokens.h"

#include <iostream>

class OpenGLWidget : public QOpenGLWidget , protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget();
protected:
    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void resizeGL(int w, int h) override;
private:
    QOpenGLShaderProgram shaderProgram;
    // pxr::UsdImagingGLEngine m_engine;
};
#endif // WIDGET_H