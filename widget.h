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
#include "pxr/usd/usdGeom/boundable.h"
#include "pxr/usd/usdGeom/bboxCache.h"
#include "pxr/usd/usdGeom/imageable.h"
#include "pxr/usd/usdGeom/tokens.h"

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
    pxr::GfVec3d _cameraPos = {0,0,10};

private:
    bool initializeGLEngine();
    bool loadUsdStage(const std::string &path);
    bool setGLEngine();

protected:
    void wheelEvent(QWheelEvent *event) override; //滚轮事件
    void mousePressEvent(QMouseEvent *event) override; //鼠标按键按下
    // void mouseReleaseEvent(QMouseEvent *event) override; //鼠标按键抬起
    // void mouseDoubleClickEvent(QMouseEvent *event) override; //鼠标按键双击
    void mouseMoveEvent(QMouseEvent *event) override; //鼠标移动
private:
    QPoint _mousePos; // position of mouse
    pxr::GfVec3d _rotation{0,0,0}; // Rotation of this status
    pxr::GfVec3d _position{0,0,0}; // position of model

private:
    void setCamera(const pxr::GfVec3d &vec);
    void setCameraOffset(const pxr::GfVec3d &vec);
    void setRotation(const pxr::GfVec3d &vec);
    void setRotationOffset(const pxr::GfVec3d &vec);
    void setTranslate(const pxr::GfVec3d &vec);
    void setTranslateOffset(const pxr::GfVec3d &vec);

    pxr::UsdGeomXformOp getOrCreateXformOp(pxr::UsdGeomXformOp::Type type);

    // T must be type that XformOp required.
    // e.g. setXformOp(pxr::UsdGeomXformOp::TypeRotateXYZ, pxr::GfVec3d(120.0,120.0,120.0));
    template <typename T>
    void setXformOp(pxr::UsdGeomXformOp::Type type, const T &vec) {
        auto &op = getOrCreateXformOp(type);
        op.Set(vec);
        this->update();
    }

    template <typename T>
    void setXformOpOffset(pxr::UsdGeomXformOp::Type type, const T &vec) {
        auto &op = getOrCreateXformOp(type);
        T cur_vec;
        op.Get(&cur_vec);
        cur_vec += vec;
        op.Set(cur_vec);
        this->update();
    }
    
    // compute the extent.
    pxr::GfRange3d computeExtentRange(const pxr::UsdPrim &prim);

private:
    // control the relative translate / zoom operation of model. 
    double _scale_rate = 1.0;
};
#endif // WIDGET_H