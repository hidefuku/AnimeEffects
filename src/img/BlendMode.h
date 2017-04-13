#ifndef IMG_BLENDMODE_H
#define IMG_BLENDMODE_H

#include <QString>

namespace img
{

enum BlendMode
{
    BlendMode_Normal,

    BlendMode_Darken,
    BlendMode_Multiply,
    BlendMode_ColorBurn,
    BlendMode_LinearBurn,

    BlendMode_Lighten,
    BlendMode_Screen,
    BlendMode_ColorDodge,
    BlendMode_LinearDodge, // Equal to Add

    BlendMode_Overlay,
    BlendMode_SoftLight,
    BlendMode_HardLight,
    BlendMode_VividLight,
    BlendMode_LinearLight,
    BlendMode_PinLight,
    BlendMode_HardMix,

    BlendMode_Difference,
    BlendMode_Exclusion,
    BlendMode_Subtract,
    BlendMode_Divide,

    BlendMode_TERM
};

BlendMode getBlendModeFromPSD(const std::string& aPSDMode);

QString getBlendFuncNameFromBlendMode(BlendMode aMode);

QString getBlendNameFromBlendMode(BlendMode aMode);

QString getQuadIdFromBlendMode(BlendMode aMode);

BlendMode getBlendModeFromQuadId(const QString& aName);

} // namespace img

#endif // IMG_BLENDMODE_H

