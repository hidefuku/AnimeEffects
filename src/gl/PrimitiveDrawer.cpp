#include "util/MathUtil.h"
#include "gl/Global.h"
#include "gl/Util.h"
#include "gl/Triangulator.h"
#include "gl/PrimitiveDrawer.h"

namespace
{
static const int kMinVtxCountOfSlot = 32;
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

    if (!program.setVertexSource(QString(kVertexShaderText)))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile vertex shader.",
                       program.log());
        return false;
    }
    if (!program.setFragmentSource(QString(kFragmentShaderText)))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile fragment shader.",
                       program.log());
        return false;
    }
    if (!program.link())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.",
                       program.log());
        return false;
    }

    locPosition = program.attributeLocation("inPosition");
    locViewMtx = program.uniformLocation("uViewMtx");
    locColor = program.uniformLocation("uColor");

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

    if (!program.setVertexSource(QString(kVertexShaderText)))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile vertex shader.",
                       program.log());
        return false;
    }
    if (!program.setFragmentSource(QString(kFragmentShaderText)))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile fragment shader.",
                       program.log());
        return false;
    }
    if (!program.link())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.",
                       program.log());
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
    , mTextureShader()
    , mVtxCountOfSlot(std::max(aVtxCountOfSlot, kMinVtxCountOfSlot))
    , mSlotCount(std::max(aSlotCount, 1))
    , mPosSlotIds()
    , mTexSlotIds()
    , mCurrentSlotIndex(0)
    , mCurrentSlotSize(0)
    , mViewMtx()
    , mBufferingCommands()
    , mCurrentState()
    , mBufferingState()
    , mInDrawing(false)
{
    // create shader
    mPlaneShader.init();
    mTextureShader.init();

    // create buffer
    {
        Global::Functions& ggl = Global::functions();

        mPosSlotIds.resize(mSlotCount);
        mTexSlotIds.resize(mSlotCount);
        ggl.glGenBuffers(mSlotCount, mPosSlotIds.data());
        ggl.glGenBuffers(mSlotCount, mTexSlotIds.data());

        auto bufferSize = (GLsizeiptr)(sizeof(gl::Vector2) * mVtxCountOfSlot);
        for (int i = 0; i < mSlotCount; ++i)
        {
            ggl.glBindBuffer(GL_ARRAY_BUFFER, mPosSlotIds[i]);
            ggl.glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);
            ggl.glBindBuffer(GL_ARRAY_BUFFER, mTexSlotIds[i]);
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
    Global::functions().glDeleteBuffers(mSlotCount, mTexSlotIds.data());
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

void PrimitiveDrawer::drawPolygon(const QPolygonF& aPolygon)
{
    Triangulator tri(aPolygon);
    if (!tri) return;

    const gl::Vector2* ptr = tri.triangles().data();
    int remainCount = tri.triangles().size();
    while (remainCount > 0)
    {
        const int count = std::min(remainCount, mVtxCountOfSlot);
        Command command = { Type_Draw };
        command.attr.draw.prim = GL_TRIANGLES;
        command.attr.draw.count = count;
        pushDrawCommand(command, ptr);

        remainCount -= count;
        ptr += count;
    }
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
        gl::Vector2 positions[4] = {
            gl::Vector2::make(aRect.left() , aRect.top()   ),
            gl::Vector2::make(aRect.left() , aRect.bottom()),
            gl::Vector2::make(aRect.right(), aRect.top()   ),
            gl::Vector2::make(aRect.right(), aRect.bottom())
        };
        /*
        gl::Vector2 texCoords[4] = {
            gl::Vector2::make(0.0f, 0.0f),
            gl::Vector2::make(0.0f, 1.0f),
            gl::Vector2::make(1.0f, 0.0f),
            gl::Vector2::make(1.0f, 1.0f)
        };
        */
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

void PrimitiveDrawer::pushDrawCommand(
        const Command& aCommand, const gl::Vector2* aPositions, const gl::Vector2* aTexCoords)
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
    if (aTexCoords)
    {
        ggl.glBindBuffer(GL_ARRAY_BUFFER, mTexSlotIds[mCurrentSlotIndex]);
        ggl.glBufferSubData(GL_ARRAY_BUFFER, sizeof(gl::Vector2) * mCurrentSlotSize,
                            (GLsizeiptr)(sizeof(gl::Vector2) * vtxCount), aTexCoords);
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

    bindAppositeShader(index);

    GLint offset = 0;
    for (auto& command : mBufferingCommands)
    {
        switch (command.type)
        {
        case Type_Draw:
        {
            setColorToCurrentShader(QColor::fromRgba(mCurrentState.color));
            ggl.glDrawArrays(command.attr.draw.prim, offset, command.attr.draw.count);
            offset += command.attr.draw.count;
        } break;

        case Type_Color:
            mCurrentState.set(command);
            break;
        case Type_Texture:
            mCurrentState.set(command);
            bindAppositeShader(index);
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
    unbindCurrentShader();
    GL_CHECK_ERROR();

    mBufferingCommands.clear();
}

void PrimitiveDrawer::bindAppositeShader(int aSlotIndex)
{
    Global::Functions& ggl = Global::functions();

    unbindCurrentShader();

    if (mCurrentState.texture == 0)
    {
        ggl.glActiveTexture(GL_TEXTURE0);
        ggl.glBindTexture(GL_TEXTURE_2D, 0);

        mPlaneShader.program.bind();
        ggl.glBindBuffer(GL_ARRAY_BUFFER, mPosSlotIds[aSlotIndex]);
        mPlaneShader.program.setAttributeBuffer(mPlaneShader.locPosition, GL_FLOAT, 2);
        mPlaneShader.program.setUniformValue(mPlaneShader.locViewMtx, mViewMtx);
    }
    else
    {
        ggl.glActiveTexture(GL_TEXTURE0);
        ggl.glBindTexture(GL_TEXTURE_2D, mCurrentState.texture);

        mTextureShader.program.bind();
        ggl.glBindBuffer(GL_ARRAY_BUFFER, mPosSlotIds[aSlotIndex]);
        mTextureShader.program.setAttributeBuffer(mTextureShader.locPosition, GL_FLOAT, 2);
        ggl.glBindBuffer(GL_ARRAY_BUFFER, mTexSlotIds[aSlotIndex]);
        mTextureShader.program.setAttributeBuffer(mTextureShader.locTexCoord, GL_FLOAT, 2);
        mTextureShader.program.setUniformValue(mTextureShader.locViewMtx, mViewMtx);
        mTextureShader.program.setUniformValue(mTextureShader.locTexture, 0);
    }

}

void PrimitiveDrawer::setColorToCurrentShader(const QColor& aColor)
{
    const QVector4D colorVec(aColor.redF(), aColor.greenF(), aColor.blueF(), aColor.alphaF());
    {
        if (mCurrentState.texture == 0)
        {
            mPlaneShader.program.setUniformValue(mPlaneShader.locColor, colorVec);
        }
        else
        {
            mTextureShader.program.setUniformValue(mTextureShader.locColor, colorVec);
        }
    }
}

void PrimitiveDrawer::unbindCurrentShader()
{
    mPlaneShader.program.release();
    mTextureShader.program.release();

    Global::Functions& ggl = Global::functions();
    ggl.glActiveTexture(GL_TEXTURE0);
    ggl.glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace gl
