#ifndef CTRL_BONE_ASSIGNINFLUENCE
#define CTRL_BONE_ASSIGNINFLUENCE

#include "cmnd/Stable.h"
#include "core/Bone2.h"

namespace ctrl {
namespace bone {

class AssignInfluence : public cmnd::Stable
{
    core::Bone2* mTarget;
    QVector2D mPrev;
    QVector2D mNext;
    int mIndex;
    bool mDone;

public:
    AssignInfluence(core::Bone2* aTarget, int aIndex, const QVector2D& aNext)
        : mTarget(aTarget), mPrev(), mNext(aNext), mIndex(aIndex), mDone(false) {}

    int index() const { return mIndex; }
    void modifyValue(const QVector2D& aNext) { mNext = aNext; if (mDone) redo(); }
    virtual void exec() { mPrev = mTarget->range(mIndex); redo(); }
    virtual void undo() { mTarget->setRange(mIndex, mPrev); mDone = false; }
    virtual void redo() { mTarget->setRange(mIndex, mNext); mDone = true; }
};

class FuzzyAssignInfluence : public cmnd::Stable
{
    struct Unit
    {
        void assign()
        {
            for (int i = 0; i < 2; ++i)
                target->setRange(i, next[i]);
        }

        void unassign()
        {
            for (int i = 0; i < 2; ++i)
                target->setRange(i, prev[i]);
        }

        core::Bone2* target;
        std::array<QVector2D, 2> prev;
        std::array<QVector2D, 2> next;
    };

    QVector<Unit> mUnits;
    bool mDone;

public:
    FuzzyAssignInfluence()
        : mUnits()
        , mDone(false)
    {}

    void push(core::Bone2& aTarget, const std::array<QVector2D, 2>& aPrev,
              const std::array<QVector2D, 2>& aNext)
    {
        for (auto& unit : mUnits)
        {
            if (&aTarget == unit.target)
            {
                unit.next = aNext;
                if (mDone) unit.assign();
                return;
            }
        }

        Unit newUnit;
        newUnit.target = &aTarget;
        newUnit.prev = aPrev;
        newUnit.next = aPrev;
        mUnits.push_back(newUnit);
        if (mDone) newUnit.assign();
    }

    virtual void undo()
    {
        for (auto unit : mUnits) unit.unassign();
        mDone = false;
    }

    virtual void redo()
    {
        for (auto unit : mUnits) unit.assign();
        mDone = true;
    }
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_ASSIGNINFLUENCE

