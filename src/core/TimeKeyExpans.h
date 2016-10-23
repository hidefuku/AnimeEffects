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

    void setAreaBoneKey(BoneKey* aKey) { mAreaBoneKey = aKey; }
    BoneKey* areaBoneKey() { return mAreaBoneKey; }
    const BoneKey* areaBoneKey() const { return mAreaBoneKey; }
    void setBoneInfluence(const BoneInfluenceMap* aMap) { mBoneInfluence = aMap; }
    const BoneInfluenceMap* boneInfluence() const { return mBoneInfluence; }
    void setOuterMatrix(const QMatrix4x4& aMtx) { mOuterMtx = aMtx; }
    const QMatrix4x4& outerMatrix() const { return mOuterMtx; }
    void setInnerMatrix(const QMatrix4x4& aMtx) { mInnerMtx = aMtx; }
    const QMatrix4x4& innerMatrix() const { return mInnerMtx; }
    void setBoneParent(const LayerMesh* aMesh) { mBoneParent = aMesh; }
    const LayerMesh* boneParent() const { return mBoneParent; }

    void setBindingRoot(ObjectNode* aRoot) { mBindingRoot = aRoot; }
    ObjectNode* bindingRoot() const { return mBindingRoot; }
    void setBinderBoneIndex(int aIndex) { mBinderBoneIndex = aIndex; }
    int binderBoneIndex() const { return mBinderBoneIndex; }
    void setBindingMatrix(const QMatrix4x4& aMtx) { mBindingMtx = aMtx; }
    const QMatrix4x4& bindingMatrix() const { return mBindingMtx; }
    bool isBoundByBone() const { return mBinderBoneIndex >= 0; }
    void setIsUnderOfBinding(bool aFlag) { mIsUnderOfBinding = aFlag; }
    bool isUnderOfBinding() const { return mIsUnderOfBinding; }
    void setIsAffectedByBinding(bool aFlag) { mIsAffectedByBinding = aFlag; }
    bool isAffectedByBinding() const { return mIsAffectedByBinding; }

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
    void setAreaTexture(gl::Texture* aTexture) { mAreaTexture = aTexture; }
    gl::Texture* areaTexture() { return mAreaTexture; }
    void setImageOffset(const QVector2D& aOffset) { mImageOffset = aOffset; }
    const QVector2D& imageOffset() const { return mImageOffset; }

private:
    Frame mMasterCache;
    std::array<Frame, TimeKeyType_TERM> mKeyCaches;
    SRTExpans mSRT;
    OpaKey::Data mOpa;
    float mWorldOpacity;
    BoneKey* mAreaBoneKey;
    const BoneInfluenceMap* mBoneInfluence;
    QMatrix4x4 mOuterMtx;
    QMatrix4x4 mInnerMtx;
    const LayerMesh* mBoneParent;
    ObjectNode* mBindingRoot;
    int mBinderBoneIndex;
    QMatrix4x4 mBindingMtx;
    bool mIsUnderOfBinding;
    bool mIsAffectedByBinding;
    PoseKey::Data mPose;
    BoneKey* mPoseParent;
    PosePalette mPosePalette;
    MeshKey* mAreaMeshKey;
    FFDKey::Data mFFD;
    LayerMesh* mFFDMesh;
    TimeKey* mFFDMeshParent;
    ImageKey* mAreaImageKey;
    gl::Texture* mAreaTexture;
    QVector2D mImageOffset;
};

} // namespace core

#endif // CORE_TIMEKEYEXPANS_H
