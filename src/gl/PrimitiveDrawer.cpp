#include "util/MathUtil.h"
#include "gl/Global.h"
#include "gl/Util.h"
#include "gl/Triangulator.h"
#include "gl/PrimitiveDrawer.h"

namespace
{
static const int kMinVtxCountOfSlot = 32;

bool fBuildShader(gl::EasyShaderProgram& aProgram, const char* aVertex, const char* aFragment)
{
    if (!aProgram.setVertexSource(QString(aVertex)))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile vertex shader.", aProgram.log());
        return false;
    }
    if (!aProgram.setFragmentSource(QString(aFragment)))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile fragment shader.", aProgram.log());
        return false;
    }
    if (!aProgram.link())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.", aProgram.log());
        return false;
    }
    return true;
}

bool fMakeLinePolygon(const QPointF& aFrom, const QPointF& aTo, float aWidth,
                      gl::Vector2* aPositions, gl::Vector2* aStyleCoords)
{
    const QVector2D dir(aTo - aFrom);
    const float length = dir.length();
    if (length < FLT_EPSILON) return false;

    auto rside = util::MathUtil::getRotateVector90Deg(dir).normalized() * aWidth * 0.5f;
    aPositions[0].set(aFrom.x() - rside.x(), aFrom.y() - rside.y());
    aPositions[1].set(aFrom.x() + rside.x(), aFrom.y() + rside.y());
    aPositions[2].set(aTo.x() - rside.x(), aTo.y() - rside.y());
    aPositions[3].set(aTo.x() + rside.x(), aTo.y() + rside.y());

    aStyleCoords[0].set(0.0f, 0.0f);
    aStyleCoords[1].set(0.0f, 0.0f);
    aStyleCoords[2].set(length, 0.0f);
    aStyleCoords[3].set(length, 0.0f);

    return true;
}

}

namespace gl
{

//-------------------------------------------------------------------------------------------------
PrimitiveDrawer::State::State()
    : brushColor(QColor(0, 0, 0, 255).rgba())
    , penColor(QColor(0, 0, 0, 255).rgba())
    , penWidth(1.0f)
    , penStyle(PenStyle_Solid)
    , texture(0)
    , textureColor(QColor(255, 255, 255, 255).rgba())
    , hasBrush(true)
    , hasPen(true)
    , hasMSAA(false)
{
}

bool PrimitiveDrawer::State::operator==(const State& aRhs) const
{
    return
            brushColor == aRhs.brushColor &&
            penColor == aRhs.penColor &&
            penWidth == aRhs.penWidth &&
            penStyle == aRhs.penStyle &&
            texture == aRhs.texture &&
            textureColor == aRhs.textureColor &&
            hasBrush == aRhs.hasBrush &&
            hasPen == aRhs.hasPen &&
            hasMSAA == aRhs.hasMSAA;
}

bool PrimitiveDrawer::State::operator!=(const State& aRhs) const
{
    return !(*this == aRhs);
}

void PrimitiveDrawer::State::set(const Command& aCommand)
{
    switch (aCommand.type)
    {
    case Type_Brush:
        brushColor = aCommand.attr.brush.color;
        break;
    case Type_Pen:
        penColor = aCommand.attr.pen.color;
        penWidth = aCommand.attr.pen.width;
        penStyle = aCommand.attr.pen.style;
        break;
    case Type_Texture:
        texture = aCommand.attr.texture.id;
        textureColor = aCommand.attr.texture.color;
        break;
    case Type_Ability:
        hasBrush = aCommand.attr.ability.hasBrush;
        hasPen = aCommand.attr.ability.hasPen;
        hasMSAA = aCommand.attr.ability.hasMSAA;
        break;
    default:
        break;
    }
}

bool PrimitiveDrawer::State::hasDifferentValueWith(const Command& aCommand) const
{
    switch (aCommand.type)
    {
    case Type_Brush:
        return brushColor != aCommand.attr.brush.color;
    case Type_Pen:
        return penColor != aCommand.attr.pen.color ||
               penWidth != aCommand.attr.pen.width ||
               penStyle != aCommand.attr.pen.style;
    case Type_Texture:
        return texture != aCommand.attr.texture.id ||
               textureColor != aCommand.attr.texture.color;
    case Type_Ability:
        return hasBrush != aCommand.attr.ability.hasBrush ||
               hasPen != aCommand.attr.ability.hasPen ||
               hasMSAA != aCommand.attr.ability.hasMSAA;
    default:
        XC_ASSERT(0);
        return false;
    }
}

//-------------------------------------------------------------------------------------------------
bool PrimitiveDrawer::PlaneShader::init()
{
    static const char* kVertexShaderText =
            "#version 330 \n"
            "in vec2 inPosition;"
            "uniform mat4 uViewMtx;"
            "void main(void){"
            "  gl_Position = uViewMtx * vec4(inPosition, 0.0, 1.0);"
            "}";
    static const char* kFragmentShaderText =
            "#version 330 \n"
            "uniform vec4 uColor;"
            "layout(location = 0, index = 0) out vec4 oFragColor;"
            "void main(void){"
            "  oFragColor = uColor;"
            "}";

    if (!fBuildShader(program, kVertexShaderText, kFragmentShaderText))
    {
        return false;
    }

    locPosition = program.attributeLocation("inPosition");
    locViewMtx = program.uniformLocation("uViewMtx");
    locColor = program.uniformLocation("uColor");

    GL_CHECK_ERROR();
    return true;
}

//-------------------------------------------------------------------------------------------------
bool PrimitiveDrawer::StippleShader::init()
{
    static const char* kVertexShaderText =
            "#version 330 \n"
            "in vec2 inPosition;"
            "in vec2 inLength;"
            "out vec2 vLength;"
            "uniform mat4 uViewMtx;"
            "uniform vec2 uScreenSize;"
            "void main(void){"
            "  gl_Position = uViewMtx * vec4(inPosition, 0.0, 1.0);"
            "  vLength = (uViewMtx * vec4(inLength, 0.0, 1.0)).xy * uScreenSize;"
            "}";
    static const char* kFragmentShaderText =
            "#version 330 \n"
            "in vec2 vLength;"
            "uniform vec4 uColor;"
            "uniform vec2 uWave;"
            "layout(location = 0, index = 0) out vec4 oFragColor;"
            "void main(void){"
            "  float f = cos(uWave.x * vLength.x * 3.1415926);"
            "  oFragColor = vec4(uColor.rgb, uColor.a * smoothstep(0.0, 1.0, f + uWave.y));"
            "}";

    if (!fBuildShader(program, kVertexShaderText, kFragmentShaderText))
    {
        return false;
    }

    locPosition = program.attributeLocation("inPosition");
    locLength = program.attributeLocation("inLength");
    locViewMtx = program.uniformLocation("uViewMtx");
    locScreenSize = program.uniformLocation("uScreenSize");
    locColor = program.uniformLocation("uColor");
    locWave = program.uniformLocation("uWave");

    GL_CHECK_ERROR();
    return true;
}

//-------------------------------------------------------------------------------------------------
bool PrimitiveDrawer::TextureShader::init()
{
    static const char* kVertexShaderText =
            "#version 330 \n"
            "in vec2 inPosition;"
            "in vec2 inTexCoord;"
            "out vec2 vTexCoord;"
            "uniform mat4 uViewMtx;"
            "void main(void){"
            "  gl_Position = uViewMtx * vec4(inPosition, 0.0, 1.0);"
            "  vTexCoord = inTexCoord;"
            "}";
    static const char* kFragmentShaderText =
            "#version 330 \n"
            "in vec2 vTexCoord;"
            "uniform vec4 uColor;"
            "uniform sampler2D uTexture;"
            "layout(location = 0, index = 0) out vec4 oFragColor;"
            "void main(void){"
            "  oFragColor = uColor * texture(uTexture, vTexCoord);"
            "}";

    if (!fBuildShader(program, kVertexShaderText, kFragmentShaderText))
    {
        return false;
    }

    locPosition = program.attributeLocation("inPosition");
    locTexCoord = program.attributeLocation("inTexCoord");
    locViewMtx = program.uniformLocation("uViewMtx");
    locColor = program.uniformLocation("uColor");
    locTexture = program.uniformLocation("uTexture");

    GL_CHECK_ERROR();
    return true;
}

//-------------------------------------------------------------------------------------------------
PrimitiveDrawer::PrimitiveDrawer(int aVtxCountOfSlot, int aSlotCount)
    : mPlaneShader()
    , mStippleShader()
    , mTextureShader()
    , mCurrentShader(ShaderType_TERM)
    , mVtxCountOfSlot(std::max(aVtxCountOfSlot, kMinVtxCountOfSlot))
    , mSlotCount(std::max(aSlotCount, 1))
    , mPosSlotIds()
    , mSubSlotIds()
    , mCurrentSlotIndex(0)
    , mCurrentSlotSize(0)
    , mViewMtx()
    , mScreenSize()
    , mPixelScale(1.0f)
    , mScheduledCommands()
    , mInDrawing(false)
    , mAppliedState()
    , mScheduledState()
    , mPosBuffer()
    , mSubBuffer()
{
    // create shader
    mPlaneShader.init();
    mStippleShader.init();
    mTextureShader.init();

    // create buffer
    {
        Global::Functions& ggl = Global::functions();

        mPosSlotIds.resize(mSlotCount);
        mSubSlotIds.resize(mSlotCount);
        ggl.glGenBuffers(mSlotCount, mPosSlotIds.data());
        ggl.glGenBuffers(mSlotCount, mSubSlotIds.data());

        auto bufferSize = (GLsizeiptr)(sizeof(gl::Vector2) * mVtxCountOfSlot);
        for (int i = 0; i < mSlotCount; ++i)
        {
            ggl.glBindBuffer(GL_ARRAY_BUFFER, mPosSlotIds[i]);
            ggl.glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);
            ggl.glBindBuffer(GL_ARRAY_BUFFER, mSubSlotIds[i]);
            ggl.glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);
        }
        ggl.glBindBuffer(GL_ARRAY_BUFFER, 0);
        GL_CHECK_ERROR();
    }
}

PrimitiveDrawer::~PrimitiveDrawer()
{
    // delete buffer
    Global::functions().glDeleteBuffers(mSlotCount, mPosSlotIds.data());
    Global::functions().glDeleteBuffers(mSlotCount, mSubSlotIds.data());
    GL_CHECK_ERROR();
}

void PrimitiveDrawer::begin()
{
    XC_ASSERT(!mInDrawing);
    mInDrawing = true;

    Util::resetRenderState();

    Global::Functions& ggl = Global::functions();
    ggl.glEnable(GL_BLEND);
    ggl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //ggl.glEnable(GL_CULL_FACE);
    //ggl.glCullFace(GL_BACK);
    Util::setAbility(GL_MULTISAMPLE, mAppliedState.hasMSAA);
}

void PrimitiveDrawer::end()
{
    XC_ASSERT(mInDrawing);
    flushCommands();
    mInDrawing = false;
}

void PrimitiveDrawer::setViewMatrix(const QMatrix4x4& aViewMtx)
{
    mViewMtx = aViewMtx;

    Global::Functions& ggl = Global::functions();
    {
        GLint viewport[4] = {};
        ggl.glGetIntegerv(GL_VIEWPORT, viewport);
        mScreenSize = QSize(viewport[2], viewport[3]);
    }
    // mPixelScale doesn't consider scaling which be applied different values at x and y axis.
    auto scrNorm = mViewMtx * QVector3D(1.0f, 0.0f, 0.0f) - mViewMtx * QVector3D(0.0f, 0.0f, 0.0f);
    auto scrRatio = QVector2D(0.5f * mScreenSize.width() * scrNorm.x(),
                              0.5f * mScreenSize.height() * scrNorm.y()).length();
    mPixelScale = (scrRatio > FLT_EPSILON) ? 1.0f / scrRatio : 1.0f;
}

void PrimitiveDrawer::setBrush(const QColor& aColor)
{
    Command command = { Type_Brush };
    command.attr.brush.color = aColor.rgba();
    pushStateCommand(command);
}

void PrimitiveDrawer::setPen(const QColor& aColor, float aWidth, PenStyle aPenStyle)
{
    Command command = { Type_Pen };
    command.attr.pen.color = aColor.rgba();
    command.attr.pen.width = aWidth;
    command.attr.pen.style = aPenStyle;
    pushStateCommand(command);
}

void PrimitiveDrawer::setBrushEnable(bool aIsEnable)
{
    if (mScheduledState.hasBrush == aIsEnable) return;
    Command command = { Type_Ability };
    command.attr.ability.hasBrush = aIsEnable;
    command.attr.ability.hasPen = mScheduledState.hasPen;
    command.attr.ability.hasMSAA = mScheduledState.hasMSAA;
    pushStateCommand(command);
}

void PrimitiveDrawer::setPenEnable(bool aIsEnable)
{
    if (mScheduledState.hasPen == aIsEnable) return;
    Command command = { Type_Ability };
    command.attr.ability.hasBrush = mScheduledState.hasBrush;
    command.attr.ability.hasPen = aIsEnable;
    command.attr.ability.hasMSAA = mScheduledState.hasMSAA;
    pushStateCommand(command);
}

void PrimitiveDrawer::setAntiAliasing(bool aIsEnable)
{
    if (mScheduledState.hasMSAA == aIsEnable) return;
    Command command = { Type_Ability };
    command.attr.ability.hasBrush = mScheduledState.hasBrush;
    command.attr.ability.hasPen = mScheduledState.hasPen;
    command.attr.ability.hasMSAA = aIsEnable;
    pushStateCommand(command);
}

void PrimitiveDrawer::drawOutline(const std::function<QPointF(int)>& aGetPos, int aPosCount, bool aForce)
{
    if (!aForce && !mScheduledState.hasPen) return;

    const int maxVtxCount = 4 * (mVtxCountOfSlot / 4);
    const float width = mScheduledState.penWidth * mPixelScale;

    Command command = { Type_Draw };
    command.attr.draw.prim = GL_TRIANGLE_STRIP;
    command.attr.draw.usePen = true;

    mPosBuffer.resize(mVtxCountOfSlot);
    mSubBuffer.resize(mVtxCountOfSlot);
    gl::Vector2* posPtr = mPosBuffer.data();
    gl::Vector2* stylePtr = mSubBuffer.data();
    int bufferingVtxCount = 0;
    auto prevPos = aGetPos(aPosCount - 1);

    for (int i = 0; i < aPosCount; ++i)
    {
        auto currPos = aGetPos(i);

        if (!fMakeLinePolygon(prevPos, currPos, width,
                              posPtr + bufferingVtxCount,
                              stylePtr + bufferingVtxCount))
        {
            continue;
        }
        bufferingVtxCount += 4;

        if (bufferingVtxCount >= maxVtxCount)
        {
            command.attr.draw.count = bufferingVtxCount;
            pushDrawCommand(command, posPtr, stylePtr);
            posPtr += bufferingVtxCount;
            stylePtr += bufferingVtxCount;
            bufferingVtxCount = 0;
        }

        prevPos = currPos;
    }

    if (bufferingVtxCount > 0)
    {
        command.attr.draw.count = bufferingVtxCount;
        pushDrawCommand(command, posPtr, stylePtr);
    }
}

void PrimitiveDrawer::drawPoint(const QPointF& aCenter)
{
    static const int kDivision = 8;
    drawEllipseImpl(aCenter, 1.0f, 1.0f, kDivision);
}

void PrimitiveDrawer::drawLine(const QPointF& aFrom, const QPointF& aTo)
{
    if (!mScheduledState.hasPen) return;

    Command command = { Type_Draw };
    command.attr.draw.prim = GL_TRIANGLE_STRIP;
    command.attr.draw.count = 4;
    command.attr.draw.usePen = true;

    const float width = mScheduledState.penWidth * mPixelScale;
    gl::Vector2 positions[4];
    gl::Vector2 styleCoords[4];
    if (!fMakeLinePolygon(aFrom, aTo, width, positions, styleCoords))
    {
        return;
    }

    pushDrawCommand(command, positions, styleCoords);
}

void PrimitiveDrawer::drawRect(const QRect& aRect)
{
    drawRect(QRectF(aRect));
}

void PrimitiveDrawer::drawRect(const QRectF& aRect)
{
    Command command = { Type_Draw };
    command.attr.draw.prim = GL_TRIANGLE_STRIP;
    command.attr.draw.count = 4;
    command.attr.draw.usePen = false;
    gl::Vector2 positions[4] = {
        gl::Vector2::make(aRect.left() , aRect.top()   ),
        gl::Vector2::make(aRect.left() , aRect.bottom()),
        gl::Vector2::make(aRect.right(), aRect.top()   ),
        gl::Vector2::make(aRect.right(), aRect.bottom())
    };
    pushDrawCommand(command, positions);

    gl::Vector2 outlines[4] = { positions[0], positions[1], positions[3], positions[2] };
    drawOutline([&](int aIndex){ return outlines[aIndex].pos().toPointF(); }, 4);
}

void PrimitiveDrawer::drawEllipse(const QPointF& aCenter, float aRadiusX, float aRadiusY)
{
    static const int kDivision = 30;
    drawEllipseImpl(aCenter, aRadiusX, aRadiusY, kDivision);
}

void PrimitiveDrawer::drawCircle(const QPointF& aCenter, float aRadius)
{
    static const int kDivision = 30;
    drawEllipseImpl(aCenter, aRadius, aRadius, kDivision);
}

void PrimitiveDrawer::drawEllipseImpl(const QPointF& aCenter, float aRadiusX, float aRadiusY, int aDivision)
{
    XC_ASSERT(3 <= aDivision && aDivision <= 30);
    const int kMaxVtxCount = 32;
    const int vtxCount = aDivision + 2;

    Command command = { Type_Draw };
    command.attr.draw.prim = GL_TRIANGLE_FAN;
    command.attr.draw.count = vtxCount;
    command.attr.draw.usePen = false;

    gl::Vector2 positions[kMaxVtxCount];
    positions[0].set(aCenter.x(), aCenter.y());
    for (int i = 1; i < vtxCount; ++i)
    {
        const float angleRad = (float)(2.0 * M_PI * (1.0 - (double)(i - 1) / aDivision));
        auto vec = util::MathUtil::getVectorFromPolarCoord(1.0f, angleRad);
        auto pos = aCenter + QPointF(vec.x() * aRadiusX, vec.y() * aRadiusY);
        positions[i].set(pos.x(), pos.y());
    }
    pushDrawCommand(command, positions);

    drawOutline([&](int aIndex){ return (positions + 2)[aIndex].pos().toPointF(); }, vtxCount - 2);
}

void PrimitiveDrawer::drawPolyline(const QPoint* aPoints, int aCount)
{
    drawOutline([=](int aIndex){ return QPointF(aPoints[aIndex]); }, aCount, true);
}

void PrimitiveDrawer::drawPolyline(const QPointF* aPoints, int aCount)
{
    drawOutline([=](int aIndex){ return aPoints[aIndex]; }, aCount, true);
}

void PrimitiveDrawer::drawConvexPolygonImpl(const std::function<QPointF(int)>& aGetPos, int aCount)
{
    if (aCount < 3) return;

    const int maxOnceCount = mVtxCountOfSlot;
    mPosBuffer.resize(maxOnceCount);

    gl::Vector2* dst = mPosBuffer.data();
    int srcIdx = 0;
    auto srcV0 = gl::Vector2::make(QVector2D(aGetPos(srcIdx))); ++srcIdx;
    auto srcV1 = gl::Vector2::make(QVector2D(aGetPos(srcIdx))); ++srcIdx;

    int remainCount = aCount;

    while (remainCount >= 3)
    {
        const int count = std::min(remainCount, maxOnceCount);
        Command command = { Type_Draw };
        command.attr.draw.prim = GL_TRIANGLE_FAN;
        command.attr.draw.count = count;
        command.attr.draw.usePen = false;

        dst[0] = srcV0;
        dst[1] = srcV1;
        for (int i = 2; i < count; ++i)
        {
            auto srcV2 = gl::Vector2::make(QVector2D(aGetPos(srcIdx))); ++srcIdx;
            dst[i] = srcV2;
            srcV1 = srcV2;
        }

        pushDrawCommand(command, dst);

        remainCount -= (count - 2);
    }
}

void PrimitiveDrawer::drawConvexPolygon(const QPoint* aPoints, int aCount)
{
    drawConvexPolygonImpl([=](int aIndex){ return QPoint(aPoints[aIndex]); }, aCount);
}

void PrimitiveDrawer::drawConvexPolygon(const QPointF* aPoints, int aCount)
{
    drawConvexPolygonImpl([=](int aIndex){ return aPoints[aIndex]; }, aCount);
}

void PrimitiveDrawer::drawPolygonImpl(const QVector<gl::Vector2>& aTriangles)
{
    const int maxOnceCount = 3 * (mVtxCountOfSlot / 3);
    const gl::Vector2* ptr = aTriangles.data();
    int remainCount = aTriangles.size();

    while (remainCount > 0)
    {
        const int count = std::min(remainCount, maxOnceCount);
        Command command = { Type_Draw };
        command.attr.draw.prim = GL_TRIANGLES;
        command.attr.draw.count = count;
        command.attr.draw.usePen = false;
        pushDrawCommand(command, ptr);

        remainCount -= count;
        ptr += count;
    }
}

void PrimitiveDrawer::drawPolygon(const QPoint* aPoints, int aCount)
{
    Triangulator tri(aPoints, aCount);
    if (!tri) return;
    drawPolygonImpl(tri.triangles());
    drawOutline([=](int aIndex){ return QPointF(aPoints[aIndex]); }, aCount);
}

void PrimitiveDrawer::drawPolygon(const QPointF* aPoints, int aCount)
{
    Triangulator tri(aPoints, aCount);
    if (!tri) return;
    drawPolygonImpl(tri.triangles());
    drawOutline([=](int aIndex){ return aPoints[aIndex]; }, aCount);
}

void PrimitiveDrawer::drawPolygon(const QPolygonF& aPolygon)
{
    drawPolygon(aPolygon.data(), aPolygon.size());
}

void PrimitiveDrawer::drawTexture(const QRectF& aRect, gl::Texture& aTexture)
{
    drawTexture(aRect, aTexture.id());
}

void PrimitiveDrawer::drawTexture(const QRectF& aRect, gl::Texture& aTexture, const QRectF& aSrcRect)
{
    drawTexture(aRect, aTexture.id(), aTexture.size(), aSrcRect);
}

void PrimitiveDrawer::drawTexture(const QRectF& aRect, GLuint aTexture)
{
    drawTexture(aRect, aTexture, QSize(1, 1), QRectF(0.0f, 0.0f, 1.0f, 1.0f));
}

void PrimitiveDrawer::drawTexture(const QRectF& aRect, GLuint aTexture, const QSize& aTexSize, const QRectF& aSrcRect)
{
    {
        Command command = { Type_Texture };
        command.attr.texture.id = aTexture;
        //command.attr.texture.color = QColor(255, 255, 255, 255).rgba();
        command.attr.texture.color = mScheduledState.brushColor;
        pushStateCommand(command);
    }

    {
        Command command = { Type_Draw };
        command.attr.draw.prim = GL_TRIANGLE_STRIP;
        command.attr.draw.count = 4;
        command.attr.draw.usePen = false;
        gl::Vector2 positions[4] = {
            gl::Vector2::make(aRect.left() , aRect.top()   ),
            gl::Vector2::make(aRect.left() , aRect.bottom()),
            gl::Vector2::make(aRect.right(), aRect.top()   ),
            gl::Vector2::make(aRect.right(), aRect.bottom())
        };
        const float tl = aSrcRect.left() / aTexSize.width();
        const float tt = 1.0f - aSrcRect.top() / aTexSize.height();
        const float tr = aSrcRect.right() / aTexSize.width();
        const float tb = 1.0f - aSrcRect.bottom() / aTexSize.height();

        gl::Vector2 texCoords[4] = {
            gl::Vector2::make(tl, tt), gl::Vector2::make(tl, tb),
            gl::Vector2::make(tr, tt), gl::Vector2::make(tr, tb)
        };
        pushDrawCommand(command, positions, texCoords);
    }

    {
        Command command = { Type_Texture };
        command.attr.texture.id = 0;
        command.attr.texture.color = QColor(255, 255, 255, 255).rgba();
        pushStateCommand(command);
    }
}

void PrimitiveDrawer::pushStateCommand(const Command& aCommand)
{
    XC_ASSERT(aCommand.type != Type_Draw);

    if (!mInDrawing)
    {
        mScheduledState.set(aCommand);
        mAppliedState = mScheduledState;
        return;
    }

    bool push = true;
    for (auto itr = mScheduledCommands.rbegin(); itr != mScheduledCommands.rend(); ++itr)
    {
        auto& command = *itr;
        if (command.type == Type_Draw)
        {
            break;
        }
        else if (command.type == aCommand.type)
        {
            command = aCommand;
            push = false;
            break;
        }
    }

    if (mScheduledState.hasDifferentValueWith(aCommand))
    {
        mScheduledState.set(aCommand);
        if (push)
        {
            mScheduledCommands.push_back(aCommand);
        }
    }
}

void PrimitiveDrawer::pushDrawCommand(
        const Command& aCommand, const gl::Vector2* aPositions, const gl::Vector2* aSubCoords)
{
    XC_ASSERT(aCommand.type == Type_Draw);

    const bool usePen = aCommand.attr.draw.usePen;
    if ((usePen && !mScheduledState.hasPen) || (!usePen && !mScheduledState.hasBrush))
    {
        return;
    }

    const int vtxCount = aCommand.attr.draw.count;

    if (!mInDrawing) return;
    if (vtxCount <= 0 || mVtxCountOfSlot < vtxCount) return;

    if (mVtxCountOfSlot < mCurrentSlotSize + vtxCount)
    {
        flushCommands();
    }

    Global::Functions& ggl = Global::functions();
    if (aPositions)
    {
        ggl.glBindBuffer(GL_ARRAY_BUFFER, mPosSlotIds[mCurrentSlotIndex]);
        ggl.glBufferSubData(GL_ARRAY_BUFFER, sizeof(gl::Vector2) * mCurrentSlotSize,
                            (GLsizeiptr)(sizeof(gl::Vector2) * vtxCount), aPositions);
        GL_CHECK_ERROR();
    }
    if (aSubCoords)
    {
        ggl.glBindBuffer(GL_ARRAY_BUFFER, mSubSlotIds[mCurrentSlotIndex]);
        ggl.glBufferSubData(GL_ARRAY_BUFFER, sizeof(gl::Vector2) * mCurrentSlotSize,
                            (GLsizeiptr)(sizeof(gl::Vector2) * vtxCount), aSubCoords);
        GL_CHECK_ERROR();
    }
    ggl.glBindBuffer(GL_ARRAY_BUFFER, 0);

    mScheduledCommands.push_back(aCommand);
    mCurrentSlotSize += vtxCount;
}

void PrimitiveDrawer::flushCommands()
{
    XC_ASSERT(mInDrawing);
    if (mScheduledCommands.empty()) return;

    // get current buffer index
    const int index = mCurrentSlotIndex;
    // to next buffer
    mCurrentSlotIndex = (mCurrentSlotIndex + 1) % mSlotCount;
    mCurrentSlotSize = 0;

    Global::Functions& ggl = Global::functions();

    mCurrentShader = ShaderType_TERM;
    bindAppositeShader(index);

    GLint offset = 0;
    for (auto& command : mScheduledCommands)
    {
        switch (command.type)
        {
        case Type_Draw:
        {
            bindAppositeShader(index);
            setColorToCurrentShader(command.attr.draw.usePen);
            ggl.glDrawArrays(command.attr.draw.prim, offset, command.attr.draw.count);
            offset += command.attr.draw.count;
        } break;

        case Type_Brush:
            mAppliedState.set(command);
            break;
        case Type_Pen:
            mAppliedState.set(command);
            bindAppositeShader(index);
            break;
        case Type_Texture:
            mAppliedState.set(command);
            //unbindCurrentShader();
            bindAppositeShader(index);
            ggl.glActiveTexture(GL_TEXTURE0);
            ggl.glBindTexture(GL_TEXTURE_2D, mAppliedState.texture);
            break;
        case Type_Ability:
            if (mAppliedState.hasMSAA != command.attr.ability.hasMSAA)
            {
                unbindCurrentShader();
                Util::setAbility(GL_MULTISAMPLE, command.attr.ability.hasMSAA);
                bindAppositeShader(index);
            }
            mAppliedState.set(command);
            break;
        default:
            break;
        }
    }
    ggl.glBindBuffer(GL_ARRAY_BUFFER, 0);
    unbindCurrentShader();
    GL_CHECK_ERROR();

    mScheduledCommands.clear();
}

void PrimitiveDrawer::bindAppositeShader(int aSlotIndex)
{
    Global::Functions& ggl = Global::functions();

    auto prevType = mCurrentShader;
    auto nextType = ShaderType_Plane;

    if (mAppliedState.texture != 0)
    {
        nextType = ShaderType_Texture;
    }
    else if (mAppliedState.penStyle != PenStyle_Solid)
    {
        nextType = ShaderType_Stipple;
    }

    if (prevType == nextType) return;

    // unbind previous shader
    unbindCurrentShader();

    // bind next shader
    if (nextType == ShaderType_Texture)
    {
        mTextureShader.program.bind();
        ggl.glBindBuffer(GL_ARRAY_BUFFER, mPosSlotIds[aSlotIndex]);
        mTextureShader.program.setAttributeBuffer(mTextureShader.locPosition, GL_FLOAT, 2);
        ggl.glBindBuffer(GL_ARRAY_BUFFER, mSubSlotIds[aSlotIndex]);
        mTextureShader.program.setAttributeBuffer(mTextureShader.locTexCoord, GL_FLOAT, 2);
        mTextureShader.program.setUniformValue(mTextureShader.locViewMtx, mViewMtx);
        mTextureShader.program.setUniformValue(mTextureShader.locTexture, 0);
        mCurrentShader = ShaderType_Texture;
    }
    else if (nextType == ShaderType_Stipple)
    {
        const QVector2D scrHalfSize(mScreenSize.width() / 2, mScreenSize.height() / 2);
        mStippleShader.program.bind();
        ggl.glBindBuffer(GL_ARRAY_BUFFER, mPosSlotIds[aSlotIndex]);
        mStippleShader.program.setAttributeBuffer(mStippleShader.locPosition, GL_FLOAT, 2);
        ggl.glBindBuffer(GL_ARRAY_BUFFER, mSubSlotIds[aSlotIndex]);
        mStippleShader.program.setAttributeBuffer(mStippleShader.locLength, GL_FLOAT, 2);
        mStippleShader.program.setUniformValue(mStippleShader.locViewMtx, mViewMtx);
        mStippleShader.program.setUniformValue(mStippleShader.locScreenSize, scrHalfSize);
        mStippleShader.program.setUniformValue(mStippleShader.locWave,
                                               (mAppliedState.penStyle == PenStyle_Dash) ?
                                                   QVector2D(0.15f, 0.9f) : QVector2D(0.3f, 0.2f));
        mCurrentShader = ShaderType_Stipple;
    }
    else
    {
        mPlaneShader.program.bind();
        ggl.glBindBuffer(GL_ARRAY_BUFFER, mPosSlotIds[aSlotIndex]);
        mPlaneShader.program.setAttributeBuffer(mPlaneShader.locPosition, GL_FLOAT, 2);
        mPlaneShader.program.setUniformValue(mPlaneShader.locViewMtx, mViewMtx);
        mCurrentShader = ShaderType_Plane;
    }

}

void PrimitiveDrawer::setColorToCurrentShader(bool aUsePen)
{
    auto color = QColor::fromRgba(aUsePen ? mAppliedState.penColor : mAppliedState.brushColor);
    const QVector4D colorVec(color.redF(), color.greenF(), color.blueF(), color.alphaF());

    if (mCurrentShader == ShaderType_Plane)
    {
        mPlaneShader.program.setUniformValue(mPlaneShader.locColor, colorVec);
    }
    else if (mCurrentShader == ShaderType_Stipple)
    {
        mStippleShader.program.setUniformValue(mStippleShader.locColor, colorVec);
    }
    else if (mCurrentShader == ShaderType_Texture)
    {
        auto texColor = QColor::fromRgba(mAppliedState.textureColor);
        const QVector4D texColorVec(texColor.redF(), texColor.greenF(), texColor.blueF(), texColor.alphaF());
        mTextureShader.program.setUniformValue(mTextureShader.locColor, texColorVec);
    }
}

void PrimitiveDrawer::unbindCurrentShader()
{
    if (mCurrentShader == ShaderType_Plane)
    {
        mPlaneShader.program.release();
    }
    else if (mCurrentShader == ShaderType_Stipple)
    {
        mStippleShader.program.release();
    }
    else if (mCurrentShader == ShaderType_Texture)
    {
        mTextureShader.program.release();
        Global::Functions& ggl = Global::functions();
        ggl.glActiveTexture(GL_TEXTURE0);
        ggl.glBindTexture(GL_TEXTURE_2D, 0);
    }
    mCurrentShader = ShaderType_TERM;
}

} // namespace gl
