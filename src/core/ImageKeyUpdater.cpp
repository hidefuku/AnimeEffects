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
    bool mCreateTransitions;

    void tryPushTarget(ImageKey* aKey)
    {
        if (aKey)
        {
            auto node = mEvent.findTarget(aKey->data().resource()->serialAddress());
            if (node)
            {
                mTargets.push_back(Target(aKey));
                mTargets.back().prevImage = aKey->data().resource();
                mTargets.back().nextImage = node->handle();
            }
        }
    }

public:
    ImageResourceUpdater(TimeLine& aTimeLine, const ResourceEvent& aEvent,
                         const ResourceUpdatingWorkspacePtr& aWorkspace, bool aCreateTransitions)
        : mTimeLine(aTimeLine)
        , mEvent(aEvent)
        , mTargets()
        , mWorkspace(aWorkspace)
        , mCreateTransitions(aCreateTransitions)
    {
    }

    virtual void exec()
    {
        // push default key
        tryPushTarget((ImageKey*)mTimeLine.defaultKey(TimeKeyType_Image));

        auto& map = mTimeLine.map(TimeKeyType_Image);
        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            TimeKey* key = itr.value();
            TIMEKEY_PTR_TYPE_ASSERT(key, Image);

            // push key
            tryPushTarget((ImageKey*)key);
        }

        for (auto& target : mTargets)
        {
            auto key = target.key;
            GridMesh::TransitionCreater transer(
                        key->data().gridMesh(),
                        key->data().resource()->pos());

            // update image
            target.key->setImage(target.nextImage);

            // create transition data
            if (mCreateTransitions)
            {
                auto& trans = mWorkspace->makeSureTransitions(key, key->data().gridMesh());
                trans = transer.create(
                            key->data().gridMesh().positions(),
                            key->data().gridMesh().vertexCount(),
                            key->data().resource()->pos());
            }
        }
        mWorkspace.reset(); // finish using
    }

    virtual void redo()
    {
        for (auto& target : mTargets)
        {
            target.key->setImage(target.nextImage);
        }
    }

    virtual void undo()
    {
        for (auto& target : mTargets)
        {
            target.key->setImage(target.prevImage);
        }
    }
};

cmnd::Stable* ImageKeyUpdater::createResourceUpdater(
        ObjectNode& aNode, const ResourceEvent& aEvent,
        const ResourceUpdatingWorkspacePtr& aWorkspace, bool aCreateTransitions)
{
    if (!aNode.timeLine()) return nullptr;
    return new ImageResourceUpdater(*aNode.timeLine(), aEvent, aWorkspace, aCreateTransitions);
}

} // namespace core
