#ifndef GL_PRIMITIVEDRAWER_H
#define GL_PRIMITIVEDRAWER_H

#include <vector>
#include <QPolygonF>
#include <QBrush>
#include <QPen>
#include "gl/BufferObject.h"
#include "gl/EasyShaderProgram.h"
#include "gl/Texture.h"

namespace gl
{

class PrimitiveDrawer
{
public:
    enum PenStyle
    {
        PenStyle_None,
        PenStyle_Solid,
        PenStyle_Dash,
        PenStyle_Dot,
        Style_TERM
    };

    PrimitiveDrawer(int aVtxCountOfSlot = 512, int aSlotCount = 8);
    virtual ~PrimitiveDrawer();

    void setViewMatrix(const QMatrix4x4& aViewMtx);

    void begin();
    void end();

    void setBrush(const QColor& aColor);
    void setPen(const QColor& aColor, float aWidth = 1.0f, PenStyle = PenStyle_Solid);
    void setAntiAliasing(bool aIsEnable);

    void drawPoint(const QPointF& aCenter);

    void drawLine(const QPointF& aFrom, const QPointF& aTo);
    void drawLine(const QLineF& aLine) { drawLine(aLine.p1(), aLine.p2()); }

    void drawRect(const QRect& aRect);
    void drawRect(const QRectF& aRect);

    void drawCircle(const QPointF& aCenter, float aRadius);
    void drawEllipse(const QPointF& aCenter, float aRadiusX, float aRadiusY);

    void drawPolygon(const QPoint* aPoints, int aCount);
    void drawPolygon(const QPointF* aPoints, int aCount);
    void drawPolygon(const QPolygonF& aPolygon);

    void drawTexture(const QRectF& aRect, gl::Texture& aTexture);
    void drawTexture(const QRectF& aRect, GLuint aTexture);

    //void drawText(const QPointF& aPos, const QFont& aFont, const QString& aText);

private:
    enum Type
    {
        Type_Draw,
        Type_Brush,
        Type_Pen,
        Type_Texture,
        Type_MSAA,
        Type_TERM
    };

    enum ShaderType
    {
        ShaderType_Plane,
        ShaderType_Stipple,
        ShaderType_Texture,
        ShaderType_TERM
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
                bool usePen;
            }draw;

            struct Brush
            {
                QRgb color;
            }brush;

            struct Pen
            {
                QRgb color;
                float width;
                PenStyle style;
            }pen;

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
        QRgb brushColor;
        QRgb penColor;
        float penWidth;
        PenStyle penStyle;
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

    struct StippleShader
    {
        bool init();
        gl::EasyShaderProgram program;
        int locPosition;
        int locLength;
        int locViewMtx;
        int locScreenSize;
        int locColor;
        int locWave;
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

    void drawPolygonImpl(const QVector<gl::Vector2>& aTriangles);
    void drawEllipseImpl(const QPointF& aCenter, float aRadiusX, float aRadiusY, int aDivision);

    void pushStateCommand(const Command& aCommand);
    void pushDrawCommand(const Command& aCommand,
                         const gl::Vector2* aPositions,
                         const gl::Vector2* aSubCoords = nullptr);
    void flushCommands();

    void bindAppositeShader(int aSlotIndex, bool aUsePen);
    void setColorToCurrentShader(const QColor& aColor);
    void unbindCurrentShader();

    PlaneShader mPlaneShader;
    StippleShader mStippleShader;
    TextureShader mTextureShader;
    ShaderType mCurrentShader;

    int mVtxCountOfSlot;
    int mSlotCount;
    std::vector<GLuint> mPosSlotIds;
    std::vector<GLuint> mSubSlotIds;
    int mCurrentSlotIndex;
    int mCurrentSlotSize;

    QMatrix4x4 mViewMtx;
    QSize mScreenSize;
    float mPixelScale;

    QVector<Command> mBufferingCommands;

    bool mInDrawing;
    State mAppliedState;
    State mBufferingState;
};

} // namespace gl

#endif // GL_PRIMITIVEDRAWER_H
