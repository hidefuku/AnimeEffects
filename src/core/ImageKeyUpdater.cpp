#include "core/ImageKeyUpdater.h"
#include "core/ImageKey.h"

namespace core
{
#if 0
//-------------------------------------------------------------------------------------------------
class ImageResourceUpdater : public cmnd::Stable
{
    struct Target
    {
        Target(ImageKey* aKey) : key(aKey), pos() {}
        ImageKey* key;
        QVector<gl::Vector3> pos;
        //img::Buffer mOldImage;
    };

    TimeLine& mTimeLine;
    ResourceEvent& mEvent;
    QList<Target> mTargets;

public:
    ImageResourceUpdater(TimeLine& aTimeLine, ResourceEvent& aEvent)
        : mTimeLine(aTimeLine)
        , mEvent(aEvent)
        , mTargets()
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
            if (mEvent.targets().contains(imgKey->data().resource().serialAddress()))
            {
                imgKey->resetCache();
            }

            mTargets.push_back(Target(ffdKey));
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

cmnd::Stable* ImageKeyUpdater::createResourceUpdater(ObjectNode& aNode, ResourceEvent& aEvent)
{
    if (!aNode.timeLine()) return nullptr;
    return new ImageResourceUpdater(*aNode.timeLine(), aEvent);
}
#endif

#if 0
void ImageKeyUpdater::onResourceModified(TimeLine* aLine, ResourceEvent& aEvent)
{
    if (aLine)
    {
        auto& map = aLine->map(TimeKeyType_Image);

        // update bone cache
        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            TimeKey* key = itr.value();
            XC_PTR_ASSERT(key);
            XC_ASSERT(key->type() == TimeKeyType_Image);
            ImageKey* imgKey = (ImageKey*)key;
            // reset cache
            if (aEvent.targets().contains(imgKey->data().resource().serialAddress()))
            {
                imgKey->resetCache();
            }
        }
    }
}
#endif

} // namespace core
