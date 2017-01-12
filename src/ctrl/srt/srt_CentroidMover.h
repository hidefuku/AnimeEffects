#ifndef CTRL_SRT_CENTROIDMOVER_H
#define CTRL_SRT_CENTROIDMOVER_H

#include <functional>
#include "cmnd/Stable.h"
#include "core/ObjectNode.h"
#include "core/Project.h"
namespace core { class MoveKey; }
namespace core { class ImageKey; }
namespace core { class MeshKey; }

namespace ctrl {
namespace srt {

#if 0
class CentroidMover : public cmnd::Stable
{
    struct KeyData { core::MoveKey* ptr; QVector2D prev; QVector2D next; };
    struct ChildKeyData { core::MoveKey* ptr; QVector2D prev; QVector2D next; };
    struct ImageKeyData { core::ImageKey* ptr; QVector2D prev; QVector2D next; };
    struct MeshKeyData { core::MeshKey* ptr; QVector2D prev; QVector2D next; };

    QMatrix4x4 getLocalSRMatrix(const core::TimeKey& aKey);
    void addAllTargets(
            core::ObjectNode& aTarget,
            core::TimeKeyType aType,
            bool aContainDefault,
            const std::function<void(core::TimeKey*)>& aPusher);

    core::Project& mProject;
    core::ObjectNode& mTarget;
    QVector2D mPrev;
    QVector2D mNext;
    bool mAdjustsPostures;
    QVector<KeyData> mKeys;
    QVector<ChildKeyData> mChildKeys;
    QVector<ImageKeyData> mImageKeys;
    QVector<MeshKeyData> mMeshKeys;
    bool mDone;
    bool mExecuteOnce;

public:
    static void pushEventTargets(core::ObjectNode& aTarget,
                                 core::TimeLineEvent& aEvent,
                                 bool aAdjustPostures);

    CentroidMover(core::Project& aProject,
                  core::ObjectNode& aTarget,
                  const QVector2D& aPrev,
                  const QVector2D& aNext,
                  bool aAdjustPostures);

    bool adjustsPostures() const { return mAdjustsPostures; }

    void modifyValue(const QVector2D& aNext);
    virtual void exec();
    virtual void undo();
    virtual void redo();
};
#else
class CentroidMover : public cmnd::Stable
{
    core::Project& mProject;
    core::ObjectNode& mTarget;
    core::MoveKey* mKey;
    QVector2D mCentroidMove;
    QVector2D mPositionMove;
    QVector2D mPrevPosition;
    QVector2D mPrevCentroid;
    int mFrame;
    bool mAdjustPos;
    bool mDone;

public:
    CentroidMover(core::Project& aProject,
                  core::ObjectNode& aTarget,
                  const QVector2D& aCentroidMove,
                  const QVector2D& aPositionMove,
                  int aFrame, bool aAdjustPos);

    void modifyValue(const QVector2D& aNewCentroidMove,
                     const QVector2D& aNewPositionMove);
    virtual void exec();
    virtual void undo();
    virtual void redo();
};
#endif

} // namespace srt
} // namespace ctrl

#endif // CTRL_SRT_CENTROIDMOVER_H
