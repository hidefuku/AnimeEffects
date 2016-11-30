#ifndef CTRL_SRT_CENTROIDMOVER_H
#define CTRL_SRT_CENTROIDMOVER_H

#include "cmnd/Stable.h"
#include "core/MoveKey.h"
#include "core/ImageKey.h"
#include "core/ObjectNode.h"
#include "core/Project.h"

namespace ctrl {
namespace srt {

class CentroidMover : public cmnd::Stable
{
    struct KeyData { core::MoveKey* ptr; QVector2D prev; QVector2D next; };
    struct ChildKeyData { core::MoveKey* ptr; QVector2D prev; QVector2D next; };
    struct ImageKeyData { core::ImageKey* ptr; QVector2D prev; QVector2D next; };

    QMatrix4x4 getLocalSRMatrix(const core::TimeKey& aKey);

    core::Project& mProject;
    core::ObjectNode& mTarget;
    QVector2D mPrev;
    QVector2D mNext;
    QVector<KeyData> mKeys;
    QVector<ChildKeyData> mChildKeys;
    QVector<ImageKeyData> mImageKeys;
    bool mDone;
    bool mExecuteOnce;

public:
    static void pushEventTargets(core::ObjectNode& aTarget, core::TimeLineEvent& aEvent);

    CentroidMover(core::Project& aProject,
                  core::ObjectNode& aTarget,
                  const QVector2D& aPrev,
                  const QVector2D& aNext);

    void modifyValue(const QVector2D& aNext);
    virtual void exec();
    virtual void undo();
    virtual void redo();
};

} // namespace srt
} // namespace ctrl

#endif // CTRL_SRT_CENTROIDMOVER_H
