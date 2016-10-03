#ifndef GL_PRIMITIVEDRAWER_H
#define GL_PRIMITIVEDRAWER_H

#include <vector>
#include "gl/BufferObject.h"
#include "gl/EasyShaderProgram.h"

namespace gl
{

class PrimitiveDrawer
{
public:
    PrimitiveDrawer(int aSlotSize = 512, int aSlotCount = 8);
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

    bool initShader();
    void pushStateCommand(const Command& aCommand);
    void pushDrawCommand(const Command& aCommand, gl::Vector2* aPositions);
    void flushCommands();

    gl::EasyShaderProgram mShader;
    int mLocationOfPos;
    int mLocationOfViewMtx;
    int mLocationOfColor;

    int mSlotSize;
    int mSlotCount;
    std::vector<GLuint> mSlotIds;
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
