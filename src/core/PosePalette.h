#ifndef CORE_POSEPALETTE_H
#define CORE_POSEPALETTE_H

#include <array>
#include <QMatrix4x4>
#include <QVector>
#include "XC.h"
#include "util/ArrayBlock.h"
#include "gl/Vector4.h"
#include "core/BoneKey.h"
#include "core/PoseKey.h"

namespace core
{

class PosePalette
{
public:
    enum { kMaxCount = 32 };

    struct KeyPair
    {
        const BoneKey::Data* origin;
        const PoseKey::Data* pose;
    };
    typedef QVector<KeyPair> KeyPairs;

    struct DualQuaternion
    {
        gl::Vector4 real;
        gl::Vector4 dual;
        const GLfloat* data() const { return (const GLfloat*)(this); }
    };

    static int getBoneIndex(const BoneKey::Data& aData, const Bone2& aBone);

    PosePalette();

    void build(const KeyPairs& aKeyPairs);
    void clear();

    util::ArrayBlock<const QMatrix4x4> matrices() const;
    util::ArrayBlock<const DualQuaternion> dualQuaternions() const;

private:
    struct BonePair { const Bone2* origin; const Bone2* pose; };
    typedef std::array<BonePair, kMaxCount> BonePairs;
    int makeBoneOrigins(const KeyPairs& aSrc, BonePairs& aDst);
    int makeBonePoses(const KeyPairs& aSrc, BonePairs& aDst);
    void clearDualQuaternions();
    static DualQuaternion makeDualQuaternion(
            const QQuaternion& aUnitQuat, const QVector3D& aTrans);

    std::array<QMatrix4x4, kMaxCount> mData;
    bool mIsUnit;
    std::array<DualQuaternion, kMaxCount> mDualQuats;
};

} // namespace core

#endif // CORE_POSEPALETTE_H

