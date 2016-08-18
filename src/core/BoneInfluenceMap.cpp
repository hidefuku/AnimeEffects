#include <float.h>
#include <QtMath>
#include "XC.h"
#include "core/Constant.h"
#include "core/Project.h"
#include "core/LayerMesh.h"
#include "core/BoneInfluenceMap.h"
//#include <QElapsedTimer>

namespace core
{

//-------------------------------------------------------------------------------------------------
BoneInfluenceMap::BoneInfluenceMap()
    : mVertexCount(0)
    , mMaxBoneCount(0)
    , mGroupMtx()
    , mBoneList()
    , mWorks()
    , mBuildTask()
{
}

void BoneInfluenceMap::setMaxBoneCount(int aBoneCount)
{
    XC_ASSERT(mBuildTask.isNull());
    mMaxBoneCount = aBoneCount;
}

void BoneInfluenceMap::allocate(int aVertexCount, bool aInitialize)
{
    // cancel previous task
    if (mBuildTask)
    {
        mBuildTask->cancel();
    }

    // reallocate
    if (mVertexCount != aVertexCount)
    {
        mWorks.reset();

        for (int t = 0; t < 2; ++t)
        {
            mIndices[t].reset();
            mWeights[t].reset();
        }

        if (aVertexCount > 0)
        {
            const int dataCount = kBonePerVtxMaxEach * aVertexCount;
            for (int t = 0; t < 2; ++t)
            {
                mIndices[t].reset(new IndicesType[dataCount]);
                XC_PTR_ASSERT(mIndices[t]);
                if (!mIndices[t]) return;

                mWeights[t].reset(new WeightsType[dataCount]);
                XC_PTR_ASSERT(mWeights[t]);
                if (!mWeights[t]) return;
            }
        }
    }

    // initialize
    if (aInitialize)
    {
        for (int t = 0; t < 2; ++t)
        {
            IndicesType* indices = mIndices[t].data();
            WeightsType* weights = mWeights[t].data();

            const int count = aVertexCount * kBonePerVtxMaxEach;
            for (int i = 0; i < count; ++i) { indices[i] = 0; }

            for (int i = 0; i < aVertexCount; ++i)
            {
                const int ixc = kBonePerVtxMaxEach * i;
                weights[ixc] = 1.0f;

                for (int k = 1; k < kBonePerVtxMaxEach; ++k)
                {
                    weights[ixc + k] = 0.0f;
                }
            }
        }
    }

    mVertexCount = aVertexCount;
}

void BoneInfluenceMap::writeAsync(
        Project& aProject, const QList<Bone2*>& aTopBones,
        const QMatrix4x4& aGroupMtx, const LayerMesh& aMesh)
{
    XC_ASSERT(mMaxBoneCount > 0);

    // cancel previous task
    if (mBuildTask)
    {
        mBuildTask->cancel();
    }

    // set group matrix
    mGroupMtx = aGroupMtx;

    // allocate work buffer
    mWorks.reset(new WorkAttribute[mVertexCount]);
    XC_PTR_ASSERT(mWorks.data());
    if (!mWorks) return;

    // initialize work buffer
    const gl::Vector3* positions = aMesh.positions();
    for (int i = 0; i < mVertexCount; ++i)
    {
        mWorks[i].clear();
        mWorks[i].vertex = positions[i].pos2D();
    }

    // make bone list
    makeBoneList(aTopBones);

#ifdef UNUSE_PARALLEL
    build();
    (void)aProject;
#else
    // create task
    mBuildTask.reset(new BuildTask(aProject, *this));
    aProject.paralleler().push(*mBuildTask);
#endif
}

BoneInfluenceMap::Accessor BoneInfluenceMap::accessor() const
{
    waitBuilding();
    return Accessor(*this);
}

void BoneInfluenceMap::makeBoneList(const QList<Bone2*>& aTopBones)
{
    mBoneList.params.clear();

    // per skeleton
    for (const Bone2* topBone : aTopBones)
    {
        XC_PTR_ASSERT(topBone);

        // per bone
        for (Bone2::ConstIterator itr = topBone; itr.hasNext();)
        {
            const Bone2* bone = itr.next();
            XC_PTR_ASSERT(bone);

            if (bone && mBoneList.params.size() < mMaxBoneCount)
            {
                BoneParam param;
                param.hasParent = (bool)(bone->parent());

                if (param.hasParent)
                {
                    param.hasRange = bone->hasValidRange();
                    param.shape = bone->shape();
                }
                mBoneList.params.push_back(param);
            }

        }
    }
}

void BoneInfluenceMap::build()
{
    // world matrix * vertices
    transformVertices();
    if (isBuildCanceled()) return;

    // write bone weight
    writeWeights();
    if (isBuildCanceled()) return;

    // write vertex attribute
    writeVertexAttribute();
    if (isBuildCanceled()) return;

    // free work buffer
    mWorks.reset();
}

void BoneInfluenceMap::transformVertices()
{
    for (int i = 0; i < mVertexCount; ++i)
    {
        mWorks[i].vertex = (mGroupMtx * QVector3D(mWorks[i].vertex)).toVector2D();
    }
}

void BoneInfluenceMap::writeWeights()
{
    // each bone
    for (int i = 0; i < mBoneList.params.size(); ++i)
    {
        const BoneParam param = mBoneList.params[i];
        if (!param.hasParent || !param.hasRange) continue;

        // calculate weights
        for (int k = 0; k < mVertexCount; ++k)
        {
            const float weight = param.shape.influence(mWorks[k].vertex);

            if (weight >= FLT_EPSILON)
            {
                mWorks[k].tryPushBoneWeight(i, weight);
            }
        }

        // check canceling
        if (isBuildCanceled()) return;
    }
}

void BoneInfluenceMap::writeVertexAttribute()
{
    static const float kMinPowerSum = 0.01f;

    // clear
    for (int t = 0; t < 2; ++t)
    {
        IndicesType* indices = mIndices[t].data();
        WeightsType* weights = mWeights[t].data();
        const int count = mVertexCount * kBonePerVtxMaxEach;

        for (int i = 0; i < count; ++i) { indices[i] = 0; }
        for (int i = 0; i < count; ++i) { weights[i] = 0.0f; }
    }

    // each vertex
    for (int i = 0; i < mVertexCount; ++i)
    {
        const int ixc = i * kBonePerVtxMaxEach;
        const WorkAttribute& work = mWorks[i];

        if (work.count == 0)
        {
            // no bone influence
            mIndices[0][ixc] = 0;
            mWeights[0][ixc] = 1.0f;
        }
        else if (work.count == 1)
        {
            // single bone influence
            mIndices[0][ixc] = work.id[0];
            mWeights[0][ixc] = 1.0f;
        }
        else
        {
            // some bones influence

            float powerSum = 0.0f;
            for (int k = 0; k < work.count; ++k)
            {
                powerSum += work.weight[k];
            }

            float weightRate = 1.0f;
            float weightAdd = 0.0f;

            if (powerSum >= kMinPowerSum)
            {
                weightRate /= powerSum;
            }
            else
            {
                weightAdd = (1.0f - powerSum) / work.count;
            }

            for (int k = 0; k < work.count; ++k)
            {
                const int t = k / kBonePerVtxMaxEach;
                const int each = ixc + k - t * kBonePerVtxMaxEach;

                mIndices[t][each] = work.id[k];
                mWeights[t][each] = work.weight[k] * weightRate + weightAdd;
            }
        }
    }
}

bool BoneInfluenceMap::isBuildCanceled() const
{
    if (mBuildTask)
    {
        return mBuildTask->isCanceling();
    }
    return false;
}

void BoneInfluenceMap::waitBuilding() const
{
#ifndef UNUSE_PARALLEL
    // wait current task
    if (mBuildTask)
    {
        mBuildTask->wait();
    }
#endif
}

bool BoneInfluenceMap::serialize(Serializer& aOut) const
{
    waitBuilding();

    // vertex count
    aOut.write(mVertexCount);
    // max bone count
    aOut.write(mMaxBoneCount);

    const int count = kBonePerVtxMaxEach * mVertexCount;
    // indices
    aOut.writeGL(mIndices[0].data(), count);
    aOut.writeGL(mIndices[1].data(), count);
    // weights
    aOut.writeGL(mWeights[0].data(), count);
    aOut.writeGL(mWeights[1].data(), count);

    return aOut.checkStream();
}

bool BoneInfluenceMap::deserialize(Deserializer& aIn)
{
    waitBuilding();

    // vertex count
    {
        int vertexCount = 0;
        aIn.read(vertexCount);

        // reallocate
        allocate(vertexCount, false);
    }
    // max bone count
    aIn.read(mMaxBoneCount);

    const int count = kBonePerVtxMaxEach * mVertexCount;
    // indices
    aIn.readGL(mIndices[0].data(), count);
    aIn.readGL(mIndices[1].data(), count);
    // weights
    aIn.readGL(mWeights[0].data(), count);
    aIn.readGL(mWeights[1].data(), count);

    return aIn.checkStream();
}

//-------------------------------------------------------------------------------------------------
BoneInfluenceMap::BoneParam::BoneParam()
    : hasParent(false)
    , hasRange(false)
    , shape()
{
}

//-------------------------------------------------------------------------------------------------
void BoneInfluenceMap::WorkAttribute::clear()
{
    count = 0;
}

void BoneInfluenceMap::WorkAttribute::tryPushBoneWeight(int aId, float aWeight)
{
    if (count < kBonePerVtxMaxAll)
    {
        id[count] = aId;
        weight[count] = aWeight;
        ++count;
    }
    else
    {
        int minId = -1;
        float minWeight = aWeight;
        for (int i = 0; i < kBonePerVtxMaxAll; ++i)
        {
            if (weight[i] < minWeight)
            {
                minId = i;
                minWeight = weight[i];
            }
        }

        if (minId != -1)
        {
            id[minId] = aId;
            weight[minId] = aWeight;
        }
    }
}

//-------------------------------------------------------------------------------------------------
BoneInfluenceMap::BuildTask::BuildTask(Project& aProject, BoneInfluenceMap& aOwner)
    : mProject(aProject)
    , mOwner(aOwner)
{
}

void BoneInfluenceMap::BuildTask::run()
{
    mOwner.build();
}

void BoneInfluenceMap::BuildTask::cancel()
{
    mProject.paralleler().cancel(*this);
}

//-------------------------------------------------------------------------------------------------
BoneInfluenceMap::Accessor::Accessor()
    : mOwner()
{
}

BoneInfluenceMap::Accessor::Accessor(const BoneInfluenceMap& aOwner)
    : mOwner(&aOwner)
{
}

const gl::Vector4I* BoneInfluenceMap::Accessor::indices0() const
{
    XC_PTR_ASSERT(mOwner);
    return (const gl::Vector4I*)mOwner->mIndices[0].data();
}

const gl::Vector4I* BoneInfluenceMap::Accessor::indices1() const
{
    XC_PTR_ASSERT(mOwner);
    return (const gl::Vector4I*)mOwner->mIndices[1].data();
}

const gl::Vector4* BoneInfluenceMap::Accessor::weights0() const
{
    XC_PTR_ASSERT(mOwner);
    return (const gl::Vector4*)mOwner->mWeights[0].data();
}

const gl::Vector4* BoneInfluenceMap::Accessor::weights1() const
{
    XC_PTR_ASSERT(mOwner);
    return (const gl::Vector4*)mOwner->mWeights[1].data();
}

} // namespace core
