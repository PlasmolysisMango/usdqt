#ifndef WIDGET_H
#define WIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
// #include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QTimer>

#include <pxr/pxr.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/imaging/hd/engine.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include "pxr/imaging/hgi/tokens.h"

#include <iostream>
#include <string>

class OpenGLWidget : public QOpenGLWidget , protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget();
    void StartAnimating();

protected:
    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void resizeGL(int w, int h) override;

private:
    // QOpenGLShaderProgram shaderProgram;
    std::unique_ptr<pxr::UsdImagingGLEngine> m_engine;
    pxr::UsdStageRefPtr m_stage;
    double m_frame = 0.0;
    QTimer *m_timer;

private:
    bool initializeGLEngine();
    bool loadUsdStage(const std::string &path);
    bool setGLEngine();
};
#endif // WIDGET_H