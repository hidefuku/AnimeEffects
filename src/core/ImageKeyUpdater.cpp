#include "core/ImageKeyUpdater.h"
#include "core/ImageKey.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
class ImageResourceUpdater : public cmnd::Stable
{
    struct Target
    {
        Target(ImageKey* aKey)
            : key(aKey)
            , prevImage()
            , nextImage()
        {}
        ImageKey* key;
        img::ResourceHandle prevImage;
        img::ResourceHandle nextImage;
    };

    TimeLine& mTimeLine;
    const ResourceEvent& mEvent;
    QList<Target> mTargets;
    ResourceUpdatingWorkspacePtr mWorkspace;

public:
    ImageResourceUpdater(TimeLine& aTimeLine, const ResourceEvent& aEvent,
                         const ResourceUpdatingWorkspacePtr& aWorkspace)
        : mTimeLine(aTimeLine)
        , mEvent(aEvent)
        , mTargets()
        , mWorkspace(aWorkspace)
    {
    }

    virtual void exec()
    {
        auto& map = mTimeLine.map(TimeKeyType_Image);
        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            TimeKey* key = itr.value();
            XC_PTR_ASSERT(key);
            XC_ASSERT(key->type() == TimeKeyType_Image);
            ImageKey* imgKey = (ImageKey*)key;

            // reset cache
            auto node = mEvent.findTarget(imgKey->data().resource()->serialAddress());
            if (node)
            {
                mTargets.push_back(Target(imgKey));
                mTargets.back().prevImage = imgKey->data().resource();
                mTargets.back().nextImage = node->handle();
            }
        }

        for (auto& target : mTargets)
        {
            auto key = target.key;
            GridMesh::TransitionCreater transer(
                        key->cache().gridMesh(),
                        key->data().resource()->pos());

            // update image
            target.key->data().resource() = target.nextImage;
            target.key->resetCache();

            // create transition data
            auto& trans = mWorkspace->makeSureTransitions(key, key->cache().gridMesh());
            trans = transer.create(
                        key->cache().gridMesh().positions(),
                        key->cache().gridMesh().vertexCount(),
                        key->data().resource()->pos());
        }
        mWorkspace.reset(); // finish using
    }

    virtual void redo()
    {
        for (auto& target : mTargets)
        {
            target.key->data().resource() = target.nextImage;
            target.key->resetCache();
        }
    }

    virtual void undo()
    {
        for (auto& target : mTargets)
        {
            target.key->data().resource() = target.prevImage;
            target.key->resetCache();
        }
    }
};

cmnd::Stable* ImageKeyUpdater::createResourceUpdater(
        ObjectNode& aNode, const ResourceEvent& aEvent,
        const ResourceUpdatingWorkspacePtr& aWorkspace)
{
    if (!aNode.timeLine()) return nullptr;
    return new ImageResourceUpdater(*aNode.timeLine(), aEvent, aWorkspace);
}

} // namespace core
