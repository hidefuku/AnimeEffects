#include <functional>
#include <QMouseEvent>
#include "XC.h"
#include "util/Finally.h"
#include "gl/Util.h"
#include "gui/MainDisplayWidget.h"
#include "gui/ProjectHook.h"
#include "gui/ViaPoint.h"
#include "gui/ProjectTabBar.h"

#include "gl/Framebuffer.h"
#include "gl/Texture.h"
#include <QOpenGLFunctions>
#include "core/ClippingFrame.h"

namespace gui
{

//-------------------------------------------------------------------------------------------------
MainDisplayWidget::MainDisplayWidget(ViaPoint& aViaPoint, QWidget* aParent)
    : QOpenGLWidget(aParent)
    , mViaPoint(aViaPoint)
    , mFramebuffer()
    , mClippingFrame()
    , mProject()
    , mTextureDrawer()
    , mRenderingLock()
    , mRenderInfo()
    , mAbstractCursor()
    , mPenInfo()
    , mDriver()
    , mProjectTabBar()
    , mUsingTablet(false)
    , mHandMove(false)
    , mViewSetting()
{
    this->setObjectName(QStringLiteral("MainDisplayWidget"));
    this->setMouseTracking(true);
    this->setAutoFillBackground(false); // avoid auto fill on QPainter::begin()
}

MainDisplayWidget::~MainDisplayWidget()
{
    mTextureDrawer.reset();
    mClippingFrame.reset();
    mFramebuffer.reset();

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

    // initialize opengl device info
    gl::DeviceInfo::createInstance();
    mViaPoint.setGLDeviceInfo(gl::DeviceInfo::instance());

    // create framebuffer for display
    mFramebuffer.reset(new QOpenGLFramebufferObject(this->size()));

    // create clipping buffer
    mClippingFrame.reset(new core::ClippingFrame());
    mClippingFrame->resize(this->size());

    // create texture drawer for copying framebuffer to display
    mTextureDrawer.reset(new gl::EasyTextureDrawer());
    if (!mTextureDrawer->init())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to initialize texture drawer.", "");
    }
}

void MainDisplayWidget::paintGL()
{
    gl::Global::Functions& ggl = gl::Global::functions();

    // clear clipping
    mClippingFrame->clearTexture();
    mClippingFrame->resetClippingId();

    if (!mFramebuffer->bind())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to bind framebuffer.", "");
    }

    // setup
    gl::Util::setViewportAsActualPixels(this->size());
    gl::Util::clearColorBuffer(0.25f, 0.25f, 0.25f, 1.0f);
    gl::Util::resetRenderState();

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
        XC_ASSERT(mRenderInfo->framebuffer != 0);
        XC_ASSERT(mRenderInfo->dest != 0);
    }

    if (mDriver)
    {
        XC_PTR_ASSERT(mRenderInfo);
        auto gridTarget = mViewSetting.showLayerMesh ?
                              mDriver->currentTarget() : nullptr;
        mDriver->renderGL(*mRenderInfo, gridTarget);
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
}

void MainDisplayWidget::paintEvent(QPaintEvent* aEvent)
{
    // try lock rendering!!
    if (!mRenderingLock.tryLockForRead()) return;
    util::Finally unlocker([=](){ this->mRenderingLock.unlock(); });

    QOpenGLWidget::paintEvent(aEvent);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (mDriver)
    {
        XC_PTR_ASSERT(mRenderInfo);
        mDriver->renderQt(*mRenderInfo, painter);
    }

    // we must call end function
    painter.end();
}

void MainDisplayWidget::resizeGL(int w, int h)
{
    if (mRenderInfo)
    {
        mRenderInfo->camera.setScreenSize(QSize(w, h));
    }
    mFramebuffer.reset();
    mFramebuffer.reset(new QOpenGLFramebufferObject(w, h));

    mClippingFrame.reset();
    mClippingFrame.reset(new core::ClippingFrame());
    mClippingFrame->resize(QSize(w, h));

    if (mProjectTabBar)
    {
        mProjectTabBar->updateTabPosition(QSize(w, h));
    }
}

void MainDisplayWidget::mouseMoveEvent(QMouseEvent* aEvent)
{
    if (mRenderInfo)
    {
        mAbstractCursor.setMouseMove(aEvent, mRenderInfo->camera);
        updateCursor(mAbstractCursor);
        //if (!mUsingTablet) qDebug() << "move" << aEvent->pos();
    }
}

void MainDisplayWidget::mousePressEvent(QMouseEvent* aEvent)
{
    if (mRenderInfo)
    {
        mAbstractCursor.setMousePress(aEvent, mRenderInfo->camera);
        updateCursor(mAbstractCursor);
        //if (!mUsingTablet) qDebug() << "press";
    }
}

void MainDisplayWidget::mouseReleaseEvent(QMouseEvent* aEvent)
{
    if (mRenderInfo)
    {
        mAbstractCursor.setMouseRelease(aEvent, mRenderInfo->camera);
        updateCursor(mAbstractCursor);
        //if (!mUsingTablet) qDebug() << "release";
    }
}

void MainDisplayWidget::wheelEvent(QWheelEvent* aEvent)
{
    if (mRenderInfo)
    {
        mRenderInfo->camera.updateByWheel(aEvent->delta(), QVector2D(aEvent->pos()));
        updateRender();
    }
}

void MainDisplayWidget::keyPressEvent(QKeyEvent* aEvent)
{
    //qDebug() << "maindisplay: input key =" << aEvent->key() << "text =" << aEvent->text();
    bool use = false;

    if (!use)
    {
        aEvent->ignore();
    }
}

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
        mHandMove = true;
    }
    else
    {
        this->setCursor(Qt::ArrowCursor);
        mHandMove = false;
    }
}

void MainDisplayWidget::onViewSettingChanged(const MainViewSetting& aSetting)
{
    mViewSetting = aSetting;

    if (mRenderInfo)
    {
        mRenderInfo->camera.setRotate(
                    util::MathUtil::getRadianFromDegree(aSetting.rotateViewDeg));
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
    bool updateRendering = false;

    // move camera by the open hand cursor
    if (mHandMove && mRenderInfo && aCursor.isLeftMoveState())
    {
        auto pos = mRenderInfo->camera.pos();
        mRenderInfo->camera.setPos(pos + aCursor.screenVel());
        updateRendering = true;
    }

    QEvent::Type tabletType = QEvent::None;
    if (aCursor.isLeftPressState()) tabletType = QEvent::TabletPress;
    else if (aCursor.isLeftMoveState()) tabletType = QEvent::TabletMove;
    else if (aCursor.isLeftReleaseState()) tabletType = QEvent::TabletRelease;

    if (!mUsingTablet && tabletType != QEvent::None)
    {
        updatePenInfo(tabletType, aCursor.screenPoint(), 1.0f);
    }

    if (mDriver)
    {
        XC_PTR_ASSERT(mRenderInfo);
        if (mDriver->updateCursor(mAbstractCursor, mPenInfo, mRenderInfo->camera))
        {
            updateRendering = true;
        }
    }

    if (updateRendering)
    {
        updateRender();
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
