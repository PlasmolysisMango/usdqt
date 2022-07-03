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
    loadUsdStage("E:\\github\\usd_atfx\\usdqt\\simpleShading.usda");
    // set engine
    setGLEngine();
}

void OpenGLWidget::paintGL()
{
    using namespace pxr;
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
    using namespace pxr;
    m_stage = pxr::UsdStage::Open(path);
    SdfPath rootPath;
    for (const auto& prim : m_stage->Traverse()) 
    {
        if (rootPath.IsEmpty()) {
            rootPath = prim.GetPath();
        }
        std::cout << prim.GetPath() << std::endl;
    }
    _xPrim = pxr::UsdGeomGprim(m_stage->GetPrimAtPath(rootPath));
    // adjust the camera. 
    auto range = computeExtentRange(_xPrim.GetPrim());
    _scale_rate = range.GetMax().GetLength();
    return m_stage != nullptr;
}

bool OpenGLWidget::setGLEngine()
{
    using namespace pxr;

    const UsdPrim* cameraPrim = nullptr;
    SdfPath cameraPath;
    UsdGeomCamera ucam;
    for (const auto& prim : m_stage->Traverse()) 
    {
        if (prim.GetTypeName() == "Camera")
        {
            cameraPrim = &prim;
            cameraPath = cameraPrim->GetPath();
            ucam = UsdGeomCamera(*cameraPrim);
            std::cout << "Camera found: " << cameraPath << "\n";
            break;
        }
    }
    // set camera if not found. 
    if (!cameraPrim)
    {
        std::cout << "No camera found on stage" << '\n';
        cameraPath = SdfPath("/cams/camera1");
        cameraPrim = &m_stage->DefinePrim(cameraPath, TfToken("Camera"));
        ucam = UsdGeomCamera(*cameraPrim);
        auto op = ucam.AddTranslateOp();
        // scale the camera pos
        _cameraPos *= _scale_rate;
        op.Set(_cameraPos);
    }

    m_engine->SetCameraPath(cameraPath);

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
    double length = 1.5;
    if(event->delta() > 0) {           
        std::cout << "zoom+" << std::endl;   
    } else {                          
        std::cout << "zoom-" << std::endl;   
        length *= -1;
    }
    setCameraOffset({0, 0, -length * _scale_rate});
}

void OpenGLWidget::setCamera(const pxr::GfVec3d &vec)
{
    using namespace pxr;
    // cameraPrim should be get in time. 
    auto cameraPrim = &m_stage->GetPrimAtPath(SdfPath("/cams/camera1"));
    auto translate = cameraPrim->GetAttribute(TfToken("xformOp:translate"));
    translate.Set(vec);
    auto ucam = UsdGeomCamera(*cameraPrim);
    GfCamera cam = ucam.GetCamera(1);
    const GfFrustum frustum = cam.GetFrustum();
    m_engine->SetCameraState(
        frustum.ComputeViewMatrix(),
        frustum.ComputeProjectionMatrix());
    this->update();
}

void OpenGLWidget::setCameraOffset(const pxr::GfVec3d &vec)
{
    using namespace pxr;
    _cameraPos += vec;
    setCamera(_cameraPos);
}

void OpenGLWidget::setRotation(const pxr::GfVec3d &vec)
{
    using namespace pxr;
    auto &op = getOrCreateXformOp(pxr::UsdGeomXformOp::TypeRotateXYZ);
    op.Set(vec);
    this->update();
}

void OpenGLWidget::setRotationOffset(const pxr::GfVec3d &vec)
{
    using namespace pxr;
    _rotation += vec;
    setRotation(_rotation);
}

void OpenGLWidget::setTranslate(const pxr::GfVec3d &vec)
{
    using namespace pxr;
    auto &op = getOrCreateXformOp(pxr::UsdGeomXformOp::TypeTranslate);
    op.Set(vec);
    
    this->update();
}

void OpenGLWidget::setTranslateOffset(const pxr::GfVec3d &vec)
{
    using namespace pxr;
    _position += vec;
    setTranslate(_position);
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event) 
{
    if (event->buttons() & Qt::LeftButton) {
        _mousePos = event->globalPos();
        std::cout << "press left" << std::endl;
    }
    if (event->buttons() & Qt::RightButton) {
        _mousePos = event->globalPos();
        std::cout << "press right" << std::endl;
    }
}

// void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event) 
// {
//     auto offset = event->globalPos() - _oldPos;
//     _oldPos = event->globalPos();
//     setCameraOffset(1, 1, 0);
//     std::cout << "release" << std::endl;
// }


// The screen pos: x+ is right, and y+ is down. 
// The model pos: x+ is right, y+ is up and z+ is front. 
// The rotation is right-hand rule. 
void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        auto offset = event->globalPos() - _mousePos;
        std::cout << offset.x() << " " << offset.y() << std::endl;
        _mousePos = event->globalPos();
        
        setRotationOffset({offset.y() * 1.0, offset.x() * 1.0, 0});
    }
    if (event->buttons() & Qt::RightButton) {
        auto offset = event->globalPos() - _mousePos;
        std::cout << offset.x() << " " << offset.y() << std::endl;
        _mousePos = event->globalPos();

        setTranslateOffset({offset.x() / 20.0 * _scale_rate, -offset.y() / 20.0 * _scale_rate, 0});
    }
}

pxr::UsdGeomXformOp OpenGLWidget::getOrCreateXformOp(pxr::UsdGeomXformOp::Type type)
{
    using namespace pxr;
    bool reset = _xPrim.GetResetXformStack();
    auto oplist = _xPrim.GetOrderedXformOps(&reset);
    for (auto &op: oplist) {
        if (op.GetOpType() == type) {
            return op;
        }
    }
    return std::move(_xPrim.AddXformOp(type));
}

pxr::GfRange3d OpenGLWidget::computeExtentRange(const pxr::UsdPrim &prim) 
{
    using namespace pxr;
    auto bb = UsdGeomBBoxCache(UsdTimeCode(), {UsdGeomTokensType().default_, UsdGeomTokensType().render, UsdGeomTokensType().guide, UsdGeomTokensType().proxy});
    auto res = bb.ComputeWorldBound(prim);
    auto range = res.ComputeAlignedRange();
    return range;
}
