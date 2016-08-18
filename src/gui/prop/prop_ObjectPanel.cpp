#include "cmnd/ScopedMacro.h"
#include "cmnd/BasicCommands.h"
#include "core/Constant.h"
#include "core/TimeKeyExpans.h"
#include "ctrl/TimeLineUtil.h"
#include "gui/prop/prop_ObjectPanel.h"
#include "gui/prop/prop_Items.h"

namespace
{
class ObjectNodeAttrNotifier : public cmnd::Listener
{
public:
    ObjectNodeAttrNotifier(core::Project& aProject, core::ObjectNode& aNode)
        : mProject(aProject)
        , mNode(aNode)
    {}
    virtual void onExecuted() { mProject.onNodeAttributeModified(mNode, false); }
    virtual void onUndone() { mProject.onNodeAttributeModified(mNode, true); }
    virtual void onRedone() { mProject.onNodeAttributeModified(mNode, false); }

private:
    core::Project& mProject;
    core::ObjectNode& mNode;
};

const core::SRTKey::Data& getSRTKeyData(const core::ObjectNode& aTarget, int aFrame)
{
    auto key = aTarget.timeLine()->timeKey(core::TimeKeyType_SRT, aFrame);
    XC_PTR_ASSERT(key);
    return ((const core::SRTKey*)key)->data();
}

const core::OpaKey::Data& getOpaKeyData(const core::ObjectNode& aTarget, int aFrame)
{
    auto key = aTarget.timeLine()->timeKey(core::TimeKeyType_Opa, aFrame);
    XC_PTR_ASSERT(key);
    return ((const core::OpaKey*)key)->data();
}

#if 0
const core::PoseKey::Data& getPoseKeyData(const core::ObjectNode& aTarget, int aFrame)
{
    auto key = aTarget.timeLine()->timeKey(core::TimeKeyType_Pose, aFrame);
    XC_PTR_ASSERT(key);
    return ((const core::PoseKey*)key)->data();
}

const core::FFDKey::Data& getFFDKeyData(const core::ObjectNode& aTarget, int aFrame)
{
    auto key = aTarget.timeLine()->timeKey(core::TimeKeyType_FFD, aFrame);
    XC_PTR_ASSERT(key);
    return ((const core::FFDKey*)key)->data();
}
#endif

}

namespace gui {
namespace prop {

//-------------------------------------------------------------------------------------------------
ObjectPanel::ObjectPanel(core::Project& aProject, const QString& aTitle, QWidget* aParent)
    : Panel(aTitle, aParent)
    , mProject(aProject)
    , mTarget()
    , mLabelWidth()
    , mAttributes()
    , mDepth()
    , mBlendMode()
    , mClipped()
    , mSRTKey()
    , mSRTEasing()
    , mSRTSpline()
    , mSRTTrans()
    , mSRTRotate()
    , mSRTScale()
    , mOpaKey()
    , mOpaEasing()
    , mOpacity()
    , mPoseKey()
    , mPoseEasing()
    , mFFDKey()
    , mFFDEasing()
{
    mLabelWidth = this->fontMetrics().boundingRect("MaxTextWidth :").width();

    build();
    this->hide();
}

void ObjectPanel::setTarget(core::ObjectNode* aTarget)
{
    mTarget = aTarget;

    if (mTarget)
    {
        this->setTitle(mTarget->name());
        this->show();
    }
    else
    {
        this->hide();
    }

    updateAttribute();
    updateKey();
}

void ObjectPanel::updateKey()
{
    updateKeyExists();
    updateKeyValue();
}

void ObjectPanel::updateFrame()
{
    updateKeyExists();
    updateKeyValue();
}

void ObjectPanel::build()
{
    using core::Constant;

    mAttributes = new AttrGroup("Constant", mLabelWidth);
    {
        this->addGroup(mAttributes);

        // depth
        mDepth = new DecimalItem(mAttributes);
        mDepth->setRange(Constant::transMin(), Constant::transMax());
        mDepth->onValueUpdated = [=](double, double aNext)
        {
            assignDepth(this->mProject, this->mTarget, aNext);
        };
        mAttributes->addItem("depth :", mDepth);

        // blend mode
        mBlendMode = new ComboItem(mAttributes);
        for (int i = 0; i < img::BlendMode_TERM; ++i)
        {
            auto mode = (img::BlendMode)i;
            mBlendMode->box().addItem(img::getBlendNameFromBlendMode(mode));
        }
        mBlendMode->onValueUpdated = [=](int, int aNext)
        {
            assignBlendMode(this->mProject, this->mTarget, (img::BlendMode)aNext);
        };
        mAttributes->addItem("blend :", mBlendMode);

        // clipped
        mClipped = new CheckItem(mAttributes);
        mClipped->onValueUpdated = [=](bool aNext)
        {
            assignClipped(this->mProject, this->mTarget, aNext);
        };
        mAttributes->addItem("clipped :", mClipped);
    }

    mSRTKey = new KeyGroup("SRT", mLabelWidth);
    {
        mSRTKey->setKeyKnocker([=](bool aCheck)
        {
            if (aCheck)
            {
                knockNewSRT(this->mProject, this->mTarget);
            }
        });
        this->addGroup(mSRTKey);

        // easing
        mSRTEasing = new EasingItem(mSRTKey);
        mSRTKey->addItem("easing :", mSRTEasing);
        mSRTEasing->onValueUpdated = [=](util::Easing::Param, util::Easing::Param aNext)
        {
            assignSRTEasing(this->mProject, this->mTarget, aNext);
        };

        // spline
        mSRTSpline = new ComboItem(mSRTKey);
        mSRTSpline->box().addItems(QStringList() << "Linear" << "CatmullRom");
        mSRTSpline->setValue(core::SRTKey::kDefaultSplineType, false);
        mSRTSpline->onValueUpdated = [=](int, int aNext)
        {
            assignSpline(this->mProject, this->mTarget, aNext);
        };
        mSRTKey->addItem("spline :", mSRTSpline);

        // translate
        mSRTTrans = new Vector2DItem(mSRTKey);
        mSRTTrans->setRange(Constant::transMin(), Constant::transMax());
        mSRTTrans->onValueUpdated = [=](QVector2D, QVector2D aNext)
        {
            assignTrans(this->mProject, this->mTarget, aNext);
        };
        mSRTKey->addItem("translate :", mSRTTrans);

        // rotate
        mSRTRotate = new DecimalItem(mSRTKey);
        mSRTRotate->setRange(Constant::rotateMin(), Constant::rotateMax());
        mSRTRotate->onValueUpdated = [=](double, double aNext)
        {
            assignRotate(this->mProject, this->mTarget, aNext);
        };
        mSRTKey->addItem("rotate :", mSRTRotate);

        // scale
        mSRTScale = new Vector2DItem(mSRTKey);
        mSRTScale->setRange(Constant::scaleMin(), Constant::scaleMax());
        mSRTScale->onValueUpdated = [=](QVector2D, QVector2D aNext)
        {
            assignScale(this->mProject, this->mTarget, aNext);
        };
        mSRTKey->addItem("scale :", mSRTScale);
    }

    mOpaKey = new KeyGroup("Opacity", mLabelWidth);
    {
        mOpaKey->setKeyKnocker([=](bool aCheck)
        {
            if (aCheck)
            {
                knockNewOpacity(this->mProject, this->mTarget);
            }
        });
        this->addGroup(mOpaKey);

        // easing
        mOpaEasing = new EasingItem(mOpaKey);
        mOpaKey->addItem("easing :", mOpaEasing);
        mOpaEasing->onValueUpdated = [=](util::Easing::Param, util::Easing::Param aNext)
        {
            assignOpaEasing(this->mProject, this->mTarget, aNext);
        };

        // opacity
        mOpacity = new DecimalItem(mOpaKey);
        mOpacity->setRange(0.0f, 1.0f);
        mOpacity->box().setSingleStep(0.1);
        mOpacity->onValueUpdated = [=](double, double aNext)
        {
            assignOpacity(this->mProject, this->mTarget, aNext);
        };
        mOpaKey->addItem("opacity :", mOpacity);
    }

    mPoseKey = new KeyGroup("Pose", mLabelWidth);
    {
        mPoseKey->setKeyKnocker([=](bool aCheck)
        {
            if (aCheck)
            {
                knockNewPose(this->mProject, this->mTarget);
            }
        });
        this->addGroup(mPoseKey);

        // easing
        mPoseEasing = new EasingItem(mPoseKey);
        mPoseKey->addItem("easing :", mPoseEasing);
        mPoseEasing->onValueUpdated = [=](util::Easing::Param, util::Easing::Param aNext)
        {
            assignPoseEasing(this->mProject, this->mTarget, aNext);
        };
    }

    mFFDKey = new KeyGroup("FFD", mLabelWidth);
    {
        mFFDKey->setKeyKnocker([=](bool aCheck)
        {
            if (aCheck)
            {
                knockNewFFD(this->mProject, this->mTarget);
            }
        });
        this->addGroup(mFFDKey);

        // easing
        mFFDEasing = new EasingItem(mFFDKey);
        mFFDKey->addItem("easing :", mFFDEasing);
        mFFDEasing->onValueUpdated = [=](util::Easing::Param, util::Easing::Param aNext)
        {
            assignFFDEasing(this->mProject, this->mTarget, aNext);
        };
    }
}

void ObjectPanel::updateAttribute()
{
    if (mTarget)
    {
        mDepth->setValue(mTarget->depth());

        if (mTarget->renderer())
        {
            auto& renderer = *(mTarget->renderer());
            if (renderer.hasBlendMode())
            {
                mBlendMode->setItemEnabled(true);
                mBlendMode->setValue(renderer.blendMode(), false);
            }
            else
            {
                mBlendMode->setItemEnabled(false);
            }

            mClipped->setItemEnabled(true);
            mClipped->setValue(renderer.isClipped(), false);
        }
        else
        {
            mBlendMode->setItemEnabled(false);
            mClipped->setItemEnabled(false);
        }
    }
}

void ObjectPanel::updateKeyExists()
{
    if (mTarget && mTarget->timeLine())
    {
        const core::TimeLine& timeLine = *mTarget->timeLine();
        const int frame = mProject.animator().currentFrame().get();
        const bool hasAreaBone = timeLine.current().areaBone();
        const bool hasAnyMesh = mTarget->gridMesh();

        mSRTKey->setKeyExists(timeLine.hasTimeKey(core::TimeKeyType_SRT, frame));
        mOpaKey->setKeyExists(timeLine.hasTimeKey(core::TimeKeyType_Opa, frame));
        mPoseKey->setKeyExists(timeLine.hasTimeKey(core::TimeKeyType_Pose, frame), hasAreaBone);
        mFFDKey->setKeyExists(timeLine.hasTimeKey(core::TimeKeyType_FFD, frame), hasAnyMesh);
    }
    else
    {
        mSRTKey->setKeyExists(false);
        mOpaKey->setKeyExists(false);
        mPoseKey->setKeyExists(false, false);
        mFFDKey->setKeyExists(false, false);
    }
}

void ObjectPanel::updateKeyValue()
{
    if (mTarget && mTarget->timeLine())
    {
        const int frame = mProject.animator().currentFrame().get();

        if (mSRTKey->keyExists())
        {
            auto key = mTarget->timeLine()->timeKey(core::TimeKeyType_SRT, frame);
            XC_PTR_ASSERT(key);

            const core::SRTKey::Data data = ((const core::SRTKey*)key)->data();
            mSRTEasing->setValue(data.easing, false);
            mSRTSpline->setValue(data.spline, false);
            mSRTTrans->setValue(data.pos.toVector2D());
            mSRTRotate->setValue(data.rotate);
            mSRTScale->setValue(data.scale);
        }
        if (mOpaKey->keyExists())
        {
            auto key = mTarget->timeLine()->timeKey(core::TimeKeyType_Opa, frame);
            XC_PTR_ASSERT(key);

            const core::OpaKey::Data data = ((const core::OpaKey*)key)->data();
            mOpaEasing->setValue(data.easing, false);
            mOpacity->setValue(data.opacity);
        }
        if (mPoseKey->keyExists())
        {
            auto key = mTarget->timeLine()->timeKey(core::TimeKeyType_Pose, frame);
            XC_PTR_ASSERT(key);

            const core::PoseKey::Data data = ((const core::PoseKey*)key)->data();
            mPoseEasing->setValue(data.easing(), false);
        }
        if (mFFDKey->keyExists())
        {
            auto key = mTarget->timeLine()->timeKey(core::TimeKeyType_FFD, frame);
            XC_PTR_ASSERT(key);

            const core::FFDKey::Data data = ((const core::FFDKey*)key)->data();
            mFFDEasing->setValue(data.easing(), false);
        }
    }
}

void ObjectPanel::assignDepth(core::Project& aProject, core::ObjectNode* aTarget, float aValue)
{
    XC_ASSERT(aTarget);
    if (!aTarget) return; // fail safe code

    const float prev = aTarget->depth();
    cmnd::ScopedMacro macro(aProject.commandStack(), "assign node depth");
    macro.grabListener(new ObjectNodeAttrNotifier(aProject, *aTarget));

    auto exec = [=](){ aTarget->setDepth(aValue); };
    auto undo = [=](){ aTarget->setDepth(prev); };
    aProject.commandStack().push(new cmnd::Delegatable(exec, undo));
}

void ObjectPanel::assignBlendMode(core::Project& aProject, core::ObjectNode* aTarget, img::BlendMode aValue)
{
    XC_ASSERT(aTarget);
    XC_PTR_ASSERT(aTarget->renderer());
    if (!aTarget || !aTarget->renderer()) return; // fail safe code

    auto prev = aTarget->renderer()->blendMode();
    cmnd::ScopedMacro macro(aProject.commandStack(), "assign blend mode");
    macro.grabListener(new ObjectNodeAttrNotifier(aProject, *aTarget));

    auto exec = [=](){ aTarget->renderer()->setBlendMode(aValue); };
    auto undo = [=](){ aTarget->renderer()->setBlendMode(prev); };
    aProject.commandStack().push(new cmnd::Delegatable(exec, undo));
}

void ObjectPanel::assignClipped(core::Project& aProject, core::ObjectNode* aTarget, bool aValue)
{
    XC_ASSERT(aTarget);
    XC_PTR_ASSERT(aTarget->renderer());
    if (!aTarget || !aTarget->renderer()) return; // fail safe code

    const bool prev = aTarget->renderer()->isClipped();
    cmnd::ScopedMacro macro(aProject.commandStack(), "assign node clippping flag");
    macro.grabListener(new ObjectNodeAttrNotifier(aProject, *aTarget));

    auto exec = [=](){ aTarget->renderer()->setClipped(aValue); };
    auto undo = [=](){ aTarget->renderer()->setClipped(prev); };
    aProject.commandStack().push(new cmnd::Delegatable(exec, undo));
}

//-------------------------------------------------------------------------------------------------
#define ASSERT_AND_RETURN_INVALID_TARGET(aTarget) \
    XC_ASSERT(aTarget && aTarget->timeLine());    \
    if (!aTarget || !aTarget->timeLine()) return; \

void ObjectPanel::assignSRTEasing(core::Project& aProject, core::ObjectNode* aTarget, util::Easing::Param aNext)
{
    ASSERT_AND_RETURN_INVALID_TARGET(aTarget);
    XC_ASSERT(aNext.isValidParam());
    const int frame = aProject.animator().currentFrame().get();
    auto newData = getSRTKeyData(*aTarget, frame);
    newData.easing = aNext;

    ctrl::TimeLineUtil::assignSRTKeyData(aProject, *aTarget, frame, newData);
}

void ObjectPanel::assignSpline(core::Project& aProject, core::ObjectNode* aTarget, int aNext)
{
    ASSERT_AND_RETURN_INVALID_TARGET(aTarget);
    XC_ASSERT(0 <= aNext && aNext < core::SRTKey::SplineType_TERM);
    const int frame = aProject.animator().currentFrame().get();
    auto newData = getSRTKeyData(*aTarget, frame);
    newData.spline = (core::SRTKey::SplineType)aNext;

    ctrl::TimeLineUtil::assignSRTKeyData(aProject, *aTarget, frame, newData);
}

void ObjectPanel::assignTrans(core::Project& aProject, core::ObjectNode* aTarget, const QVector2D& aNewPos)
{
    ASSERT_AND_RETURN_INVALID_TARGET(aTarget);
    const int frame = aProject.animator().currentFrame().get();
    auto newData = getSRTKeyData(*aTarget, frame);
    newData.pos.setX(aNewPos.x());
    newData.pos.setY(aNewPos.y());

    ctrl::TimeLineUtil::assignSRTKeyData(aProject, *aTarget, frame, newData);
}

void ObjectPanel::assignRotate(core::Project& aProject, core::ObjectNode* aTarget, float aRotate)
{
    ASSERT_AND_RETURN_INVALID_TARGET(aTarget);
    const int frame = aProject.animator().currentFrame().get();
    auto newData = getSRTKeyData(*aTarget, frame);
    newData.rotate = aRotate;

    ctrl::TimeLineUtil::assignSRTKeyData(aProject, *aTarget, frame, newData);
}

void ObjectPanel::assignScale(core::Project& aProject, core::ObjectNode* aTarget, const QVector2D& aNewScale)
{
    ASSERT_AND_RETURN_INVALID_TARGET(aTarget);
    const int frame = aProject.animator().currentFrame().get();
    auto newData = getSRTKeyData(*aTarget, frame);
    newData.scale = aNewScale;

    ctrl::TimeLineUtil::assignSRTKeyData(aProject, *aTarget, frame, newData);
}

void ObjectPanel::assignOpacity(core::Project& aProject, core::ObjectNode* aTarget, float aOpacity)
{
    ASSERT_AND_RETURN_INVALID_TARGET(aTarget);
    const int frame = aProject.animator().currentFrame().get();
    auto newData = getOpaKeyData(*aTarget, frame);
    newData.opacity = aOpacity;

    ctrl::TimeLineUtil::assignOpaKeyData(aProject, *aTarget, frame, newData);
}

void ObjectPanel::assignOpaEasing(core::Project& aProject, core::ObjectNode* aTarget, util::Easing::Param aNext)
{
    ASSERT_AND_RETURN_INVALID_TARGET(aTarget);
    XC_ASSERT(aNext.isValidParam());
    const int frame = aProject.animator().currentFrame().get();
    auto newData = getOpaKeyData(*aTarget, frame);
    newData.easing = aNext;

    ctrl::TimeLineUtil::assignOpaKeyData(aProject, *aTarget, frame, newData);
}

void ObjectPanel::assignPoseEasing(core::Project& aProject, core::ObjectNode* aTarget, util::Easing::Param aNext)
{
    ASSERT_AND_RETURN_INVALID_TARGET(aTarget);
    XC_ASSERT(aNext.isValidParam());
    const int frame = aProject.animator().currentFrame().get();
    ctrl::TimeLineUtil::assignPoseKeyEasing(aProject, *aTarget, frame, aNext);
}

void ObjectPanel::assignFFDEasing(core::Project& aProject, core::ObjectNode* aTarget, util::Easing::Param aNext)
{
    ASSERT_AND_RETURN_INVALID_TARGET(aTarget);
    XC_ASSERT(aNext.isValidParam());
    const int frame = aProject.animator().currentFrame().get();
    ctrl::TimeLineUtil::assignFFDKeyEasing(aProject, *aTarget, frame, aNext);
}

//-------------------------------------------------------------------------------------------------
void ObjectPanel::knockNewSRT(core::Project& aProject, core::ObjectNode* aTarget)
{
    ASSERT_AND_RETURN_INVALID_TARGET(aTarget);
    auto newKey = new core::SRTKey();
    newKey->data() = aTarget->timeLine()->current().srt().data();
    const int frame = aProject.animator().currentFrame().get();

    ctrl::TimeLineUtil::pushNewSRTKey(aProject, *aTarget, frame, newKey);
}

void ObjectPanel::knockNewOpacity(core::Project& aProject, core::ObjectNode* aTarget)
{
    ASSERT_AND_RETURN_INVALID_TARGET(aTarget);
    auto newKey = new core::OpaKey();
    newKey->data() = aTarget->timeLine()->current().opa();
    const int frame = aProject.animator().currentFrame().get();

    ctrl::TimeLineUtil::pushNewOpaKey(aProject, *aTarget, frame, newKey);
}

void ObjectPanel::knockNewPose(core::Project& aProject, core::ObjectNode* aTarget)
{
    ASSERT_AND_RETURN_INVALID_TARGET(aTarget);
    auto newKey = new core::PoseKey();
    newKey->data() = aTarget->timeLine()->current().pose();
    core::BoneKey* parentKey = aTarget->timeLine()->current().areaBone();
    XC_PTR_ASSERT(parentKey);
    newKey->data().createBonesBy(*parentKey);
    const int frame = aProject.animator().currentFrame().get();
    ctrl::TimeLineUtil::pushNewPoseKey(aProject, *aTarget, frame, newKey, parentKey);
}

void ObjectPanel::knockNewFFD(core::Project& aProject, core::ObjectNode* aTarget)
{
    ASSERT_AND_RETURN_INVALID_TARGET(aTarget);
    auto newKey = new core::FFDKey();
    newKey->data() = aTarget->timeLine()->current().ffd();
    core::MeshKey* parentKey = aTarget->timeLine()->current().areaMesh();
    const int frame = aProject.animator().currentFrame().get();

    ctrl::TimeLineUtil::pushNewFFDKey(aProject, *aTarget, frame, newKey, parentKey);
}

} // namespace prop
} // namespace gui

