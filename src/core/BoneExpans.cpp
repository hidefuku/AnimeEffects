#include "core/BoneExpans.h"

namespace core
{

BoneExpans::BoneExpans()
    : mAreaKey()
    , mInfluenceMap()
    , mOuterMtx()
    , mInnerMtx()
    , mTargetMesh()
    , mBindingRoot()
    , mBinderIndex(-1)
    , mBindingMtx()
    , mIsUnderOfBinding()
    , mIsAffectedByBinding()
{
}

} // namespace core
