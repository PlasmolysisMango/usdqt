#ifndef WIDGET_H
#define WIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
// #include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QTimer>
#include <QMouseEvent>

#include <pxr/pxr.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/imaging/hd/engine.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include "pxr/imaging/hgi/tokens.h"
#include "pxr/usd/usdGeom/gprim.h"

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
    pxr::UsdGeomGprim _xPrim;

    // camera position
    pxr::GfVec3d _cameraPos;

private:
    bool initializeGLEngine();
    bool loadUsdStage(const std::string &path);
    bool setGLEngine();

protected:
    void wheelEvent(QWheelEvent *event) override; //滚轮事件
    // void mousePressEvent(QMouseEvent *event) override; //鼠标按键按下
    // void mouseReleaseEvent(QMouseEvent *event) override; //鼠标按键抬起
    // void mouseDoubleClickEvent(QMouseEvent *event) override; //鼠标按键双击
    void mouseMoveEvent(QMouseEvent *event) override; //鼠标移动
private:
    QPoint _oldPos; // old position

private:
    void setCameraOffset(int x, int y, int z);
    void setRotation(const pxr::GfVec3f &vec);
    void setTranslate(const pxr::GfVec3d &vec);

};
#endif // WIDGET_H