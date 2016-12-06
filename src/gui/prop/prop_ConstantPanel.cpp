#include "cmnd/ScopedMacro.h"
#include "cmnd/BasicCommands.h"
#include "core/Constant.h"
#include "gui/ResourceDialog.h"
#include "gui/prop/prop_ConstantPanel.h"
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
}

namespace gui {
namespace prop {

//-------------------------------------------------------------------------------------------------
ConstantPanel::ConstantPanel(ViaPoint& aViaPoint, core::Project& aProject, const QString& aTitle, QWidget* aParent)
    : Panel(aTitle, aParent)
    , mViaPoint(aViaPoint)
    , mProject(aProject)
    , mTarget()
    , mLabelWidth()
    , mRenderingAttributes()
    , mBlendMode()
    , mClipped()
{
    mLabelWidth = this->fontMetrics().boundingRect("MaxTextWidth :").width();

    build();
    this->hide();
}

void ConstantPanel::setTarget(core::ObjectNode* aTarget)
{
    mTarget = aTarget;

    if (mTarget)
    {
        this->setTitle(mTarget->name() + " Constant");
        this->show();
    }
    else
    {
        this->hide();
    }

    updateAttribute();
}

void ConstantPanel::setPlayBackActivity(bool aIsActive)
{
    this->setEnabled(!aIsActive);
}

void ConstantPanel::build()
{
    using core::Constant;

    mRenderingAttributes = new AttrGroup("Rendering", mLabelWidth);
    {
        this->addGroup(mRenderingAttributes);

        // blend mode
        mBlendMode = new ComboItem(mRenderingAttributes);
        for (int i = 0; i < img::BlendMode_TERM; ++i)
        {
            auto mode = (img::BlendMode)i;
            mBlendMode->box().addItem(img::getBlendNameFromBlendMode(mode));
        }
        mBlendMode->onValueUpdated = [=](int, int aNext)
        {
            assignBlendMode(this->mProject, this->mTarget, (img::BlendMode)aNext);
        };
        mRenderingAttributes->addItem("blend :", mBlendMode);

        // clipped
        mClipped = new CheckItem(mRenderingAttributes);
        mClipped->onValueUpdated = [=](bool aNext)
        {
            assignClipped(this->mProject, this->mTarget, aNext);
        };
        mRenderingAttributes->addItem("clipped :", mClipped);
    }

    this->addStretch();
}

void ConstantPanel::updateAttribute()
{
    if (mTarget)
    {
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

void ConstantPanel::assignBlendMode(core::Project& aProject, core::ObjectNode* aTarget, img::BlendMode aValue)
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

void ConstantPanel::assignClipped(core::Project& aProject, core::ObjectNode* aTarget, bool aValue)
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

} // namespace prop
} // namespace gui
