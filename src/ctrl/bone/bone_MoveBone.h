#ifndef CTRL_BONE_MOVEBONE
#define CTRL_BONE_MOVEBONE

#include "cmnd/Stable.h"
#include "core/Bone2.h"

namespace ctrl {
namespace bone {

class MoveBone : public cmnd::Stable
{
    core::Bone2* mTarget;
    QVector2D mPrev;
    QVector2D mNext;
    bool mDone;
    bool mFixChildren;

    void readChildrenPos(QVector<QVector2D>& aStack)
    {
        for (auto rootChild : mTarget->children())
        {
            core::Bone2::ConstIterator itr(rootChild);
            while (itr.hasNext())
            {
                aStack.push_back(itr.next()->worldPos());
            }
        }
    }

    void writeChildrenPos(QVector<QVector2D>& aStack)
    {
        for (auto rootChild : mTarget->children())
        {
            core::Bone2::Iterator itr(rootChild);
            while (itr.hasNext())
            {
                auto child = itr.next();
                child->setWorldPos(aStack.first(), child->parent());
                aStack.pop_front();
                child->updateWorldTransform();
            }
        }
    }

public:
    MoveBone(core::Bone2* aTarget, const QVector2D& aNext, bool aFixChildren)
        : mTarget(aTarget)
        , mPrev()
        , mNext(aNext)
        , mDone(false)
        , mFixChildren(aFixChildren)
    {
    }

    void modifyValue(const QVector2D& aNext)
    {
        mNext = aNext;
        if (mDone) redo();
    }

    virtual void exec()
    {
        mPrev = mTarget->worldPos();
        redo();
    }

    virtual void undo()
    {
        QVector<QVector2D> childrenPos;
        if (mFixChildren)
        {
            readChildrenPos(childrenPos);
        }

        mTarget->setWorldPos(mPrev, mTarget->parent());
        mTarget->updateWorldTransform();

        if (mFixChildren)
        {
            writeChildrenPos(childrenPos);
        }

        mDone = false;
    }

    virtual void redo()
    {
        QVector<QVector2D> childrenPos;
        if (mFixChildren)
        {
            readChildrenPos(childrenPos);
        }

        mTarget->setWorldPos(mNext, mTarget->parent());
        mTarget->updateWorldTransform();

        if (mFixChildren)
        {
            writeChildrenPos(childrenPos);
        }

        mDone = true;
    }
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_MOVEBONE

