#include <functional>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QGuiApplication>
#include "XC.h"
#include "util/Finally.h"
#include "gl/Util.h"
#include "gl/Framebuffer.h"
#include "gl/Texture.h"
#include "core/ClippingFrame.h"
#include "gui/MainDisplayWidget.h"
#include "gui/ProjectHook.h"
#include "gui/ViaPoint.h"
#include "gui/ProjectTabBar.h"
#include "gui/KeyCommandMap.h"
#include "gui/MouseSetting.h"

namespace gui
{

//-------------------------------------------------------------------------------------------------
MainDisplayWidget::MainDisplayWidget(ViaPoint& aViaPoint, QWidget* aParent)
    : QOpenGLWidget(aParent)
    , mViaPoint(aViaPoint)
    , mGLDeviceInfo()
    , mProject()
    , mGLRoot()
    , mGLContextAccessor(this)
    , mDefaultVAO()
    , mFramebuffer()
    , mClippingFrame()
    , mDestinationTexturizer()
    , mTextureDrawer()
    , mPainterHandle()
    , mRenderingLock()
    , mRenderInfo()
    , mAbstractCursor()
    , mDriver()
    , mProjectTabBar()
    , mUsingTablet(false)
    , mViewSetting()
    , mCanvasMover()
    , mMovingCanvasByTool(false)
    , mMovingCanvasByKey(false)
    , mDevicePixelRatio(1.0)
{
#ifdef USE_GL_CORE_PROFILE
    // setup opengl format
    QSurfaceFormat format = this->format();
    format.setVersion(gl::Global::kMajorVersion, gl::Global::kMinorVersion);
    format.setProfile(QSurfaceFormat::CoreProfile);
    this->setFormat(format);
#endif

    this->setObjectName(QStringLiteral("MainDisplayWidget"));
    this->setMouseTracking(true);
    this->setAutoFillBackground(false); // avoid auto fill on QPainter::begin()

    // key binding
    if (mViaPoint.keyCommandMap())
    {
        // move canvas
        {
            auto key = mViaPoint.keyCommandMap()->get("MoveCanvas");
            if (key)
            {
                key->invoker = [=]()
                {
                    mAbstractCursor.suspendEvent([=]() { updateCursor(); });
                    mMovingCanvasByKey = true;
                    mCanvasMover.setDragAndMove(mMovingCanvasByKey || mMovingCanvasByTool);
                };
                key->releaser = [=]()
                {
                    mMovingCanvasByKey = false;
                    mCanvasMover.setDragAndMove(mMovingCanvasByKey || mMovingCanvasByTool);
                    mAbstractCursor.resumeEvent();
                };
            }
        }

        // rotate canvas
        {
            auto key = mViaPoint.keyCommandMap()->get("RotateCanvas");
            if (key)
            {
                key->invoker = [=]()
                {
                    mAbstractCursor.suspendEvent([=]() { updateCursor(); });
                    mCanvasMover.setDragAndRotate(true);
                };
                key->releaser = [=]()
                {
                    mCanvasMover.setDragAndRotate(false);
                    mAbstractCursor.resumeEvent();
                };
            }
        }

        // reset canvas angle
        {
            auto key = mViaPoint.keyCommandMap()->get("ResetCanvasAngle");
            if (key)
            {
                key->invoker = [=]()
                {
                    mViaPoint.mainViewSetting().resetRotateView = true;
                    onViewSettingChanged(mViaPoint.mainViewSetting());
                    mViaPoint.mainViewSetting().resetRotateView = false;
                };
            }
        }
    }
}

MainDisplayWidget::~MainDisplayWidget()
{
    mPainterHandle.reset();
    mTextureDrawer.reset();
    mDestinationTexturizer.reset();
    mClippingFrame.reset();
    mFramebuffer.reset();
    mDefaultVAO.reset();

    gl::DeviceInfo::setInstance(nullptr);
    gl::Global::clearFunctions();
}

void MainDisplayWidget::setProject(core::Project* aProject)
{
    mProject.reset();

    mRenderInfo = nullptr;
    mAbstractCursor = core::AbstractCursor();

    if (aProject)
    {
        mProject = aProject->pointee();
        mRenderInfo = &(static_cast<ProjectHook*>(mProject->hook())->renderInfo());
        mRenderInfo->camera.setDevicePixelRatio(this->devicePixelRatioF());
        mRenderInfo->camera.setScreenSize(this->size());
        mRenderInfo->camera.setImageSize(mProject->attribute().imageSize());
        mCanvasMover.setCamera(&(mRenderInfo->camera));
    }

    updateRender();
}

void MainDisplayWidget::setDriver(ctrl::Driver* aDriver)
{
    mDriver = aDriver;
}

void MainDisplayWidget::resetCamera()
{
    if (mRenderInfo)
    {
        auto& camera = mRenderInfo->camera;
        auto scrSize = this->size();
        auto imgSize = camera.imageSize();
        camera.setCenter(QVector2D(scrSize.width() * 0.5f, scrSize.height() * 0.5f));
        if (scrSize.width() > 0 && scrSize.height() > 0 && imgSize.width() > 0 && imgSize.height() > 0)
        {
            auto scaleX = (float)scrSize.width() / imgSize.width();
            auto scaleY = (float)scrSize.height() / imgSize.height();
            auto minScale = scaleX < scaleY ? scaleX : scaleY;
            camera.setScale(minScale);
        }

        mCanvasMover.setCamera(&camera);
    }
}

void MainDisplayWidget::setProjectTabBar(ProjectTabBar* aTabBar)
{
    mProjectTabBar = aTabBar;
}

void MainDisplayWidget::updateRender()
{
    this->update();
}

void MainDisplayWidget::initializeGL()
{
    if (!this->context()->isValid())
    {
        XC_FATAL_ERROR("OpenGL Error", "Invalid opengl context.", "");
    }
    // check version
    {
        auto flags = QGLFormat::openGLVersionFlags();
        if (!flags.testFlag(gl::Global::kVersionFlag))
        {
            auto version = QString::number(gl::Global::kMajorVersion) + "." + QString::number(gl::Global::kMinorVersion);
            XC_FATAL_ERROR("OpenGL Error", QString("The OpenGL version lower than ") + version + ".", "");
        }
    }
    // initialize opengl functions
    auto functions = this->context()->versionFunctions<gl::Global::Functions>();
    if (!functions)
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to get opengl functions.", "");
    }
    if (!functions->initializeOpenGLFunctions())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to initialize opengl functions.", "");
    }

    // setup global info
    gl::Global::setContext(*this);
    gl::Global::setFunctions(*functions);
    // setup gl root
    mGLRoot.setContextAccessor(mGLContextAccessor);
    mGLRoot.setFunctions(*functions);

    // initialize opengl device info
    mGLDeviceInfo.load();
    gl::DeviceInfo::setInstance(&mGLDeviceInfo);
    mViaPoint.setGLDeviceInfo(mGLDeviceInfo);

#ifdef USE_GL_CORE_PROFILE
    // initialize default vao
    mDefaultVAO.reset(new gl::VertexArrayObject());
    mDefaultVAO->bind(); // keep binding
#endif

    mDevicePixelRatio = this->devicePixelRatioF();

    // create framebuffer for display
    mFramebuffer.reset(new QOpenGLFramebufferObject(deviceSize()));

    // create clipping buffer
    mClippingFrame.reset(new core::ClippingFrame());
    mClippingFrame->resize(deviceSize());

    // create texturizer for destination colors of the framebuffer
    mDestinationTexturizer.reset(new core::DestinationTexturizer());
    mDestinationTexturizer->resize(deviceSize());

    // create texture drawer for copying framebuffer to display
    mTextureDrawer.reset(new gl::EasyTextureDrawer());
    if (!mTextureDrawer->init())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to initialize texture drawer.", "");
    }

    mPainterHandle.reset(new ctrl::PainterHandle());

    GL_CHECK_ERROR();
}

void MainDisplayWidget::paintGL()
{
    gl::Global::Functions& ggl = gl::Global::functions();

    // clear clipping
    mClippingFrame->clearTexture();
    mClippingFrame->resetClippingId();
    GL_CHECK_ERROR();

    mDestinationTexturizer->clearTexture();
    GL_CHECK_ERROR();

    if (!mFramebuffer->bind())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to bind framebuffer.", "");
    }

    // setup
    gl::Util::setViewportAsActualPixels(deviceSize());
    gl::Util::clearColorBuffer(0.25f, 0.25f, 0.25f, 1.0f);
    gl::Util::resetRenderState();
    GL_CHECK_ERROR();

    // setup renderinfo
    if (mProject)
    {
        XC_PTR_ASSERT(mRenderInfo);
        mRenderInfo->time = mProject->currentTimeInfo();
        mRenderInfo->framebuffer = mFramebuffer->handle();
        mRenderInfo->dest = mFramebuffer->texture();
        mRenderInfo->isGrid = false;
        mRenderInfo->nonPosed = false;
        mRenderInfo->clippingId = 0;
        mRenderInfo->clippingFrame = mClippingFrame.data();
        mRenderInfo->destTexturizer = mDestinationTexturizer.data();
        XC_ASSERT(mRenderInfo->framebuffer != 0);
        XC_ASSERT(mRenderInfo->dest != 0);
    }

    if (mDriver)
    {
        XC_PTR_ASSERT(mRenderInfo);
        auto gridTarget = mViewSetting.showLayerMesh ?
                              mDriver->currentTarget() : nullptr;
        mDriver->renderGL(*mRenderInfo, gridTarget);
        GL_CHECK_ERROR();
    }

    if (!mFramebuffer->release())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to unbind framebuffer.", "");
    }

    ggl.glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());

    if (mViewSetting.cutImagesByTheFrame && mProject)
    {
        XC_PTR_ASSERT(mRenderInfo);
        gl::Util::clearColorBuffer(0.25f, 0.25f, 0.25f, 1.0f);

        auto imageQuad = mRenderInfo->camera.screenImageQuadangle();
        const QSize screenSize = mRenderInfo->camera.screenSize();

        mTextureDrawer->draw(mFramebuffer->texture(),
                             imageQuad, screenSize, imageQuad, screenSize);
    }
    else
    {
        mTextureDrawer->draw(mFramebuffer->texture());
    }

    ggl.glFlush();
    GL_CHECK_ERROR();
}

void MainDisplayWidget::paintEvent(QPaintEvent* aEvent)
{
    // try lock rendering!!
    if (!mRenderingLock.tryLockForRead()) return;
    util::Finally unlocker([=](){ this->mRenderingLock.unlock(); });

    QOpenGLWidget::paintEvent(aEvent);

    QPainter* painter = mPainterHandle->begin(*this);
    painter->setRenderHint(QPainter::Antialiasing);

    if (mDriver)
    {
        XC_PTR_ASSERT(mRenderInfo);
        mDriver->renderQt(*mRenderInfo, *painter);
        GL_CHECK_ERROR();
    }
    // we must call end function
    mPainterHandle->end();
}

void MainDisplayWidget::resizeGL(int w, int h)
{
    // currrent device pixel ratio (Attention that it is variable.)
    mDevicePixelRatio = this->devicePixelRatioF();
    const QSize devSize(mDevicePixelRatio * w, mDevicePixelRatio * h); // device pixel size
    const QSize absSize(w, h); // abstract pixel size

    if (mRenderInfo)
    {
        mRenderInfo->camera.setDevicePixelRatio(mDevicePixelRatio);
        mRenderInfo->camera.setScreenSize(absSize);
        mCanvasMover.onScreenResized();
    }
    mFramebuffer.reset();
    mFramebuffer.reset(new QOpenGLFramebufferObject(devSize.width(), devSize.height()));

    mClippingFrame.reset();
    mClippingFrame.reset(new core::ClippingFrame());
    mClippingFrame->resize(devSize);

    mDestinationTexturizer->resize(devSize);

    if (mProjectTabBar)
    {
        mProjectTabBar->updateTabPosition(absSize);
    }
    GL_CHECK_ERROR();
}

void MainDisplayWidget::mouseMoveEvent(QMouseEvent* aEvent)
{
    if (mRenderInfo)
    {
        if (mAbstractCursor.setMouseMove(aEvent, mRenderInfo->camera))
        {
            updateCursor();
            //if (!mUsingTablet) qDebug() << "move" << aEvent->pos();
        }

        if (mCanvasMover.updateByMove(mAbstractCursor.screenPos(),
                                      mAbstractCursor.screenVel(),
                                      mAbstractCursor.isPressedLeft(),
                                      mAbstractCursor.isPressedRight()))
        {
            updateRender();
        }
    }
}

void MainDisplayWidget::mousePressEvent(QMouseEvent* aEvent)
{
    if (mRenderInfo)
    {
        if (mAbstractCursor.setMousePress(aEvent, mRenderInfo->camera))
        {
            updateCursor();
            //if (!mUsingTablet) qDebug() << "press";
        }
    }
}

void MainDisplayWidget::mouseReleaseEvent(QMouseEvent* aEvent)
{
    if (mRenderInfo)
    {
        if (mAbstractCursor.setMouseRelease(aEvent, mRenderInfo->camera))
        {
            updateCursor();
            //if (!mUsingTablet) qDebug() << "release";
        }
    }
}

void MainDisplayWidget::wheelEvent(QWheelEvent* aEvent)
{
    if (mCanvasMover.updateByWheel(QVector2D(aEvent->position()), aEvent->angleDelta().y(),
                                   mViaPoint.mouseSetting().invertMainViewScaling))
    {
        updateRender();
    }
}

void MainDisplayWidget::tabletEvent(QTabletEvent* aEvent)
{
    /// Tablet behavior is difference between mac and windows. It's Qt's bug?
    /// In windows, Duplicate mouse events occur while operating a tablet.
#ifdef Q_OS_MAC
    if (mRenderInfo)
    {
        if (mAbstractCursor.setTabledEvent(aEvent, mRenderInfo->camera))
        {
            updateCursor();
        }
    }
    if (mCanvasMover.updateByMove(mAbstractCursor.screenPos(),
                                  mAbstractCursor.screenVel(),
                                  mAbstractCursor.isPressedLeft(),
                                  mAbstractCursor.isPressedRight()))
    {
        updateRender();
    }
#else
    mAbstractCursor.setTabletPressure(aEvent);
#endif
    aEvent->accept();
}

void MainDisplayWidget::onVisualUpdated()
{
    updateRender();
}

void MainDisplayWidget::onToolChanged(ctrl::ToolType aType)
{
    if (aType == ctrl::ToolType_Cursor)
    {
        this->setCursor(Qt::OpenHandCursor);
        mMovingCanvasByTool = true;
        mCanvasMover.setDragAndMove(mMovingCanvasByKey || mMovingCanvasByTool);
    }
    else
    {
        this->setCursor(Qt::ArrowCursor);
        mMovingCanvasByTool = false;
        mCanvasMover.setDragAndMove(mMovingCanvasByKey || mMovingCanvasByTool);
    }
}

void MainDisplayWidget::onFinalizeTool(ctrl::ToolType)
{
    mAbstractCursor.suspendEvent([=]() { updateCursor(); });
    mAbstractCursor.resumeEvent();
}

void MainDisplayWidget::onViewSettingChanged(const MainViewSetting& aSetting)
{
    mViewSetting = aSetting;

    if (mViewSetting.resetRotateView)
    {
        mCanvasMover.resetRotation();
    }
    else if (mViewSetting.rotateViewACW)
    {
        mCanvasMover.rotate((float)(-M_PI / 18.0));
    }
    else if (mViewSetting.rotateViewCW)
    {
        mCanvasMover.rotate((float)(M_PI / 18.0));
    }

    updateRender();
}

void MainDisplayWidget::onProjectAttributeUpdated()
{
    if (mProject && mRenderInfo)
    {
        const QSize newSize = mProject->attribute().imageSize();
        if (newSize != mRenderInfo->camera.imageSize())
        {
            mRenderInfo->camera.setImageSize(newSize);
        }
    }
}

void MainDisplayWidget::updateCursor()
{
    if (mDriver)
    {
        XC_PTR_ASSERT(mRenderInfo);
        if (mDriver->updateCursor(mAbstractCursor, mRenderInfo->camera))
        {
            updateRender();
        }
    }
}

} // namespace gui
