#include "core/TimeKeyExpans.h"
#include "ctrl/BoneEditor.h"
#include "ctrl/bone/bone_CreateMode.h"
#include "ctrl/bone/bone_DeleteMode.h"
#include "ctrl/bone/bone_MoveJointMode.h"
#include "ctrl/bone/bone_BindNodesMode.h"
#include "ctrl/bone/bone_InfluenceMode.h"
#include "ctrl/bone/bone_PaintInflMode.h"
#include "ctrl/bone/bone_EraseInflMode.h"
#include "ctrl/bone/bone_GeoBuilder.h"

using namespace core;

namespace ctrl
{

BoneEditor::BoneEditor(Project& aProject)
    : mProject(aProject)
    , mParam()
    , mCurrent()
    , mTarget()
    , mKeyOwner()
{
    /// @todo
    /// jugdge a targetability by a binding tree
    /// merge a holder timeline
    /// support timeline modifying to a holder key
    /// brush up a rendering userbility
}

BoneEditor::~BoneEditor()
{
    finalize();
}

void BoneEditor::setTarget(core::ObjectNode* aTarget)
{
    finalize();

    if (!aTarget || !aTarget->timeLine()) return;

    // ignore a node which was already bound by bone
    /// @todo this condition should update by resetCurrentTarget
    {
        auto parent = aTarget->parent();
        while (parent)
        {
            if (parent->timeLine() && parent->timeLine()->current().areaBone())
            {
                return;
            }
            parent = parent->parent();
        }

        ObjectNode::Iterator itr(aTarget);
        itr.next();
        while (itr.hasNext())
        {
            auto child = itr.next();
            if (child->timeLine() && child->timeLine()->current().areaBone())
            {
                return;
            }
        }
    }

    mTarget.node = aTarget;
    resetCurrentTarget();
}

void BoneEditor::updateParam(const BoneParam& aParam)
{
    const BoneParam prev = mParam;
    mParam = aParam;

    if (prev.mode != mParam.mode)
    {
        resetCurrentTarget();
    }

    if (mCurrent)
    {
        mCurrent->updateParam(mParam);
    }
}

bool BoneEditor::updateCursor(const core::CameraInfo& aCamera, const core::AbstractCursor& aCursor)
{
    if (mCurrent)
    {
        return mCurrent->updateCursor(aCamera, aCursor);
    }
    return false;
}

void BoneEditor::updateEvent(EventType)
{
    resetCurrentTarget();
}

void BoneEditor::renderQt(const core::RenderInfo& aInfo, QPainter& aPainter)
{
#if 0
    if (mKeyOwner.key)
    {
        bone::GeoBuilder builder(mKeyOwner.key->data().topBones());


        const QColor color[4] = {
            QColor(255, 0, 0, 255),
            QColor(0, 255, 0, 255),
            QColor(0, 0, 255, 255),
            QColor(0, 0,   0, 255) };
        int colorIndex = 0;

        for (auto& infl : builder.influence())
        {
            QPolygonF drawInfl = infl;
            for (int i = 0; i < drawInfl.count(); ++i)
            {
                drawInfl[i] = aInfo.camera.toScreenPos(drawInfl[i]);
            }

            const QBrush brush(color[colorIndex]);
            aPainter.setBrush(brush);
            aPainter.setPen(QPen(brush, 2, Qt::SolidLine));

            //if (colorIndex == 2) qDebug() << drawInfl;
            aPainter.drawPolyline(drawInfl);
            colorIndex = (colorIndex + 1) % 4;
        }
    }
#endif

    if (mCurrent)
    {
        return mCurrent->renderQt(aInfo, aPainter);
    }
}

void BoneEditor::finalize()
{
    mCurrent.reset();
    mKeyOwner.deleteOwnsKey();
    mTarget.clear();
}

void BoneEditor::resetCurrentTarget()
{
    mCurrent.reset();
    mKeyOwner.deleteOwnsKey();

    // read srt matrix
    if (mTarget)
    {
        XC_PTR_ASSERT(mTarget->timeLine());

        bool success = false;
        mTarget.mtx = mTarget->timeLine()->current().srt().worldMatrix();
        mTarget.invMtx = mTarget.mtx.inverted(&success);
        if (!success)
        {
            mTarget.node = nullptr;
        }
    }

    if (mTarget)
    {
        initializeKey(*mTarget->timeLine());
        createMode();
    }
}

void BoneEditor::createMode()
{
    mCurrent.reset();

    if (!mTarget || !mKeyOwner.key) return;

    switch (mParam.mode)
    {
    case BoneEditMode_Create:
        mCurrent.reset(new bone::CreateMode(mProject, mTarget, mKeyOwner));
        break;

    case BoneEditMode_Delete:
        mCurrent.reset(new bone::DeleteMode(mProject, mTarget, mKeyOwner));
        break;

    case BoneEditMode_MoveJoint:
        mCurrent.reset(new bone::MoveJointMode(mProject, mTarget, mKeyOwner));
        break;

    case BoneEditMode_BindNodes:
        mCurrent.reset(new bone::BindNodesMode(mProject, mTarget, mKeyOwner));
        break;

    case BoneEditMode_Influence:
        mCurrent.reset(new bone::InfluenceMode(mProject, mTarget, mKeyOwner));
        break;

    case BoneEditMode_PaintInfl:
        mCurrent.reset(new bone::PaintInflMode(mProject, mTarget, mKeyOwner));
        break;

    case BoneEditMode_EraseInfl:
        mCurrent.reset(new bone::EraseInflMode(mProject, mTarget, mKeyOwner));
        break;

    default:
        break;
    }
    if (mCurrent)
    {
        mCurrent->updateParam(mParam);
    }
}

void BoneEditor::initializeKey(TimeLine& aLine)
{
    const TimeLine::MapType& map = aLine.map(TimeKeyType_Bone);
    const int frame = mProject.animator().currentFrame().get();

    if (map.contains(frame))
    {
        // a key is exists
        mKeyOwner.key = static_cast<BoneKey*>(map.value(frame));
        XC_PTR_ASSERT(mKeyOwner.key);
        mKeyOwner.ownsKey = false;
    }
    else
    {
        // create new key
        mKeyOwner.key = new BoneKey();
        mKeyOwner.ownsKey = true;
    }

}

} // namespace ctrl
