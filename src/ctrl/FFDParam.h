#ifndef CTRL_FFDPARAM
#define CTRL_FFDPARAM

namespace ctrl
{

class FFDParam
{
public:
    enum Type
    {
        Type_Drag,
        Type_Pencil,
        Type_Eraser,
        Type_TERM
    };

    FFDParam()
        : type(Type_Pencil)
        , hardness(1)
        , radius(100)
        , pressure(0.5f)
        , blur(0.0f)
        , eraseHardness(1)
        , eraseRadius(100)
        , erasePressure(0.1f)
        , focusRadius(1.0f)
    {}
    Type type;

    // deformer
    int hardness; // 0, 1, 2
    int radius; // 5~1000
    float pressure; // 0.0~1.0
    float blur; // 0.0~1.0

    // eraser
    int eraseHardness; // 0, 1, 2
    int eraseRadius; // 5~1000
    float erasePressure;

    // focuser
    float focusRadius;
};

} // namespace ctrl

#endif // CTRL_FFDPARAM

