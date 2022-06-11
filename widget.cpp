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
    const UsdPrim& pseudoRoot = m_stage->GetPseudoRoot();
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
    // test
    // auto _hgi = pxr::Hgi::CreatePlatformDefaultHgi();
    // pxr::HdDriver hdDriver;
    // hdDriver.name = pxr::HgiTokens->renderDriver;
    // hdDriver.driver = pxr::VtValue(_hgi.get());

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

    GfCamera cam = UsdGeomCamera(*cameraPrim).GetCamera(1);
    const GfFrustum frustum = cam.GetFrustum();
    const GfVec3d cameraPos = frustum.GetPosition();

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

    glEnable(GL_DEPTH_TEST);
    //glViewport(0, 0, 800, 800);
    glClearColor(1, 0, 0, 1);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    const GLfloat CLEAR_DEPTH[1] = { 1.0f };
    // m_rootPrim = m_stage->GetPseudoRoot();

    m_engine->SetEnablePresentation(true);

    return m_engine->IsHydraEnabled();
}