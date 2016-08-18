#include <QFileInfo>
#include <QUndoCommand>
#include "XC.h"
#include "core/Project.h"

namespace
{
static const int kParaThreadCount = 4;
static const int kStandardFps = 60;
static const int kDefaultMaxFrame = 60 * 10;
}

namespace core
{
//-------------------------------------------------------------------------------------------------
Project::Attribute::Attribute()
    : mImageSize()
    , mMaxFrame(kDefaultMaxFrame)
    , mFps(kStandardFps)
    , mLoop(false)
{
}

//-------------------------------------------------------------------------------------------------
Project::Project(QString aFileName, Animator& aAnimator, Hook* aHookGrabbed)
    : mLifeLink()
    , mFileName(aFileName)
    , mAttribute()
    , mParalleler(new thr::Paralleler(kParaThreadCount))
    , mResourceHolder()
    , mCommandStack()
    , mObjectTree()
    , mAnimator(aAnimator)
    , mHook(aHookGrabbed)
{
    if (!aFileName.isEmpty())
    {
        setFileName(aFileName);
    }

    onTimeLineModified.connect([=](TimeLineEvent& aEvent, bool)
    {
        aEvent.setProject(*this);
    });

    onTimeLineModified.connect(&mObjectTree, &ObjectTree::onTimeLineModified);
    onTreeRestructured.connect(&mObjectTree, &ObjectTree::onTreeRestructured);
    onResourceModified.connect(&mObjectTree, &ObjectTree::onResourceModified);
    onProjectAttributeModified.connect(&mObjectTree, &ObjectTree::onProjectAttributeModified);

#ifdef UNUSE_PARALLEL
#else
    mParalleler->start();
#endif
}

Project::~Project()
{
    // clear all command firstly
    mCommandStack.clear();
}

void Project::setFileName(const QString& aFileName)
{
    mFileName = aFileName;
    mResourceHolder.setRootPath(QFileInfo(aFileName).path());
}

TimeInfo Project::currentTimeInfo() const
{
    TimeInfo time;
    time.frameMax = mAttribute.maxFrame();
    time.fps = mAttribute.fps();
    time.loop = mAttribute.loop();
    time.frame = mAnimator.currentFrame();
    return time;
}

} // namespace core
