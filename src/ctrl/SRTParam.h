#ifndef CTRL_SRTPARAM
#define CTRL_SRTPARAM

namespace ctrl
{

class SRTParam
{
public:
    SRTParam()
        : mode(0)
        , necessarilyMove()
        , necessarilyRotate()
        , necessarilyScale()
        , adjustPosition(true)
    {}
    int mode;
    bool necessarilyMove;
    bool necessarilyRotate;
    bool necessarilyScale;
    bool adjustPosition;
};

} // namespace ctrl

#endif // CTRL_SRTPARAM

