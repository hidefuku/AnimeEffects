#include "gl/Global.h"
#include "XC.h"

namespace
{
gl::Global::Functions* gGLGlobalFunctions = nullptr;
//QOpenGLContext* gGLGlobalContext = nullptr;
//QSurface* gGlobalSurface = nullptr;
QOpenGLWidget* gGLGlobalWidget = nullptr;
}
QGLFormat::OpenGLVersionFlag gl::Global::kVersionFlag = QGLFormat::OpenGL_Version_4_0;

namespace gl
{

void Global::setFunctions(Functions& aFunctions)
{
    XC_ASSERT(!gGLGlobalFunctions);
    gGLGlobalFunctions = &aFunctions;
}

void Global::clearFunctions()
{
    gGLGlobalFunctions = nullptr;
}

Global::Functions& Global::functions()
{
    XC_PTR_ASSERT(gGLGlobalFunctions);
    return *gGLGlobalFunctions;
}

#if 0
void Global::setContext(QOpenGLContext& aContext, QSurface& aSurface)
{
    XC_ASSERT(!gGLGlobalContext);
    gGLGlobalContext = &aContext;
    gGlobalSurface = &aSurface;
}

void Global::clearContext()
{
    gGLGlobalContext = nullptr;
}

void Global::makeCurrent()
{
    XC_PTR_ASSERT(gGLGlobalContext);
    gGLGlobalContext->makeCurrent(gGlobalSurface);
}

void Global::doneCurrent()
{
    XC_PTR_ASSERT(gGLGlobalContext);
    gGLGlobalContext->doneCurrent();
}
#else
void Global::setContext(QOpenGLWidget& aWidget)
{
    XC_ASSERT(!gGLGlobalWidget);
    gGLGlobalWidget = &aWidget;
}

void Global::clearContext()
{
    gGLGlobalWidget = nullptr;
}

void Global::makeCurrent()
{
    XC_PTR_ASSERT(gGLGlobalWidget);
    gGLGlobalWidget->makeCurrent();
}

void Global::doneCurrent()
{
    XC_PTR_ASSERT(gGLGlobalWidget);
    gGLGlobalWidget->doneCurrent();
}
#endif

} // namespace gl
