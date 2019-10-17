#ifndef GUI_MAINDISPLAYWIDGET_H
#define GUI_MAINDISPLAYWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFramebufferObject>
#include <QScopedPointer>
#include <QTabBar>
#include <QReadWriteLock>
#include <QtMath>
#include "util/LinkPointer.h"
#include "gl/Global.h"
#include "gl/Root.h"
#include "gl/VertexArrayObject.h"
#include "gl/EasyTextureDrawer.h"
#include "core/Project.h"
#include "core/AbstractCursor.h"
#include "core/TimeInfo.h"
#include "core/ClippingFrame.h"
#include "core/DestinationTexturizer.h"
#include "ctrl/Driver.h"
#include "ctrl/Painter.h"
#include "gui/MainViewSetting.h"
#include "gui/CanvasMover.h"
namespace gui { class ProjectTabBar; }
namespace gui { class ViaPoint; }

namespace gui
{

class MainDisplayWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    MainDisplayWidget(ViaPoint& aViaPoint, QWidget* aParent);
    ~MainDisplayWidget();

    void setProject(core::Project* aProject);
    void setDriver(ctrl::Driver* aDriver);
    void setProjectTabBar(ProjectTabBar* aTabBar);
    void updateRender();
    void resetCamera();

    QReadWriteLock& renderingLock() { return mRenderingLock; }
    const QReadWriteLock& renderingLock() const { return mRenderingLock; }


    // boostlike signals
public:
    void onVisualUpdated();
    void onToolChanged(ctrl::ToolType);
    void onFinalizeTool(ctrl::ToolType);
    void onViewSettingChanged(const MainViewSetting&);
    void onProjectAttributeUpdated();

private:
    class GLContextAccessor : public gl::ContextAccessor
    {
        MainDisplayWidget* mOwner;
    public:
        GLContextAccessor(MainDisplayWidget* aOwner) : mOwner(aOwner) {}
        virtual void makeCurrent() { mOwner->makeCurrent(); }
        virtual void doneCurrent() { mOwner->doneCurrent(); }
    };

    // from QOpenGLWidget
    virtual void initializeGL();
    virtual void paintGL();
    virtual void paintEvent(QPaintEvent* event);
    virtual void resizeGL(int w, int h);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void tabletEvent(QTabletEvent* event);

    void updateCursor();
    QSize deviceSize() const { return this->size() * mDevicePixelRatio; }

    ViaPoint& mViaPoint;
    gl::DeviceInfo mGLDeviceInfo;
    util::LinkPointer<core::Project> mProject;
    gl::Root mGLRoot;
    GLContextAccessor mGLContextAccessor;
    QScopedPointer<gl::VertexArrayObject> mDefaultVAO;
    QScopedPointer<QOpenGLFramebufferObject> mFramebuffer;
    QScopedPointer<core::ClippingFrame> mClippingFrame;
    QScopedPointer<core::DestinationTexturizer> mDestinationTexturizer;
    QScopedPointer<gl::EasyTextureDrawer> mTextureDrawer;
    QScopedPointer<ctrl::PainterHandle> mPainterHandle;
    QReadWriteLock mRenderingLock;
    core::RenderInfo* mRenderInfo;
    core::AbstractCursor mAbstractCursor;
    ctrl::Driver* mDriver;
    ProjectTabBar* mProjectTabBar;
    bool mUsingTablet;
    MainViewSetting mViewSetting;
    CanvasMover mCanvasMover;
    bool mMovingCanvasByTool;
    bool mMovingCanvasByKey;
    double mDevicePixelRatio;
};

} // namespace gui

#endif // GUI_MAINDISPLAYWIDGET_H
