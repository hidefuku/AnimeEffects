#include <functional>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include "XC.h"
#include "util/Finally.h"
#include "gl/Util.h"
#include "gui/MainDisplayWidget.h"
#include "gui/ProjectHook.h"
#include "gui/ViaPoint.h"
#include "gui/ProjectTabBar.h"
#include "gui/KeyCommandMap.h"
#include "gl/Framebuffer.h"
#include "gl/Texture.h"
#include "core/ClippingFrame.h"

namespace gui
{

//-------------------------------------------------------------------------------------------------
MainDisplayWidget::MainDisplayWidget(ViaPoint& aViaPoint, QWidget* aParent)
    : QOpenGLWidget(aParent)
    , mViaPoint(aViaPoint)
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
    , mPenInfo()
    , mDriver()
    , mProjectTabBar()
    , mUsingTablet(false)
    , mViewSetting()
    , mCanvasMover()
    , mMovingCanvasByTool(false)
    , mMovingCanvasByKey(false)
{
#ifdef USE_GL_CORE_PROFILE
    // setup opengl format (for gl removed)
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
                    mAbstractCursor.suspendEvent([=]() { updateCursor(mAbstractCursor); });
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
                    mAbstractCursor.suspendEvent([=]() { updateCursor(mAbstractCursor); });
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

    gl::Global::clearFunctions();
}

void MainDisplayWidget::setProject(core::Project* aProject)
{
    mProject.reset();

    mRenderInfo = nullptr;
    mAbstractCursor = core::AbstractCursor();
    mPenInfo = core::PenInfo();

    if (aProject)
    {
        mProject = aProject->pointee();
        mRenderInfo = &(static_cast<ProjectHook*>(mProject->hook())->renderInfo());
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
    gl::DeviceInfo::createInstance();
    mViaPoint.setGLDeviceInfo(gl::DeviceInfo::instance());

#ifdef USE_GL_CORE_PROFILE
    // initialize default vao
    mDefaultVAO.reset(new gl::VertexArrayObject());
    mDefaultVAO->bind(); // keep binding
#endif

    // create framebuffer for display
    mFramebuffer.reset(new QOpenGLFramebufferObject(this->size()));

    // create clipping buffer
    mClippingFrame.reset(new core::ClippingFrame());
    mClippingFrame->resize(this->size());

    // create texturizer for destination colors of the framebuffer
    mDestinationTexturizer.reset(new core::DestinationTexturizer());
    mDestinationTexturizer->resize(this->size());

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

    if (!mFramebuffer->bind())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to bind framebuffer.", "");
    }

    // setup
    gl::Util::setViewportAsActualPixels(this->size());
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
    const QSize newSize(w, h);

    if (mRenderInfo)
    {
        mRenderInfo->camera.setScreenSize(newSize);
        mCanvasMover.onScreenResized();
    }
    mFramebuffer.reset();
    mFramebuffer.reset(new QOpenGLFramebufferObject(w, h));

    mClippingFrame.reset();
    mClippingFrame.reset(new core::ClippingFrame());
    mClippingFrame->resize(newSize);

    mDestinationTexturizer->resize(newSize);

    if (mProjectTabBar)
    {
        mProjectTabBar->updateTabPosition(newSize);
    }
    GL_CHECK_ERROR();
}

void MainDisplayWidget::mouseMoveEvent(QMouseEvent* aEvent)
{
    if (mRenderInfo)
    {
        if (mAbstractCursor.setMouseMove(aEvent, mRenderInfo->camera))
        {
            updateCursor(mAbstractCursor);
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
            updateCursor(mAbstractCursor);
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
            updateCursor(mAbstractCursor);
            //if (!mUsingTablet) qDebug() << "release";
        }
    }
}

void MainDisplayWidget::wheelEvent(QWheelEvent* aEvent)
{
    if (mCanvasMover.updateByWheel(QVector2D(aEvent->pos()), aEvent->delta()))
    {
        updateRender();
    }
}

#if 0
void MainDisplayWidget::keyPressEvent(QKeyEvent* aEvent)
{
    //qDebug() << "maindisplay: input key =" << aEvent->key() << "text =" << aEvent->text();
    bool use = false;

    if (!use)
    {
        aEvent->ignore();
    }
}
#endif

void MainDisplayWidget::tabletEvent(QTabletEvent* aEvent)
{
    updatePenInfo(aEvent->type(), aEvent->pos(), aEvent->pressure());
    aEvent->accept();
    updateRender();

#if 0
    if (aEvent->type() == QEvent::TabletPress)
    {
        qDebug() << "tablet press" << aEvent->pressure();
    }
    else if (aEvent->type() == QEvent::TabletRelease)
    {
        qDebug() << "tablet release" << aEvent->pressure();
    }
    else
    {
        qDebug() << "tablet move" << aEvent->pressure() << aEvent->pos();
    }
#endif
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

void MainDisplayWidget::updateCursor(const core::AbstractCursor& aCursor)
{
    QEvent::Type tabletType = QEvent::None;
    if (aCursor.emitsLeftPressedEvent()) tabletType = QEvent::TabletPress;
    else if (aCursor.emitsLeftDraggedEvent()) tabletType = QEvent::TabletMove;
    else if (aCursor.emitsLeftReleasedEvent()) tabletType = QEvent::TabletRelease;

    if (!mUsingTablet && tabletType != QEvent::None)
    {
        updatePenInfo(tabletType, aCursor.screenPoint(), 1.0f);
    }

    if (mDriver)
    {
        XC_PTR_ASSERT(mRenderInfo);
        if (mDriver->updateCursor(mAbstractCursor, mPenInfo, mRenderInfo->camera))
        {
            updateRender();
        }
    }
}

void MainDisplayWidget::updatePenInfo(QEvent::Type aType, const QPoint& aPos, float aPressure)
{
    if (!mRenderInfo) return;

    const QVector2D pos(aPos.x(), aPos.y());

    core::CameraInfo& camera = mRenderInfo->camera;
    bool isMoving = false;
    mPenInfo.screenVel = QVector2D();
    mPenInfo.screenWidth = camera.screenWidth();
    mPenInfo.screenHeight = camera.screenHeight();

    if (aType== QEvent::TabletPress)
    {
        mPenInfo.isPressing = true;
        mUsingTablet = true;
    }
    else if (aType == QEvent::TabletRelease)
    {
        mPenInfo.isPressing = false;
        mUsingTablet = false;
    }
    else if (aType == QEvent::TabletMove)
    {
        isMoving = true;
    }

    if (mPenInfo.isPressing && isMoving)
    {
        mPenInfo.screenVel = pos - mPenInfo.screenPos;
    }

    mPenInfo.screenPos = pos;
    mPenInfo.pressure = aPressure;

    mPenInfo.pos = camera.toWorldPos(mPenInfo.screenPos);
    mPenInfo.vel = camera.toWorldVector(mPenInfo.screenVel);
}

} // namespace gui
