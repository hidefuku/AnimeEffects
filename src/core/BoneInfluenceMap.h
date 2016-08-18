#ifndef CORE_BONEINFLUENCEMAP_H
#define CORE_BONEINFLUENCEMAP_H

#include <QGL>
#include <QVector>
#include <QVector2D>
#include <QRectF>
#include <QMatrix4x4>
#include "util/Segment2D.h"
#include "util/NonCopyable.h"
#include "thr/Task.h"
#include "gl/Vector3.h"
#include "gl/Vector4.h"
#include "gl/Vector4I.h"
#include "core/Bone2.h"
namespace core { class Project; }
namespace core { class LayerMesh; }

namespace core
{

class BoneInfluenceMap : private util::NonCopyable
{
public:
    typedef GLint   IndicesType;
    typedef GLfloat WeightsType;

    class Accessor
    {
    public:
        Accessor();
        Accessor(const BoneInfluenceMap& aOwner);
        const gl::Vector4I* indices0() const;
        const gl::Vector4I* indices1() const;
        const gl::Vector4* weights0() const;
        const gl::Vector4* weights1() const;
    private:
        const BoneInfluenceMap* mOwner;
    };

    enum
    {
        kBonePerVtxMaxEach = 4,
        kBonePerVtxMaxAll  = 8
    };

    BoneInfluenceMap();

    void setMaxBoneCount(int aBoneCount);
    void allocate(int aVertexCount, bool aInitialize = true);
    int vertexCount() const { return mVertexCount; }

    void writeAsync(
            Project& aProject, const QList<Bone2*>& aTopBones,
            const QMatrix4x4& aGroupMtx, const LayerMesh& aMesh);

    Accessor accessor() const;

    bool serialize(Serializer& aOut) const;
    bool deserialize(Deserializer& aIn);

private:
    class BoneParam
    {
    public:
        BoneParam();
        bool hasParent;
        bool hasRange;
        BoneShape shape;
    };

    class BoneList
    {
    public:
        BoneList() {}
        QVector<BoneParam> params;
    };

    struct WorkAttribute
    {
        int id[kBonePerVtxMaxAll];
        float weight[kBonePerVtxMaxAll];
        short count;
        QVector2D vertex;
        void clear();
        void tryPushBoneWeight(int aId, float aWeight);
    };

    class BuildTask : public thr::Task
    {
    public:
        BuildTask(Project& aProject, BoneInfluenceMap& aOwner);
        virtual void run();
        void cancel();
    private:
        Project& mProject;
        BoneInfluenceMap& mOwner;
    };

    void makeBoneList(const QList<Bone2*>& aTopBones);
    void build();
    void transformVertices();
    void writeWeights();
    void writeVertexAttribute();
    bool isBuildCanceled() const;
    void waitBuilding() const;

    int mVertexCount;
    int mMaxBoneCount;
    QMatrix4x4 mGroupMtx;
    BoneList mBoneList;
    QScopedArrayPointer<IndicesType> mIndices[2];
    QScopedArrayPointer<WeightsType> mWeights[2];
    QScopedArrayPointer<WorkAttribute> mWorks;
    QScopedPointer<BuildTask> mBuildTask;
};

} // namespace core

#endif // CORE_BONEINFLUENCEMAP_H
