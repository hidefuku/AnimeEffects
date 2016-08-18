#include <QQuaternion>
#include "core/PosePalette.h"
#include "util/MathUtil.h"

namespace core
{

PosePalette::PosePalette()
    : mData()
    , mIsUnit(true)
{
    clearDualQuaternions();
}

int PosePalette::makeBoneOrigins(const KeyPairs& aSrc, BonePairs& aDst)
{
    int count = 0;
    for (auto keyPair : aSrc)
    {
        for (auto topBone : keyPair.origin->topBones())
        {
            Bone2::ConstIterator itr(topBone);
            while (itr.hasNext())
            {
                const Bone2* bone = itr.next();
                XC_PTR_ASSERT(bone);
                aDst[count].origin = bone;

                ++count;
                if (count >= kMaxCount) return count;
            }
        }
    }
    return count;
}

int PosePalette::makeBonePoses(const KeyPairs& aSrc, BonePairs& aDst)
{
    int count = 0;
    for (auto keyPair : aSrc)
    {
        for (auto topBone : keyPair.pose->topBones())
        {
            Bone2::ConstIterator itr(topBone);
            while (itr.hasNext())
            {
                const Bone2* bone = itr.next();
                XC_PTR_ASSERT(bone);
                aDst[count].pose = bone;

                ++count;
                if (count >= kMaxCount) return count;
            }
        }
    }
    return count;
}

void PosePalette::build(const KeyPairs& aKeyPairs)
{
    const QVector3D kRotateAxis(0.0f, 0.0f, 1.0f);
    mIsUnit = false;

    BonePairs bonePairs;
    int count = makeBoneOrigins(aKeyPairs, bonePairs);
    int index = makeBonePoses(aKeyPairs, bonePairs);

    XC_MSG_ASSERT(count == index, "%d, %d", count, index); (void)index;

    for (int i = 0; i < count; ++i)
    {
        const float rotate = util::MathUtil::getDegreeFromRadian(
                    bonePairs[i].pose->worldAngle() - bonePairs[i].origin->worldAngle());

        mData[i] = QMatrix4x4();
        mData[i].translate(bonePairs[i].pose->worldPos());
        mData[i].rotate(rotate, kRotateAxis);
        mData[i].translate(-bonePairs[i].origin->worldPos());

        // dual quaternion
        {
            QMatrix4x4 rotMtx;
            rotMtx.rotate(rotate, kRotateAxis);
            const QVector2D originPos =
                    (rotMtx * QVector3D(bonePairs[i].origin->worldPos())).toVector2D();

            const QVector3D trans = bonePairs[i].pose->worldPos() - originPos;
            auto quat = QQuaternion::fromAxisAndAngle(kRotateAxis, rotate);
            mDualQuats[i] = makeDualQuaternion(quat, trans);
        }
    }
    for (int i = count; i < kMaxCount; ++i)
    {
        mData[i] = QMatrix4x4();
    }
}

void PosePalette::clear()
{
    if (mIsUnit) return;
    mIsUnit = true;

    for (int i = 0; i < kMaxCount; ++i)
    {
        mData[i] = QMatrix4x4();
    }

    clearDualQuaternions();
}

void PosePalette::clearDualQuaternions()
{
    for (int i = 0; i < kMaxCount; ++i)
    {
        mDualQuats[i].real.set(1.0f, 0.0f, 0.0f, 0.0f);
        mDualQuats[i].dual.set(0.0f, 0.0f, 0.0f, 0.0f);
    }

}

util::ArrayBlock<const QMatrix4x4> PosePalette::matrices() const
{
    return util::ArrayBlock<const QMatrix4x4>(mData.data(), kMaxCount);
}

util::ArrayBlock<const PosePalette::DualQuaternion> PosePalette::dualQuaternions() const
{
    return util::ArrayBlock<const DualQuaternion>(mDualQuats.data(), kMaxCount);
}

PosePalette::DualQuaternion PosePalette::makeDualQuaternion(
        const QQuaternion& aUnitQuat, const QVector3D& aTrans)
{
    const float tx = aTrans.x(), ty = aTrans.y(), tz = aTrans.z();
    const float qw = aUnitQuat.scalar();
    const float qx = aUnitQuat.x(), qy = aUnitQuat.y(), qz = aUnitQuat.z();

    DualQuaternion dq;
    dq.real.x = qw;
    dq.real.y = qx;
    dq.real.z = qy;
    dq.real.w = qz;

    dq.dual.x = 0.5 * (-tx * qx - ty * qy - tz * qz);
    dq.dual.y = 0.5 * ( tx * qw + ty * qz - tz * qy);
    dq.dual.z = 0.5 * (-tx * qz + ty * qw + tz * qx);
    dq.dual.w = 0.5 * ( tx * qy - ty * qx + tz * qw);

    return dq;
}

} // namespace core
