#ifndef GL_PRIMITIVEDRAWER_H
#define GL_PRIMITIVEDRAWER_H

#include <vector>
#include <QPolygonF>
#include "gl/BufferObject.h"
#include "gl/EasyShaderProgram.h"
#include "gl/Texture.h"

namespace gl
{

class PrimitiveDrawer
{
public:
    PrimitiveDrawer(int aVtxCountOfSlot = 512, int aSlotCount = 8);
    virtual ~PrimitiveDrawer();

    void setViewMtx(const QMatrix4x4& aViewMtx);

    void begin();
    void end();

    void setColor(const QColor& aColor);
    void setAntiAliasing(bool aIsEnable);

    void drawRect(const QRect& aRect);
    void drawRect(const QRectF& aRect);

    //void drawEllipse(const QPointF& aCenter, float aRadX, float aRadY);
    void drawCircle(const QPointF& aCenter, float aRadius);

    void drawLine(const QPointF& aFrom, const QPointF& aTo);

    void drawPolygon(const QPolygonF& aPolygon);

    void drawTexture(const QRectF& aRect, gl::Texture& aTexture);
    void drawTexture(const QRectF& aRect, GLuint aTexture);

    //void drawText(const QPointF& aPos, const QFont& aFont, const QString& aText);

private:
    enum Type
    {
        Type_Draw,
        Type_Color,
        Type_Texture,
        Type_MSAA,
        Type_TERM
    };

    struct Command
    {
        Type type;
        union Attribute
        {
            struct Draw
            {
                GLenum prim;
                int count;
            }draw;

            struct Color
            {
                QRgb value;
            }color;

            struct Texture
            {
                GLuint id;
            }texture;

            struct MSAA
            {
                bool use;
            }msaa;
        }attr;
    };

    struct State
    {
        State();
        void set(const Command& aCommand);
        bool hasDifferentValueWith(const Command& aCommand) const;
        bool operator==(const State& aRhs) const;
        bool operator!=(const State& aRhs) const;
        QRgb color;
        GLuint texture;
        bool useMSAA;
    };

    struct PlaneShader
    {
        bool init();
        gl::EasyShaderProgram program;
        int locPosition;
        int locViewMtx;
        int locColor;
    };
    struct TextureShader
    {
        bool init();
        gl::EasyShaderProgram program;
        int locPosition;
        int locTexCoord;
        int locViewMtx;
        int locColor;
        int locTexture;
    };

    void pushStateCommand(const Command& aCommand);
    void pushDrawCommand(const Command& aCommand,
                         const gl::Vector2* aPositions,
                         const gl::Vector2* aTexCoords = nullptr);
    void flushCommands();

    void bindAppositeShader(int aSlotIndex);
    void setColorToCurrentShader(const QColor& aColor);
    void unbindCurrentShader();

    PlaneShader mPlaneShader;
    TextureShader mTextureShader;

    int mVtxCountOfSlot;
    int mSlotCount;
    std::vector<GLuint> mPosSlotIds;
    std::vector<GLuint> mTexSlotIds;
    int mCurrentSlotIndex;
    int mCurrentSlotSize;

    QMatrix4x4 mViewMtx;

    QVector<Command> mBufferingCommands;

    bool mInDrawing;
    State mCurrentState;
    State mBufferingState;
};

} // namespace gl

#endif // GL_PRIMITIVEDRAWER_H
