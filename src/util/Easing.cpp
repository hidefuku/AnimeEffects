#include <QtMath>
#include "util/Easing.h"
#include "util/EasingName.h"

namespace util
{

//-------------------------------------------------------------------------------------------------
bool Easing::Param::isValidParam() const
{
    if (type < 0 || Type_TERM <= type) return false;
    if (range < 0 || Range_TERM <= range) return false;
    if (weight < 0.0f || 1.0f < weight) return false;
    return true;
}

bool Easing::Param::operator==(const Param& aRhs) const
{
    return type == aRhs.type && range == aRhs.range && weight == aRhs.weight;
}

//-------------------------------------------------------------------------------------------------
QString Easing::getTypeName(Type aType)
{
    switch (aType)
    {
    case Type_None:
        return EasingName::tr("None");
    case Type_Linear:
        return EasingName::tr("Linear");
    case Type_Sine:
        return EasingName::tr("Sine");
    case Type_Quad:
        return EasingName::tr("Quad");
    case Type_Cubic:
        return EasingName::tr("Cubic");
    case Type_Quart:
        return EasingName::tr("Quart");
    case Type_Quint:
        return EasingName::tr("Quint");
    case Type_Expo:
        return EasingName::tr("Expo");
    case Type_Circ:
        return EasingName::tr("Circ");
    case Type_Back:
        return EasingName::tr("Back");
    case Type_Elastic:
        return EasingName::tr("Elastic");
    case Type_Bounce:
        return EasingName::tr("Bounce");
    default:
        return "";
    }
}

//-------------------------------------------------------------------------------------------------
QStringList Easing::getTypeNameList()
{
    QStringList list;
    for (int i = 0; i < Type_TERM; ++i)
    {
        list.append(getTypeName((Type)i));
    }
    return list;
}

//-------------------------------------------------------------------------------------------------
float Easing::calculate(Type aType, Range aRange, float t, float b, float c, float d)
{
#define RETURN_BY_EASING_FUNCTION(func) switch (aRange) {   \
        case Range_In: return func##In(t, b, c, d);         \
        case Range_Out: return func##Out(t, b, c, d);       \
        case Range_InOut: return func##InOut(t, b, c, d);   \
        default: return func##InOut(t, b, c, d); }

    switch (aType)
    {
    case Type_None:
        return b;
    case Type_Linear:
        return c * (t / d) + b;
    case Type_Sine:
        RETURN_BY_EASING_FUNCTION(sine);
    case Type_Quad:
        RETURN_BY_EASING_FUNCTION(quad);
    case Type_Cubic:
        RETURN_BY_EASING_FUNCTION(cubic);
    case Type_Quart:
        RETURN_BY_EASING_FUNCTION(quart);
    case Type_Quint:
        RETURN_BY_EASING_FUNCTION(quint);
    case Type_Expo:
        RETURN_BY_EASING_FUNCTION(expo);
    case Type_Circ:
        RETURN_BY_EASING_FUNCTION(circ);
    case Type_Back:
        RETURN_BY_EASING_FUNCTION(back);
    case Type_Elastic:
        RETURN_BY_EASING_FUNCTION(elastic);
    case Type_Bounce:
        RETURN_BY_EASING_FUNCTION(bounce);
    default:
        return b;
    }
#undef RETURN_BY_EASING_FUNCTION
}

float Easing::calculate(Param aParam, float t, float b, float c, float d)
{
    const float result = calculate(aParam.type, aParam.range, t, b, c, d);

    if (aParam.type > Type_Linear)
    {
        return result * aParam.weight + (c * (t / d) + b) * (1.0f - aParam.weight);
    }
    return result;
}

//-------------------------------------------------------------------------------------------------
float Easing::sineIn(float t, float b, float c, float d)
{
    return -c * qCos(t / d * (M_PI / 2)) + c + b;
}

float Easing::sineOut(float t, float b, float c, float d)
{
    return c * qSin(t / d * (M_PI / 2)) + b;
}

float Easing::sineInOut(float t, float b, float c, float d)
{
    return -c / 2 * (qCos(M_PI * t / d) - 1) + b;
}

//-------------------------------------------------------------------------------------------------
float Easing::quadIn(float t, float b, float c, float d)
{
    t /= d;
    return c * t * t + b;
}

float Easing::quadOut(float t, float b, float c, float d)
{
    t /= d;
    return -c * t * (t - 2) + b;
}

float Easing::quadInOut(float t, float b, float c, float d)
{
    t /= d / 2;
    if (t < 1)
    {
        return ((c / 2) * (t * t)) + b;
    }
    else
    {
        --t;
        return -c / 2 * ((t - 2) * t - 1) + b;
    }
}

//-------------------------------------------------------------------------------------------------
float Easing::cubicIn(float t, float b, float c, float d)
{
    t /= d;
    return c * t * t * t + b;
}

float Easing::cubicOut(float t, float b, float c, float d)
{
    t = t / d - 1;
    return c * (t * t * t + 1) + b;
}

float Easing::cubicInOut(float t, float b, float c, float d)
{
    t /= d / 2;
    if (t < 1)
    {
        return c / 2 * t * t * t + b;
    }
    else
    {
        t -= 2;
        return c / 2 * (t * t * t + 2) + b;
    }
}

//-------------------------------------------------------------------------------------------------
float Easing::quartIn(float t, float b, float c, float d)
{
    t /= d;
    return c * t * t * t * t + b;
}

float Easing::quartOut(float t, float b, float c, float d)
{
    t = t / d - 1;
    return -c * (t * t * t * t - 1) + b;
}

float Easing::quartInOut(float t, float b, float c, float d)
{
    t /= d / 2;
    if (t < 1)
    {
        return c / 2 * t * t * t * t + b;
    }
    else
    {
        t -= 2;
        return -c / 2 * (t * t * t * t - 2) + b;
    }
}

//-------------------------------------------------------------------------------------------------
float Easing::quintIn(float t, float b, float c, float d)
{
    t /= d;
    return c * t * t * t * t * t + b;
}

float Easing::quintOut(float t, float b, float c, float d)
{
    t = t / d - 1;
    return c * (t * t * t * t * t + 1) + b;
}

float Easing::quintInOut(float t, float b, float c, float d)
{
    t /= d/2;
    if (t < 1)
    {
        return c / 2 * t * t * t * t * t + b;
    }
    else
    {
        t -= 2;
        return c / 2 * (t * t * t * t * t + 2) + b;
    }
}

//-------------------------------------------------------------------------------------------------
float Easing::expoIn(float t, float b, float c, float d)
{
    return (t == 0) ? b : c * qPow(2, 10 * (t / d - 1)) + b;
}

float Easing::expoOut(float t, float b, float c, float d)
{
    return (t == d) ? b + c : c * (-qPow(2, -10 * t / d) + 1) + b;
}

float Easing::expoInOut(float t, float b, float c, float d)
{
    if (t == 0) return b;
    if (t == d) return b + c;

    if ((t /= d / 2) < 1)
    {
        return c / 2 * qPow(2, 10 * (t - 1)) + b;
    }
    else
    {
        return c / 2 * (-qPow(2, -10 * --t) + 2) + b;
    }
}

//-------------------------------------------------------------------------------------------------
float Easing::circIn(float t, float b , float c, float d)
{
    t /= d;
    return -c * (qSqrt(1 - t * t) - 1) + b;
}

float Easing::circOut(float t, float b, float c, float d)
{
    t = t / d - 1;
    return c * qSqrt(1 - t * t) + b;
}

float Easing::circInOut(float t, float b, float c, float d)
{
    t /= d / 2;
    if (t < 1)
    {
        return -c / 2 * (qSqrt(1 - t * t) - 1) + b;
    }
    else
    {
        t -= 2;
        return c / 2 * (qSqrt(1 - t * t) + 1) + b;
    }
}

//-------------------------------------------------------------------------------------------------
float Easing::backIn(float t, float b, float c, float d)
{
    float s = 1.70158f;
    t /= d;
    return c * t * t * ((s + 1) * t - s) + b;
}

float Easing::backOut(float t, float b, float c, float d)
{
    float s = 1.70158f;
    t = t / d - 1;
    return c * (t * t * ((s + 1) * t + s) + 1) + b;
}

float Easing::backInOut(float t, float b, float c, float d)
{
    float s = 1.70158f;
    t /= d / 2;
    if (t < 1)
    {
        s *= 1.525f;
        return c / 2 * (t * t * ((s + 1) * t - s)) + b;
    }
    else
    {
        t -= 2;
        s *= 1.525f;
        return c / 2 * (t * t * ((s + 1) * t + s) + 2) + b;
    }
}

//-------------------------------------------------------------------------------------------------
float Easing::elasticIn(float t, float b, float c, float d)
{
    if (t == 0) return b;
    if ((t /= d) == 1) return b + c;

    float p = d * 0.3f;
    float a = c;
    float s = p / 4;
    float postFix = a * qPow(2, 10 * (t -= 1));
    return -(postFix * qSin((t * d - s) * (2 * M_PI) / p)) + b;
}

float Easing::elasticOut(float t, float b, float c, float d)
{
    if (t == 0) return b;
    if ((t /= d) == 1) return b + c;

    float p = d * 0.3f;
    float a = c;
    float s = p / 4;
    return (a * qPow(2, -10 * t) * qSin((t * d - s) * (2 * M_PI) / p) + c + b);
}

float Easing::elasticInOut(float t, float b, float c, float d)
{
    if (t == 0) return b;
    if ((t /= d / 2) == 2) return b + c;

    float p = d * (0.3f * 1.5f);
    float a = c;
    float s = p / 4;

    if (t < 1)
    {
        float postFix = a * qPow(2, 10 * (t -= 1));
        return -0.5f * (postFix * qSin((t * d - s) * (2 * M_PI) / p)) + b;
    }
    else
    {
        float postFix = a * qPow(2, -10 * (t -= 1));
        return postFix * qSin((t * d - s) * (2 * M_PI) / p) * 0.5f + c + b;
    }
}

//-------------------------------------------------------------------------------------------------
float Easing::bounceIn(float t, float b, float c, float d)
{
    return c - bounceOut(d - t, 0, c, d) + b;
}

float Easing::bounceOut(float t, float b, float c, float d)
{
    if ((t /= d) < (1.0f / 2.75f))
    {
        return c * (7.5625f * t * t) + b;
    }
    else if (t < (2.0f / 2.75f))
    {
        float postFix = t -= (1.5f / 2.75f);
        return c * (7.5625f * (postFix) * t + 0.75f) + b;
    }
    else if (t < (2.5f / 2.75f))
    {
        float postFix = t -= (2.25f / 2.75f);
        return c * (7.5625f * (postFix) * t + 0.9375f) + b;
    }
    else
    {
        float postFix = t -= (2.625f / 2.75f);
        return c * (7.5625f * (postFix) * t + 0.984375f) + b;
    }
}

float Easing::bounceInOut(float t, float b, float c, float d)
{
    if (t < d / 2)
    {
        return bounceIn(t * 2, 0, c, d) * 0.5f + b;
    }
    else
    {
        return bounceOut(t * 2 - d, 0, c, d) * 0.5f + c * 0.5f + b;
    }
}

} // namespace util
