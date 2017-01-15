#ifndef CORE_BONEEXPANS_H
#define CORE_BONEEXPANS_H

#include "core/BoneKey.h"

namespace core
{

class BoneExpans
{
public:
    BoneExpans();

    void setAreaKey(BoneKey* aKey) { mAreaKey = aKey; }
    BoneKey* areaKey() { return mAreaKey; }
    const BoneKey* areaKey() const { return mAreaKey; }

    void setInfluenceMap(const BoneInfluenceMap* aMap) { mInfluenceMap = aMap; }
    const BoneInfluenceMap* influenceMap() const { return mInfluenceMap; }

    void setOuterMatrix(const QMatrix4x4& aMtx) { mOuterMtx = aMtx; }
    void setInnerMatrix(const QMatrix4x4& aMtx) { mInnerMtx = aMtx; }
    const QMatrix4x4& outerMatrix() const { return mOuterMtx; }
    const QMatrix4x4& innerMatrix() const { return mInnerMtx; }
    QMatrix4x4 worldCSRTMatrix() const { return mOuterMtx * mInnerMtx; }

    void setTargetMesh(const LayerMesh* aMesh) { mTargetMesh = aMesh; }
    const LayerMesh* targetMesh() const { return mTargetMesh; }

    void setBindingRoot(ObjectNode* aRoot) { mBindingRoot = aRoot; }
    ObjectNode* bindingRoot() const { return mBindingRoot; }

    void setBinderIndex(int aIndex) { mBinderIndex = aIndex; }
    int binderIndex() const { return mBinderIndex; }
    bool isBoundByBone() const { return mBinderIndex >= 0; }

    void setBindingMatrix(const QMatrix4x4& aMtx) { mBindingMtx = aMtx; }
    const QMatrix4x4& bindingMatrix() const { return mBindingMtx; }

    void setIsUnderOfBinding(bool aFlag) { mIsUnderOfBinding = aFlag; }
    bool isUnderOfBinding() const { return mIsUnderOfBinding; }

    void setIsAffectedByBinding(bool aFlag) { mIsAffectedByBinding = aFlag; }
    bool isAffectedByBinding() const { return mIsAffectedByBinding; }

private:
    BoneKey* mAreaKey;
    const BoneInfluenceMap* mInfluenceMap;
    QMatrix4x4 mOuterMtx;
    QMatrix4x4 mInnerMtx;
    const LayerMesh* mTargetMesh;
    ObjectNode* mBindingRoot;
    int mBinderIndex;
    QMatrix4x4 mBindingMtx;
    bool mIsUnderOfBinding;
    bool mIsAffectedByBinding;
};

} // namespace core

#endif // CORE_BONEEXPANS_H
