#ifndef CORE_TIMEKEYEXPANS_H
#define CORE_TIMEKEYEXPANS_H

#include "gl/Texture.h"
#include "core/SRTExpans.h"
#include "core/OpaKey.h"
#include "core/BoneKey.h"
#include "core/PoseKey.h"
#include "core/MeshKey.h"
#include "core/FFDKey.h"
#include "core/ImageKey.h"
#include "core/TimeKeyType.h"
#include "core/PosePalette.h"
#include "core/BoneExpans.h"

namespace core
{

class TimeKeyExpans
{
public:
    TimeKeyExpans();

    // This value is valid when all of my children and I have caches.
    void setMasterCache(Frame aFrame);
    bool hasMasterCache(Frame aFrame) const;
    void clearMasterCache();

    void setKeyCache(TimeKeyType aType, Frame aFrame);
    bool hasKeyCache(TimeKeyType aType, Frame aFrame) const;

    void clearCaches();

    SRTExpans& srt() { return mSRT; }
    const SRTExpans& srt() const { return mSRT; }

    OpaKey::Data& opa() { return mOpa; }
    const OpaKey::Data& opa() const { return mOpa; }

    void setWorldOpacity(float aValue) { mWorldOpacity = aValue; }
    float worldOpacity() const { return mWorldOpacity; }

    BoneExpans& bone() { return mBone; }
    const BoneExpans& bone() const { return mBone; }

    PoseKey::Data& pose() { return mPose; }
    const PoseKey::Data& pose() const { return mPose; }
    void setPoseParent(BoneKey* aKey) { mPoseParent = aKey; }
    BoneKey* poseParent() { return mPoseParent; }
    const BoneKey* poseParent() const { return mPoseParent; }
    PosePalette& posePalette() { return mPosePalette; }
    const PosePalette& posePalette() const { return mPosePalette; }

    void setAreaMeshKey(MeshKey* aKey) { mAreaMeshKey = aKey; }
    MeshKey* areaMeshKey() { return mAreaMeshKey; }
    const MeshKey* areaMeshKey() const { return mAreaMeshKey; }

    FFDKey::Data& ffd() { return mFFD; }
    const FFDKey::Data& ffd() const { return mFFD; }
    void setFFDMesh(LayerMesh* aMesh) { mFFDMesh = aMesh; }
    LayerMesh* ffdMesh() { return mFFDMesh; }
    void setFFDMeshParent(TimeKey* aKey) { mFFDMeshParent = aKey; }
    TimeKey* ffdMeshParent() { return mFFDMeshParent; }

    void setAreaImageKey(ImageKey* aKey) { mAreaImageKey = aKey; }
    ImageKey* areaImageKey() { return mAreaImageKey; }
    const ImageKey* areaImageKey() const { return mAreaImageKey; }
    const gl::Texture* areaTexture() const;
    img::BlendMode blendMode() const;
    void setImageOffset(const QVector2D& aOffset) { mImageOffset = aOffset; }
    QVector2D imageOffset() const { return mImageOffset; }

private:
    Frame mMasterCache;
    std::array<Frame, TimeKeyType_TERM> mKeyCaches;
    SRTExpans mSRT;
    OpaKey::Data mOpa;
    float mWorldOpacity;
    BoneExpans mBone;
    PoseKey::Data mPose;
    BoneKey* mPoseParent;
    PosePalette mPosePalette;
    MeshKey* mAreaMeshKey;
    FFDKey::Data mFFD;
    LayerMesh* mFFDMesh;
    TimeKey* mFFDMeshParent;
    ImageKey* mAreaImageKey;
    QVector2D mImageOffset;
};

} // namespace core

#endif // CORE_TIMEKEYEXPANS_H
