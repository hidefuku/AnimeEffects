#include "util/MathUtil.h"
#include "core/SRTKey.h"
#include "core/Constant.h"

namespace core
{
const SRTKey::SplineType SRTKey::kDefaultSplineType = SRTKey::SplineType_CatmullRom;

//-------------------------------------------------------------------------------------------------
SRTKey::Data::Data()
    : easing()
    , spline(kDefaultSplineType)
    , pos()
    , rotate(0.0f)
    , scale(1.0f, 1.0f)
{
}

QMatrix4x4 SRTKey::Data::localMatrix() const
{
    QMatrix4x4 mtx;
    mtx.translate(pos);
    mtx.rotate(util::MathUtil::getDegreeFromRadian(rotate), QVector3D(0.0f, 0.0f, 1.0f));
    mtx.scale(QVector3D(scale, 1.0f));
    return mtx;
}

QMatrix4x4 SRTKey::Data::localSRMatrix() const
{
    QMatrix4x4 mtx;
    mtx.rotate(util::MathUtil::getDegreeFromRadian(rotate), QVector3D(0.0f, 0.0f, 1.0f));
    mtx.scale(QVector3D(scale, 1.0f));
    return mtx;
}

void SRTKey::Data::clamp()
{
    clampPos();
    clampRotate();
    clampScale();
}

void SRTKey::Data::clampPos()
{
    pos.setX(xc_clamp(pos.x(), Constant::transMin(), Constant::transMax()));
    pos.setY(xc_clamp(pos.y(), Constant::transMin(), Constant::transMax()));
    pos.setZ(xc_clamp(pos.z(), Constant::transMin(), Constant::transMax()));
}

void SRTKey::Data::clampRotate()
{
    rotate = xc_clamp(rotate, Constant::rotateMin(), Constant::rotateMax());
}

void SRTKey::Data::clampScale()
{
    scale.setX(xc_clamp(scale.x(), Constant::scaleMin(), Constant::scaleMax()));
    scale.setY(xc_clamp(scale.y(), Constant::scaleMin(), Constant::scaleMax()));
}

//-------------------------------------------------------------------------------------------------
SRTKey::SRTKey()
    : mData()
{
}

bool SRTKey::serialize(Serializer& aOut) const
{
    aOut.write((int)mData.easing.type);
    aOut.write((int)mData.easing.range);
    aOut.write(mData.easing.weight);
    aOut.write((int)mData.spline);
    aOut.write(mData.pos);
    aOut.write(mData.rotate);
    aOut.write(mData.scale);
    return aOut.checkStream();
}

bool SRTKey::deserialize(Deserializer &aIn)
{
    aIn.pushLogScope("SRTKey");

    aIn.read((int&)mData.easing.type);
    aIn.read((int&)mData.easing.range);
    aIn.read(mData.easing.weight);
    if (!mData.easing.isValidParam())
    {
        return aIn.errored("invalid easing param");
    }

    aIn.read((int&)mData.spline);
    aIn.read(mData.pos);
    aIn.read(mData.rotate);
    aIn.read(mData.scale);
    mData.clamp();

    aIn.popLogScope();
    return aIn.checkStream();
}

}
