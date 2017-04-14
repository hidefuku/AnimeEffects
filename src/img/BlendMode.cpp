#include "img/BlendMode.h"
#include "img/BlendModeName.h"

namespace img
{

BlendMode getBlendModeFromPSD(const std::string& aMode)
{
    if      (aMode == "norm") { return BlendMode_Normal; }
    else if (aMode == "dark") { return BlendMode_Darken; }
    else if (aMode == "mul ") { return BlendMode_Multiply; }
    else if (aMode == "idiv") { return BlendMode_ColorBurn; }
    else if (aMode == "lbrn") { return BlendMode_LinearBurn; }
    else if (aMode == "lite") { return BlendMode_Lighten; }
    else if (aMode == "scrn") { return BlendMode_Screen; }
    else if (aMode == "div ") { return BlendMode_ColorDodge; }
    else if (aMode == "lddg") { return BlendMode_LinearDodge; }
    else if (aMode == "over") { return BlendMode_Overlay; }
    else if (aMode == "sLit") { return BlendMode_SoftLight; }
    else if (aMode == "hLit") { return BlendMode_HardLight; }
    else if (aMode == "vLit") { return BlendMode_VividLight; }
    else if (aMode == "lLit") { return BlendMode_LinearLight; }
    else if (aMode == "pLit") { return BlendMode_PinLight; }
    else if (aMode == "hMix") { return BlendMode_HardMix; }
    else if (aMode == "diff") { return BlendMode_Difference; }
    else if (aMode == "smud") { return BlendMode_Exclusion; }
    else if (aMode == "fsub") { return BlendMode_Subtract; }
    else if (aMode == "fdiv") { return BlendMode_Divide; }

    return BlendMode_TERM;
}

QString getBlendFuncNameFromBlendMode(BlendMode aMode)
{
    switch (aMode)
    {
    case BlendMode_Normal:      return "Normal";
    case BlendMode_Darken:      return "Darken";
    case BlendMode_Multiply:    return "Multiply";
    case BlendMode_ColorBurn:   return "ColorBurn";
    case BlendMode_LinearBurn:  return "LinearBurn";
    case BlendMode_Lighten:     return "Lighten";
    case BlendMode_Screen:      return "Screen";
    case BlendMode_ColorDodge:  return "ColorDodge";
    case BlendMode_LinearDodge: return "LinearDodge";
    case BlendMode_Overlay:     return "Overlay";
    case BlendMode_SoftLight:   return "SoftLight";
    case BlendMode_HardLight:   return "HardLight";
    case BlendMode_VividLight:  return "VividLight";
    case BlendMode_LinearLight: return "LinearLight";
    case BlendMode_PinLight:    return "PinLight";
    case BlendMode_HardMix:     return "HardMix";
    case BlendMode_Difference:  return "Difference";
    case BlendMode_Exclusion:   return "Exclusion";
    case BlendMode_Subtract:    return "Subtract";
    case BlendMode_Divide:      return "Divide";
    default:                    return "Normal";
    }
}

QString getBlendNameFromBlendMode(BlendMode aMode)
{
    switch (aMode)
    {
    case BlendMode_Normal:      return BlendModeName::tr("Normal");
    case BlendMode_Darken:      return BlendModeName::tr("Darken");
    case BlendMode_Multiply:    return BlendModeName::tr("Multiply");
    case BlendMode_ColorBurn:   return BlendModeName::tr("ColorBurn");
    case BlendMode_LinearBurn:  return BlendModeName::tr("LinearBurn");
    case BlendMode_Lighten:     return BlendModeName::tr("Lighten");
    case BlendMode_Screen:      return BlendModeName::tr("Screen");
    case BlendMode_ColorDodge:  return BlendModeName::tr("ColorDodge");
    case BlendMode_LinearDodge: return BlendModeName::tr("LinearDodge");
    case BlendMode_Overlay:     return BlendModeName::tr("Overlay");
    case BlendMode_SoftLight:   return BlendModeName::tr("SoftLight");
    case BlendMode_HardLight:   return BlendModeName::tr("HardLight");
    case BlendMode_VividLight:  return BlendModeName::tr("VividLight");
    case BlendMode_LinearLight: return BlendModeName::tr("LinearLight");
    case BlendMode_PinLight:    return BlendModeName::tr("PinLight");
    case BlendMode_HardMix:     return BlendModeName::tr("HardMix");
    case BlendMode_Difference:  return BlendModeName::tr("Difference");
    case BlendMode_Exclusion:   return BlendModeName::tr("Exclusion");
    case BlendMode_Subtract:    return BlendModeName::tr("Subtract");
    case BlendMode_Divide:      return BlendModeName::tr("Divide");
    default:                    return BlendModeName::tr("Normal");
    }
}

QString getQuadIdFromBlendMode(BlendMode aMode)
{
    switch (aMode)
    {
    case BlendMode_Normal:      return "norm";
    case BlendMode_Darken:      return "dark";
    case BlendMode_Multiply:    return "mul ";
    case BlendMode_ColorBurn:   return "idiv";
    case BlendMode_LinearBurn:  return "lbrn";
    case BlendMode_Lighten:     return "lite";
    case BlendMode_Screen:      return "scrn";
    case BlendMode_ColorDodge:  return "div ";
    case BlendMode_LinearDodge: return "lddg";
    case BlendMode_Overlay:     return "over";
    case BlendMode_SoftLight:   return "sLit";
    case BlendMode_HardLight:   return "hLit";
    case BlendMode_VividLight:  return "vLit";
    case BlendMode_LinearLight: return "lLit";
    case BlendMode_PinLight:    return "pLit";
    case BlendMode_HardMix:     return "hMix";
    case BlendMode_Difference:  return "diff";
    case BlendMode_Exclusion:   return "smud";
    case BlendMode_Subtract:    return "fsub";
    case BlendMode_Divide:      return "fdiv";
    default:                    return "    ";
    }
}

BlendMode getBlendModeFromQuadId(const QString& aName)
{
    if      (aName == "norm") { return BlendMode_Normal; }
    else if (aName == "dark") { return BlendMode_Darken; }
    else if (aName == "mul ") { return BlendMode_Multiply; }
    else if (aName == "idiv") { return BlendMode_ColorBurn; }
    else if (aName == "lbrn") { return BlendMode_LinearBurn; }
    else if (aName == "lite") { return BlendMode_Lighten; }
    else if (aName == "scrn") { return BlendMode_Screen; }
    else if (aName == "div ") { return BlendMode_ColorDodge; }
    else if (aName == "lddg") { return BlendMode_LinearDodge; }
    else if (aName == "over") { return BlendMode_Overlay; }
    else if (aName == "sLit") { return BlendMode_SoftLight; }
    else if (aName == "hLit") { return BlendMode_HardLight; }
    else if (aName == "vLit") { return BlendMode_VividLight; }
    else if (aName == "lLit") { return BlendMode_LinearLight; }
    else if (aName == "pLit") { return BlendMode_PinLight; }
    else if (aName == "hMix") { return BlendMode_HardMix; }
    else if (aName == "diff") { return BlendMode_Difference; }
    else if (aName == "smud") { return BlendMode_Exclusion; }
    else if (aName == "fsub") { return BlendMode_Subtract; }
    else if (aName == "fdiv") { return BlendMode_Divide; }

    return BlendMode_TERM;
}

} // namespace img
