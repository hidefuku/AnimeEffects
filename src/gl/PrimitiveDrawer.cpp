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
    , useMSAA(false)
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
            useMSAA == aRhs.useMSAA;
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
        break;
    case Type_MSAA:
        useMSAA = aCommand.attr.msaa.use;
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
        return texture != aCommand.attr.texture.id;
    case Type_MSAA:
        return useMSAA != aCommand.attr.msaa.use;
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
    , mBufferingCommands()
    , mAppliedState()
    , mBufferingState()
    , mInDrawing(false)
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
    Util::setAbility(GL_MULTISAMPLE, mAppliedState.useMSAA);
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

void PrimitiveDrawer::setAntiAliasing(bool aIsEnable)
{
    Command command = { Type_MSAA };
    command.attr.msaa.use = aIsEnable;
    pushStateCommand(command);
}

void PrimitiveDrawer::drawPoint(const QPointF& aCenter)
{
    static const int kDivision = 8;
    drawEllipseImpl(aCenter, 1.0f, 1.0f, kDivision);
}

void PrimitiveDrawer::drawLine(const QPointF& aFrom, const QPointF& aTo)
{
    Command command = { Type_Draw };
    command.attr.draw.prim = GL_TRIANGLE_STRIP;
    command.attr.draw.count = 4;

    const bool usePen = mBufferingState.penStyle != PenStyle_None && mBufferingState.penWidth > 0.0f;
    command.attr.draw.usePen = usePen;

    const QVector2D dir(aTo - aFrom);
    const float length = dir.length();
    if (length < FLT_EPSILON) return;
    const float width = usePen ? mBufferingState.penWidth * mPixelScale : 1.0f;
    auto rside = util::MathUtil::getRotateVector90Deg(dir).normalized() * width * 0.5f;

    gl::Vector2 positions[4] = {
        gl::Vector2::make(aFrom.x() - rside.x(), aFrom.y() - rside.y()),
        gl::Vector2::make(aFrom.x() + rside.x(), aFrom.y() + rside.y()),
        gl::Vector2::make(aTo.x() - rside.x(), aTo.y() - rside.y()),
        gl::Vector2::make(aTo.x() + rside.x(), aTo.y() + rside.y())
    };
    gl::Vector2 subCoords[4] = {
        //positions[0],
        //positions[0]
        gl::Vector2::make(0.0f, 0.0f),
        gl::Vector2::make(0.0f, 0.0f),
        gl::Vector2::make(length, 0.0f),
        gl::Vector2::make(length, 0.0f)
    };
    pushDrawCommand(command, positions, subCoords);
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
    XC_ASSERT(aDivision <= 30);
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
}

void PrimitiveDrawer::drawPolygonImpl(const QVector<gl::Vector2>& aTriangles)
{
    const gl::Vector2* ptr = aTriangles.data();
    int remainCount = aTriangles.size();

    while (remainCount > 0)
    {
        const int count = std::min(remainCount, mVtxCountOfSlot);
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
}

void PrimitiveDrawer::drawPolygon(const QPointF* aPoints, int aCount)
{
    Triangulator tri(aPoints, aCount);
    if (!tri) return;
    drawPolygonImpl(tri.triangles());
}

void PrimitiveDrawer::drawPolygon(const QPolygonF& aPolygon)
{
    drawPolygon(aPolygon.data(), aPolygon.size());
}

void PrimitiveDrawer::drawTexture(const QRectF& aRect, gl::Texture& aTexture)
{
    drawTexture(aRect, aTexture.id());
}

void PrimitiveDrawer::drawTexture(const QRectF& aRect, GLuint aTexture)
{
    {
        Command command = { Type_Texture };
        command.attr.texture.id = aTexture;
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
        gl::Vector2 texCoords[4] = {
            gl::Vector2::make(0.0f, 1.0f),
            gl::Vector2::make(0.0f, 0.0f),
            gl::Vector2::make(1.0f, 1.0f),
            gl::Vector2::make(1.0f, 0.0f)
        };
        pushDrawCommand(command, positions, texCoords);
    }

    {
        Command command = { Type_Texture };
        command.attr.texture.id = 0;
        pushStateCommand(command);
    }
}

#if 0
void PrimitiveDrawer::drawText(const QPointF& aPos, const QFont& aFont, const QString& aText)
{
    //QFont thinFont = aFont;
    //qDebug() << aFont.pi
    //thinFont.setWeight(QFont::Thin);
    QPainterPath path;
    path.addText(aPos, aFont, aText);
    auto polygons = path.toSubpathPolygons();

    QVector<gl::Vector2> positions;

    int k = 0;
    for (auto& polygon : polygons)
    {
        auto count = polygon.size();
        if (count <= 0) continue;
        Command command = { Type_Draw };
        command.attr.draw.prim = GL_LINE_STRIP;
        command.attr.draw.count = count;
        positions.resize(count);
        for (int i = 0; i < count; ++i)
        {
            auto pos = polygon[i];
            positions[i].set(pos.x(), pos.y());
        }
        k = k % 7;
        switch (k)
        {
        case 0: setColor(QColor(0, 0, 0, 255)); break;
        case 1: setColor(QColor(255, 0, 0, 255)); break;
        case 2: setColor(QColor(0, 255, 0, 255)); break;
        case 3: setColor(QColor(0, 0, 255, 255)); break;
        case 4: setColor(QColor(255, 255, 0, 255)); break;
        case 5: setColor(QColor(0, 255, 255, 255)); break;
        case 6: setColor(QColor(255, 255, 255, 255)); break;
        }
        ++k;

        pushDrawCommand(command, positions.data());
    }
}
#endif

void PrimitiveDrawer::pushStateCommand(const Command& aCommand)
{
    XC_ASSERT(aCommand.type != Type_Draw);

    if (!mInDrawing)
    {
        mBufferingState.set(aCommand);
        mAppliedState = mBufferingState;
        return;
    }

    for (auto itr = mBufferingCommands.rbegin(); itr != mBufferingCommands.rend(); ++itr)
    {
        auto& command = *itr;
        if (command.type == Type_Draw)
        {
            break;
        }
        else if (command.type == aCommand.type)
        {
            command = aCommand;
            return;
        }
    }

    if (mBufferingState.hasDifferentValueWith(aCommand))
    {
        mBufferingCommands.push_back(aCommand);
        mBufferingState.set(aCommand);
    }
}

void PrimitiveDrawer::pushDrawCommand(
        const Command& aCommand, const gl::Vector2* aPositions, const gl::Vector2* aSubCoords)
{
    XC_ASSERT(aCommand.type == Type_Draw);
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

    mBufferingCommands.push_back(aCommand);
    mCurrentSlotSize += vtxCount;
}

void PrimitiveDrawer::flushCommands()
{
    XC_ASSERT(mInDrawing);
    if (mBufferingCommands.empty()) return;

    // get current buffer index
    const int index = mCurrentSlotIndex;
    // to next buffer
    mCurrentSlotIndex = (mCurrentSlotIndex + 1) % mSlotCount;
    mCurrentSlotSize = 0;

    Global::Functions& ggl = Global::functions();

    bool usePen = false;
    mCurrentShader = ShaderType_TERM;
    bindAppositeShader(index, usePen);

    GLint offset = 0;
    for (auto& command : mBufferingCommands)
    {
        switch (command.type)
        {
        case Type_Draw:
        {
            if (usePen != command.attr.draw.usePen)
            {
                usePen = command.attr.draw.usePen;
                bindAppositeShader(index, usePen);
            }

            setColorToCurrentShader(QColor::fromRgba(usePen ? mAppliedState.penColor : mAppliedState.brushColor));
            ggl.glDrawArrays(command.attr.draw.prim, offset, command.attr.draw.count);
            offset += command.attr.draw.count;
        } break;

        case Type_Brush:
            mAppliedState.set(command);
            break;
        case Type_Pen:
            mAppliedState.set(command);
            if (usePen) bindAppositeShader(index, usePen);
            break;
        case Type_Texture:
            mAppliedState.set(command);
            bindAppositeShader(index, usePen);
            break;
        case Type_MSAA:
            Util::setAbility(GL_MULTISAMPLE, command.attr.msaa.use);
            mAppliedState.set(command);
            break;
        default:
            break;
        }
    }
    ggl.glBindBuffer(GL_ARRAY_BUFFER, 0);
    unbindCurrentShader();
    GL_CHECK_ERROR();

    mBufferingCommands.clear();
}

void PrimitiveDrawer::bindAppositeShader(int aSlotIndex, bool aUsePen)
{
    Global::Functions& ggl = Global::functions();

    unbindCurrentShader();

    if (mAppliedState.texture != 0)
    {
        ggl.glActiveTexture(GL_TEXTURE0);
        ggl.glBindTexture(GL_TEXTURE_2D, mAppliedState.texture);

        mTextureShader.program.bind();
        ggl.glBindBuffer(GL_ARRAY_BUFFER, mPosSlotIds[aSlotIndex]);
        mTextureShader.program.setAttributeBuffer(mTextureShader.locPosition, GL_FLOAT, 2);
        ggl.glBindBuffer(GL_ARRAY_BUFFER, mSubSlotIds[aSlotIndex]);
        mTextureShader.program.setAttributeBuffer(mTextureShader.locTexCoord, GL_FLOAT, 2);
        mTextureShader.program.setUniformValue(mTextureShader.locViewMtx, mViewMtx);
        mTextureShader.program.setUniformValue(mTextureShader.locTexture, 0);
        mCurrentShader = ShaderType_Texture;
    }
    else if (aUsePen && mAppliedState.penStyle != PenStyle_None && mAppliedState.penStyle != PenStyle_Solid)
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
                                                   QVector2D(0.15f, 0.9f) : QVector2D(0.25f, -0.2f));
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

void PrimitiveDrawer::setColorToCurrentShader(const QColor& aColor)
{
    const QVector4D colorVec(aColor.redF(), aColor.greenF(), aColor.blueF(), aColor.alphaF());

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
        mTextureShader.program.setUniformValue(mTextureShader.locColor, colorVec);
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
