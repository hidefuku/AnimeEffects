#ifndef UTIL_EASING
#define UTIL_EASING

#include <QString>
#include <QStringList>

namespace util
{

class Easing
{
public:
    enum Type
    {
        Type_None,
        Type_Linear,
        Type_Sine,
        Type_Quad,
        Type_Cubic,
        Type_Quart,
        Type_Quint,
        Type_Expo,
        Type_Circ,
        Type_Back,
        Type_Elastic,
        Type_Bounce,
        Type_TERM
    };

    enum Range
    {
        Range_In,
        Range_Out,
        Range_InOut,
        Range_TERM
    };

    struct Param
    {
        Param()
            : type(Type_Linear)
            , range(Range_InOut)
            , weight(1.0f)
        {}
        bool isValidParam() const;
        bool operator==(const Param& aRhs) const;
        inline bool operator!=(const Param& aRhs) const { return !(*this == aRhs); }
        Type type;
        Range range;
        float weight;
    };

    static QString getTypeName(Type aType);
    static QStringList getTypeNameList();

    static float calculate(Type, Range, float t, float b, float c, float d);
    static float calculate(Param, float t, float b, float c, float d);

    static float sineIn(float t, float b, float c, float d);
    static float sineOut(float t, float b, float c, float d);
    static float sineInOut(float t, float b, float c, float d);

    static float quadIn(float t, float b, float c, float d);
    static float quadOut(float t, float b, float c, float d);
    static float quadInOut(float t, float b, float c, float d);

    static float cubicIn(float t, float b, float c, float d);
    static float cubicOut(float t, float b, float c, float d);
    static float cubicInOut(float t, float b, float c, float d);

    static float quartIn(float t, float b, float c, float d);
    static float quartOut(float t, float b, float c, float d);
    static float quartInOut(float t, float b, float c, float d);

    static float quintIn(float t, float b, float c, float d);
    static float quintOut(float t, float b, float c, float d);
    static float quintInOut(float t, float b, float c, float d);

    static float expoIn(float t, float b, float c, float d);
    static float expoOut(float t, float b, float c, float d);
    static float expoInOut(float t, float b, float c, float d);

    static float circIn(float t, float b , float c, float d);
    static float circOut(float t, float b, float c, float d);
    static float circInOut(float t, float b, float c, float d);

    static float backIn(float t, float b, float c, float d);
    static float backOut(float t, float b, float c, float d);
    static float backInOut(float t, float b, float c, float d);

    static float elasticIn(float t, float b, float c, float d);
    static float elasticOut(float t, float b, float c, float d);
    static float elasticInOut(float t, float b, float c, float d);

    static float bounceIn(float t, float b, float c, float d);
    static float bounceOut(float t, float b, float c, float d);
    static float bounceInOut(float t, float b, float c, float d);

private:
    Easing() {}
};

} // namespace util

#endif // UTIL_EASING

