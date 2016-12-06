#include "cmnd/ScopedMacro.h"
#include "cmnd/BasicCommands.h"
#include "core/Constant.h"
#include "core/TimeKeyExpans.h"
#include "core/DepthKey.h"
#include "ctrl/TimeLineUtil.h"
#include "gui/ResourceDialog.h"
#include "gui/prop/prop_DefaultKeyPanel.h"
#include "gui/prop/prop_Items.h"

namespace gui {
namespace prop {

//-------------------------------------------------------------------------------------------------
DefaultKeyPanel::DefaultPanel::DefaultPanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
    : mAccessor(aAccessor)
    , mGroup()
    , mDepth()
    , mOpacity()
{
    mGroup = new KeyGroup("Default", aLabelWidth);
    {
        aPanel.addGroup(mGroup);

        // depth
        mDepth = new DecimalItem(mGroup);
        mDepth->setRange(core::Constant::transMin(), core::Constant::transMax());
        mDepth->onValueUpdated = [=](double, double aNext)
        {
            this->mAccessor.assignDefaultDepth(aNext);
        };
        mGroup->addItem("depth :", mDepth);

        // opacity
        mOpacity = new DecimalItem(mGroup);
        mOpacity->setRange(0.0f, 1.0f);
        mOpacity->onValueUpdated = [=](double, double aNext)
        {
            this->mAccessor.assignDefaultOpacity(aNext);
        };
        mGroup->addItem("opacity :", mOpacity);
    }
    mGroup->setEnabled(false);
}

void DefaultKeyPanel::DefaultPanel::setEnabled(bool aEnabled)
{
    mGroup->setEnabled(aEnabled);
}

void DefaultKeyPanel::DefaultPanel::setKeyValue(const core::TimeLine& aLine)
{
    auto depthKey = (core::DepthKey*)aLine.defaultKey(core::TimeKeyType_Depth);
    if (depthKey)
    {
        mDepth->setItemEnabled(true);
        mDepth->setValue(depthKey->depth());
    }
    else
    {
        mDepth->setItemEnabled(false);
    }

    auto opaKey = (core::OpaKey*)aLine.defaultKey(core::TimeKeyType_Opa);
    if (opaKey)
    {
        mOpacity->setItemEnabled(true);
        mOpacity->setValue(opaKey->opacity());
    }
    else
    {
        mOpacity->setItemEnabled(false);
    }
}

//-------------------------------------------------------------------------------------------------
DefaultKeyPanel::DefaultKeyPanel(ViaPoint& aViaPoint, core::Project& aProject, const QString& aTitle, QWidget* aParent)
    : Panel(aTitle, aParent)
    , mViaPoint(aViaPoint)
    , mProject(aProject)
    , mTarget()
    , mKeyAccessor()
    , mLabelWidth()
    , mDefaultPanel()
{
    mKeyAccessor.setProject(&aProject);
    mLabelWidth = this->fontMetrics().boundingRect("MaxTextWidth :").width();

    build();
    this->hide();
}

void DefaultKeyPanel::setTarget(core::ObjectNode* aTarget)
{
    mTarget = aTarget;
    mKeyAccessor.setTarget(aTarget);

    if (mTarget)
    {
        this->setTitle(mTarget->name() + " Default Keys");
        this->show();
    }
    else
    {
        this->hide();
    }

    updateKey();
}

void DefaultKeyPanel::setPlayBackActivity(bool aIsActive)
{
    // resume
    if (!this->isEnabled() && !aIsActive)
    {
        updateKeyExists();
        updateKeyValue();
    }
    this->setEnabled(!aIsActive);
}

void DefaultKeyPanel::updateKey()
{
    updateKeyExists();
    updateKeyValue();
}

void DefaultKeyPanel::build()
{
    using core::Constant;

    mDefaultPanel.reset(new DefaultPanel(*this, mKeyAccessor, mLabelWidth));

    this->addStretch();
}

void DefaultKeyPanel::updateKeyExists()
{
    if (mTarget && mTarget->timeLine())
    {
        mDefaultPanel->setEnabled(true);
    }
    else
    {
        mDefaultPanel->setEnabled(false);
    }
}

void DefaultKeyPanel::updateKeyValue()
{
    if (mTarget && mTarget->timeLine())
    {
        auto& timeLine = *mTarget->timeLine();
        mDefaultPanel->setKeyValue(timeLine);
    }
}

} // namespace prop
} // namespace gui
