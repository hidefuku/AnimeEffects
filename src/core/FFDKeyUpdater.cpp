#include "util/ArrayBlock.h"
#include "core/FFDKeyUpdater.h"
#include "core/FFDKey.h"

namespace core
{
//-------------------------------------------------------------------------------------------------
class FFDResourceUpdater : public cmnd::Stable
{
    struct Target
    {
        Target(FFDKey* aKey)
            : key(aKey), pos() {}
        FFDKey* key;
        QVector<gl::Vector3> pos;
    };

    TimeLine& mTimeLine;
    const GridMesh& mNewMesh;
    const GridMesh::Transitions& mTransitions;
    QList<Target> mTargets;

public:
    FFDResourceUpdater(TimeLine& aTimeLine,
               const GridMesh& aNewMesh,
               const GridMesh::Transitions& aTransitions)
        : mTimeLine(aTimeLine)
        , mNewMesh(aNewMesh)
        , mTransitions(aTransitions)
        , mTargets()
    {
    }

    virtual void exec()
    {
        auto& map = mTimeLine.map(TimeKeyType_FFD);
        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            TimeKey* timeKey = itr.value();
            XC_PTR_ASSERT(timeKey);
            XC_ASSERT(timeKey->type() == TimeKeyType_FFD);
            FFDKey* ffdKey = (FFDKey*)timeKey;

            if (!ffdKey->belongsToDefaultParent())
            {
                continue;
            }

            util::ArrayBlock<const gl::Vector3> posArray(
                        ffdKey->data().positions(),
                        ffdKey->data().count());

            auto newFFD = mNewMesh.createFFD(posArray, mTransitions);

            mTargets.push_back(Target(ffdKey));

            auto& target = mTargets.back();
            target.pos.resize(newFFD.count());
            memcpy(target.pos.data(), newFFD.array(),
                   sizeof(gl::Vector3) * newFFD.count());

            delete[] newFFD.array();
        }

        redo();
    }

    virtual void redo()
    {
        for (auto& target : mTargets)
        {
            target.key->data().swap(target.pos);
        }
    }

    virtual void undo()
    {
        for (auto& target : mTargets)
        {
            target.key->data().swap(target.pos);
        }
    }
};


//-------------------------------------------------------------------------------------------------
cmnd::Stable* FFDKeyUpdater::createResourceUpdater(
        ObjectNode& aNode,
        const GridMesh& aNewMesh,
        const GridMesh::Transitions& aTransitions)
{
    if (!aNode.timeLine()) return nullptr;

    return new FFDResourceUpdater(*aNode.timeLine(), aNewMesh, aTransitions);
}

} // namespace core

