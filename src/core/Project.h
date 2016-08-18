#ifndef CORE_PROJECT_H
#define CORE_PROJECT_H

#include <QSize>
#include <QString>
#include <QScopedPointer>
#include <functional>
#include "util/LifeLink.h"
#include "util/Signaler.h"
#include "util/NonCopyable.h"
#include "thr/Paralleler.h"
#include "cmnd/Stack.h"
#include "core/ObjectTree.h"
#include "core/Animator.h"
#include "core/TimeInfo.h"
#include "core/TimeLineEvent.h"
#include "core/ResourceHolder.h"
#include "core/ResourceEvent.h"
#include "core/ObjectTreeEvent.h"
#include "core/ProjectEvent.h"

namespace core
{

class Project : private util::NonCopyable
{
public:
    class Attribute
    {
    public:
        Attribute();
        void setImageSize(const QSize& aSize) { mImageSize = aSize; }
        const QSize& imageSize() const { return mImageSize; }
        void setMaxFrame(int aFrame) { mMaxFrame = aFrame; }
        int maxFrame() const { return mMaxFrame; }
        void setFps(int aFps) { mFps = aFps; }
        int fps() const { return mFps; }
        void setLoop(bool aLoop) { mLoop = aLoop; }
        bool loop() const { return mLoop; }

    private:
        QSize mImageSize;
        int mMaxFrame;
        int mFps;
        bool mLoop;
    };

    class Hook
    {
    public:
        virtual ~Hook() {}
    };

    Project(QString aFileName, Animator& aAnimator, Hook* aHookGrabbed);
    ~Project();

    util::LifeLink::Pointee<Project> pointee() { return mLifeLink.pointee<Project>(this); }
    util::LifeLink::Pointee<const Project> constPointee() { return mLifeLink.pointee<const Project>(this); }

    void setFileName(const QString& aFileName);
    const QString& fileName() const { return mFileName; }
    bool isNameless() const { return mFileName.isEmpty(); }

    Attribute& attribute() { return mAttribute; }
    const Attribute& attribute() const { return mAttribute; }

    Animator& animator() { return mAnimator; }
    const Animator& animator() const { return mAnimator; }

    TimeInfo currentTimeInfo() const;

    ResourceHolder& resourceHolder() { return mResourceHolder; }
    const ResourceHolder& resourceHolder() const { return mResourceHolder; }

    cmnd::Stack& commandStack() { return mCommandStack; }
    const cmnd::Stack& commandStack() const { return mCommandStack; }

    ObjectTree& objectTree() { return mObjectTree; }
    const ObjectTree& objectTree() const { return mObjectTree; }

    thr::Paralleler& paralleler() { return *mParalleler; }
    const thr::Paralleler& paralleler() const { return *mParalleler; }

    Hook* hook() { return mHook.data(); }

    util::Signaler<void(TimeLineEvent&, bool)> onTimeLineModified;
    util::Signaler<void(ObjectNode&, bool)> onNodeAttributeModified;
    util::Signaler<void(ResourceEvent&, bool)> onResourceModified;
    util::Signaler<void(ObjectTreeEvent&, bool)> onTreeRestructured;
    util::Signaler<void(ProjectEvent&, bool)> onProjectAttributeModified;

private:
    util::LifeLink mLifeLink;
    QString mFileName;
    Attribute mAttribute;

    QScopedPointer<thr::Paralleler> mParalleler;
    ResourceHolder mResourceHolder;
    cmnd::Stack mCommandStack;
    ObjectTree mObjectTree;
    Animator& mAnimator;
    QScopedPointer<Hook> mHook;
};

} // namespace core

#endif // CORE_PROJECT_H
