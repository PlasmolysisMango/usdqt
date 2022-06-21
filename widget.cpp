#include "widget.h"
#include "pxr/imaging/hd/engine.h"

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_timer = new QTimer(this);
    QObject::connect(m_timer, &QTimer::timeout, [this](){ 
        this->update();
    });
}

OpenGLWidget::~OpenGLWidget()
{
    m_timer->deleteLater();
}

void OpenGLWidget::StartAnimating()
{
    m_timer->start(10);
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    // glClearColor(0.0f,0.5f,0.9f,1.0f);
    glClear(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // initalize UsdImagingGLEngine
    initializeGLEngine();
    // load .usd file
    loadUsdStage("E:\\github\\Usd_atfx\\qt\\usdqt\\stage.usda");
    // set engine
    setGLEngine();
}

void OpenGLWidget::paintGL()
{
    using namespace pxr;
    // static bool once = false;
    // auto prim = m_stage->GetPrimAtPath(SdfPath("/cube/mesh"));
    // UsdGeomGprim xprim(prim);
    // if (!once) {
    //     auto op = xprim.AddRotateXYZOp();
    //     op.Set(GfVec3f(120.0f, 120.0f, 120.0f));
    //     once = true;
    // }
    // const UsdPrim& pseudoRoot = m_stage->GetPseudoRoot();
    UsdPrim pseudoRoot = _xPrim.GetPrim();
    UsdImagingGLRenderParams renderParams;
    if (m_frame > m_stage->GetEndTimeCode()) {
        m_frame = 0.0;
    }
    renderParams.frame = m_frame;
    m_engine->Render(pseudoRoot, renderParams);
    ++m_frame;
}

void OpenGLWidget::resizeGL(int w, int h)
{
    // resize the viewport
    m_engine->SetRenderViewport({0,0,(double)w,(double)h});
}

bool OpenGLWidget::initializeGLEngine()
{
    // Initialize UsdImagingGLEngine
    m_engine = std::make_unique<pxr::UsdImagingGLEngine>();
    // use default constructor of HdDriver to connect OpenGL context of QT. 
    auto curRender = m_engine->GetCurrentRendererId();
    std::cout << curRender << std::endl;
    bool isValid = m_engine->IsHydraEnabled();
    std::cout << (isValid ? "Yes" : "No") << std::endl;
    return isValid;
    
}

bool OpenGLWidget::loadUsdStage(const std::string &path)
{
    m_stage = pxr::UsdStage::Open(path);
    for (const auto& prim : m_stage->Traverse()) 
    {
        std::cout << prim.GetPath() << std::endl;
    }
    _xPrim = pxr::UsdGeomGprim(m_stage->GetPrimAtPath(pxr::SdfPath("/cube/mesh")));
    return m_stage != nullptr;
}

bool OpenGLWidget::setGLEngine()
{
    using namespace pxr;

    const UsdPrim* cameraPrim = nullptr;
    for (const auto& prim : m_stage->Traverse()) 
    {
        if (prim.GetTypeName() == "Camera")
        {
            cameraPrim = &prim;
            std::cout << "Camera found: " << cameraPrim->GetPath() << "\n";
            break;
        }
    }
    if (!cameraPrim)
    {
        std::cout << "No camera found on stage" << '\n';
        exit(1);
    }
    m_engine->SetCameraPath(cameraPrim->GetPath());
    // auto color = prim.GetAttribute(TfToken("primvars:displayColor"));
    // color.Set(VtArray<GfVec3f>(1, GfVec3f(0,1,0)));

    auto ucam = UsdGeomCamera(*cameraPrim);


    // auto attr = ucam.GetFocusDistanceAttr();
    // attr.Set(20);

    GfCamera cam = ucam.GetCamera(1);
    const GfFrustum frustum = cam.GetFrustum();
    const GfVec3d cameraPos = frustum.GetPosition();
    _cameraPos = cameraPos;

    const GfVec4f SCENE_AMBIENT(0.01f, 0.01f, 0.01f, 1.0f);
    const GfVec4f SPECULAR_DEFAULT(0.1f, 0.1f, 0.1f, 1.0f);
    const GfVec4f AMBIENT_DEFAULT(0.2f, 0.2f, 0.2f, 1.0f);
    const float   SHININESS_DEFAULT(32.0);

    GlfSimpleLight cameraLight(
        GfVec4f(cameraPos[0], cameraPos[1], cameraPos[2], 1.0f));
    cameraLight.SetAmbient(SCENE_AMBIENT);

    const GlfSimpleLightVector lights({cameraLight});

    // Make default material and lighting match usdview's defaults... we expect 
    // GlfSimpleMaterial to go away soon, so not worth refactoring for sharing
    GlfSimpleMaterial material;
    material.SetAmbient(AMBIENT_DEFAULT);
    material.SetSpecular(SPECULAR_DEFAULT);
    material.SetShininess(SHININESS_DEFAULT);

    m_engine->SetLightingState(lights, material, SCENE_AMBIENT);

    m_engine->SetRendererAov(HdAovTokens->color);
    m_engine->SetCameraState(
        frustum.ComputeViewMatrix(),
        frustum.ComputeProjectionMatrix());
    //glEngine.SetCameraPath(cameraPrim->GetPath());
    m_engine->SetRenderViewport(GfVec4d(0, 0, 800, 800));
    const GfVec4f CLEAR_COLOR(0.0f);

    const GLfloat CLEAR_DEPTH[1] = { 1.0f };
    // m_rootPrim = m_stage->GetPseudoRoot();

    m_engine->SetEnablePresentation(true);

    return m_engine->IsHydraEnabled();
}

void OpenGLWidget::wheelEvent(QWheelEvent *event)
{
    // static int z_zoom = 20;
    int length = 2;
    if(event->delta() > 0) {           
        std::cout << "zoom+" << std::endl;   
        // z_zoom -= length;
    } else {                          
        std::cout << "zoom-" << std::endl;   
        length *= -1;
    }
    setCameraOffset(0, 0, -length);
    setRotation({120.0,120.0,120.0});
    setTranslate({0,1,0});
}

void OpenGLWidget::setCameraOffset(int x, int y, int z)
{
    using namespace pxr;
    // cameraPrim should be get in time. 
    auto cameraPrim = &m_stage->GetPrimAtPath(SdfPath("/cams/camera1"));
    auto translate = cameraPrim->GetAttribute(TfToken("xformOp:translate"));
    _cameraPos[0] += x;
    _cameraPos[1] += y;
    _cameraPos[2] += z;
    translate.Set(_cameraPos);
    auto ucam = UsdGeomCamera(*cameraPrim);
    GfCamera cam = ucam.GetCamera(1);
    const GfFrustum frustum = cam.GetFrustum();
    m_engine->SetCameraState(
        frustum.ComputeViewMatrix(),
        frustum.ComputeProjectionMatrix());
    this->update();
}

void OpenGLWidget::setRotation(const pxr::GfVec3f &vec)
{
    using namespace pxr;
    _xPrim.ClearXformOpOrder();
    auto op = _xPrim.AddRotateXYZOp();
    op.Set(vec);
    this->update();
}

void OpenGLWidget::setTranslate(const pxr::GfVec3d &vec)
{
    using namespace pxr;
    _xPrim.ClearXformOpOrder();
    auto op = _xPrim.AddTranslateOp();
    op.Set(vec);
    this->update();
}

// void OpenGLWidget::mousePressEvent(QMouseEvent *event) 
// {
//     if (event->buttons() & Qt::MiddleButton) {
//         _oldPos = event->globalPos();
//         std::cout << "press" << std::endl;
//     }
// }

// void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event) 
// {
//     auto offset = event->globalPos() - _oldPos;
//     _oldPos = event->globalPos();
//     setCameraOffset(1, 1, 0);
//     std::cout << "release" << std::endl;
// }

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::MiddleButton) {
        auto offset = event->globalPos() - _oldPos;
         std::cout << offset.x() << " " << offset.y() << std::endl;
        _oldPos = event->globalPos();
        int x, y;
        x = y = 1;
        if (offset.x() > 0)
            x = -x;
        if (offset.y() > 0)
            y = -y;
        setCameraOffset(x, y, 0);
    }
}