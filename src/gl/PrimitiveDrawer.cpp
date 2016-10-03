#include "util/MathUtil.h"
#include "gl/Global.h"
#include "gl/Util.h"
#include "gl/PrimitiveDrawer.h"

namespace
{
static const int kMinSlotSize = 32;
}

namespace gl
{

//-------------------------------------------------------------------------------------------------
PrimitiveDrawer::State::State()
    : color(QColor(0, 0, 0, 255).rgba())
    , texture(0)
    , useMSAA(false)
{
}

bool PrimitiveDrawer::State::operator==(const State& aRhs) const
{
    return color == aRhs.color &&
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
    case Type_Color:
        color = aCommand.attr.color.value;
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
    case Type_Color:
        return color != aCommand.attr.color.value;
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
PrimitiveDrawer::PrimitiveDrawer(int aSlotSize, int aSlotCount)
    : mShader()
    , mLocationOfPos(-1)
    , mLocationOfViewMtx(-1)
    , mLocationOfColor(-1)
    , mSlotSize(aSlotSize > kMinSlotSize ? aSlotSize : kMinSlotSize)
    , mSlotCount(aSlotCount > 1 ? aSlotCount : 1)
    , mSlotIds()
    , mCurrentSlotIndex(0)
    , mCurrentSlotSize(0)
    , mViewMtx()
    , mBufferingCommands()
    , mCurrentState()
    , mBufferingState()
    , mInDrawing(false)
{
    // create shader
    initShader();

    // create buffer
    {
        Global::Functions& ggl = Global::functions();

        mSlotIds.resize(mSlotCount);
        ggl.glGenBuffers(mSlotCount, mSlotIds.data());

        auto bufferSize = (GLsizeiptr)(sizeof(gl::Vector2) * mSlotSize);
        for (int i = 0; i < mSlotCount; ++i)
        {
            ggl.glBindBuffer(GL_ARRAY_BUFFER, mSlotIds[i]);
            ggl.glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);
        }
        ggl.glBindBuffer(GL_ARRAY_BUFFER, 0);
        GL_CHECK_ERROR();
    }
}

PrimitiveDrawer::~PrimitiveDrawer()
{
    // delete buffer
    Global::functions().glDeleteBuffers(mSlotCount, mSlotIds.data());
    GL_CHECK_ERROR();
}

bool PrimitiveDrawer::initShader()
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

    if (!mShader.setVertexSource(QString(kVertexShaderText)))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile vertex shader.",
                       mShader.log());
        return false;
    }
    if (!mShader.setFragmentSource(QString(kFragmentShaderText)))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile fragment shader.",
                       mShader.log());
        return false;
    }
    if (!mShader.link())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.",
                       mShader.log());
        return false;
    }

    mLocationOfPos = mShader.attributeLocation("inPosition");
    mLocationOfViewMtx = mShader.uniformLocation("uViewMtx");
    mLocationOfColor = mShader.uniformLocation("uColor");

    GL_CHECK_ERROR();
    return true;
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
    Util::setAbility(GL_MULTISAMPLE, mCurrentState.useMSAA);
}

void PrimitiveDrawer::end()
{
    XC_ASSERT(mInDrawing);
    flushCommands();
    mInDrawing = false;
}

void PrimitiveDrawer::setViewMtx(const QMatrix4x4& aViewMtx)
{
    mViewMtx = aViewMtx;
}

void PrimitiveDrawer::setColor(const QColor& aColor)
{
    Command command = { Type_Color };
    command.attr.color.value = aColor.rgba();
    pushStateCommand(command);
}

void PrimitiveDrawer::setAntiAliasing(bool aIsEnable)
{
    Command command = { Type_MSAA };
    command.attr.msaa.use = aIsEnable;
    pushStateCommand(command);
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
    gl::Vector2 positions[4] = {
        gl::Vector2::make(aRect.left() , aRect.top()   ),
        gl::Vector2::make(aRect.left() , aRect.bottom()),
        gl::Vector2::make(aRect.right(), aRect.top()   ),
        gl::Vector2::make(aRect.right(), aRect.bottom())
    };
    pushDrawCommand(command, positions);
}

void PrimitiveDrawer::drawCircle(const QPointF& aCenter, float aRadius)
{
    static const int kDivision = 30;
    static const int kVtxCount = kDivision + 2;

    Command command = { Type_Draw };
    command.attr.draw.prim = GL_TRIANGLE_FAN;
    command.attr.draw.count = kVtxCount;

    gl::Vector2 positions[kVtxCount];
    positions[0].set(aCenter.x(), aCenter.y());
    for (int i = 1; i < kVtxCount; ++i)
    {
        const float angleRad = (float)(2.0 * M_PI * (1.0 - (double)(i - 1) / kDivision));
        auto pos = aCenter + util::MathUtil::getVectorFromPolarCoord(aRadius, angleRad).toPointF();
        positions[i].set(pos.x(), pos.y());
    }
    pushDrawCommand(command, positions);
}

void PrimitiveDrawer::drawLine(const QPointF& aFrom, const QPointF& aTo)
{
    Command command = { Type_Draw };
    command.attr.draw.prim = GL_LINES;
    command.attr.draw.count = 2;

    gl::Vector2 positions[2] = {
        gl::Vector2::make(aFrom.x(), aFrom.y()),
        gl::Vector2::make(aTo.x()  , aTo.y()  )
    };
    pushDrawCommand(command, positions);
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
        mCurrentState = mBufferingState;
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

void PrimitiveDrawer::pushDrawCommand(const Command& aCommand, gl::Vector2* aPositions)
{
    XC_ASSERT(aCommand.type == Type_Draw);
    const int vtxCount = aCommand.attr.draw.count;

    if (!mInDrawing) return;
    if (vtxCount <= 0 || mSlotSize < vtxCount) return;

    if (mSlotSize < mCurrentSlotSize + vtxCount)
    {
        flushCommands();
    }

    Global::Functions& ggl = Global::functions();
    ggl.glBindBuffer(GL_ARRAY_BUFFER, mSlotIds[mCurrentSlotIndex]);
    ggl.glBufferSubData(GL_ARRAY_BUFFER,
                        sizeof(gl::Vector2) * mCurrentSlotSize,
                        (GLsizeiptr)(sizeof(gl::Vector2) * vtxCount),
                        aPositions);
    ggl.glBindBuffer(GL_ARRAY_BUFFER, 0);
    GL_CHECK_ERROR();

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

    mShader.bind();

    ggl.glBindBuffer(GL_ARRAY_BUFFER, mSlotIds[index]);
    mShader.setAttributeBuffer(mLocationOfPos, GL_FLOAT, 2);
    mShader.setUniformValue(mLocationOfViewMtx, mViewMtx);

    GLint offset = 0;
    for (auto& command : mBufferingCommands)
    {
        switch (command.type)
        {
        case Type_Draw:
        {
            const QColor color = QColor::fromRgba(mCurrentState.color);
            const QVector4D colorVec(color.redF(), color.greenF(), color.blueF(), color.alphaF());
            {
                mShader.setUniformValue(mLocationOfColor, colorVec);
                ggl.glDrawArrays(command.attr.draw.prim, offset, command.attr.draw.count);
            }
            offset += command.attr.draw.count;
        } break;

        case Type_Color:
            mCurrentState.set(command);
            break;
        case Type_Texture:
            mCurrentState.set(command);
            break;
        case Type_MSAA:
            Util::setAbility(GL_MULTISAMPLE, command.attr.msaa.use);
            mCurrentState.set(command);
            break;
        default:
            break;
        }
    }
    ggl.glBindBuffer(GL_ARRAY_BUFFER, 0);
    mShader.release();
    GL_CHECK_ERROR();

    mBufferingCommands.clear();
}

} // namespace gl
