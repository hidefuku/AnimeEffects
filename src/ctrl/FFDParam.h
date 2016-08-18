#ifndef CTRL_FFDPARAM
#define CTRL_FFDPARAM

namespace ctrl
{

class FFDParam
{
public:
    FFDParam()
        : type(0)
        , hardness(1)
        , radius(100)
        , pressure(0.5f)
        , blur(0.0f)
        , eraseRadius(100)
        , erasePressure(0.1f)
    {}
    int type; // (0 = deformer), (1 = eraser)
    int hardness; // 0, 1, 2

    // deformer
    int radius; // 5~1000
    float pressure; // 0.0~1.0
    float blur; // 0.0~1.0

    // eraser
    int eraseRadius; // 5~1000
    float erasePressure;
};

} // namespace ctrl

#endif // CTRL_FFDPARAM

