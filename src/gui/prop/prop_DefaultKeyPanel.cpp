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
DefaultKeyPanel::DepthPanel::DepthPanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
    : mAccessor(aAccessor)
    , mGroup()
    , mDepth()
{
    mGroup = new KeyGroup("Depth", aLabelWidth);
    {
        aPanel.addGroup(mGroup);

        // depth
        mDepth = new DecimalItem(mGroup);
        mDepth->setRange(core::Constant::transMin(), core::Constant::transMax());
        mDepth->onValueUpdated = [=](double, double aNext)
        {
            this->mAccessor.assignDefaultDepth(aNext);
        };
        mGroup->addItem("position :", mDepth);
    }
    mGroup->setEnabled(false);
}

void DefaultKeyPanel::DepthPanel::setEnabled(bool aEnabled)
{
    mGroup->setEnabled(aEnabled);
}

void DefaultKeyPanel::DepthPanel::setKeyValue(const core::TimeLine& aLine)
{
    auto key = (core::DepthKey*)aLine.defaultKey(core::TimeKeyType_Depth);
    mGroup->setVisible((bool)key);
    if (key)
    {
        mDepth->setValue(key->depth());
    }
}

//-------------------------------------------------------------------------------------------------
DefaultKeyPanel::OpaPanel::OpaPanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
    : mAccessor(aAccessor)
    , mGroup()
    , mOpacity()
{
    mGroup = new KeyGroup("Opacity", aLabelWidth);
    {
        aPanel.addGroup(mGroup);

        // opacity
        mOpacity = new DecimalItem(mGroup);
        mOpacity->setRange(0.0f, 1.0f);
        mOpacity->onValueUpdated = [=](double, double aNext)
        {
            this->mAccessor.assignDefaultOpacity(aNext);
        };
        mGroup->addItem("rate :", mOpacity);
    }
    mGroup->setEnabled(false);
}

void DefaultKeyPanel::OpaPanel::setEnabled(bool aEnabled)
{
    mGroup->setEnabled(aEnabled);
}

void DefaultKeyPanel::OpaPanel::setKeyValue(const core::TimeLine& aLine)
{
    auto key = (core::OpaKey*)aLine.defaultKey(core::TimeKeyType_Opa);
    mGroup->setVisible((bool)key);
    if (key)
    {
        mOpacity->setValue(key->opacity());
    }
}

//-------------------------------------------------------------------------------------------------
DefaultKeyPanel::ImagePanel::ImagePanel(
        Panel& aPanel, KeyAccessor& aAccessor,
        int aLabelWidth, ViaPoint& aViaPoint)
    : mAccessor(aAccessor)
    , mGroup()
    , mBrowse()
    , mOffset()
    , mCellSize()
    , mViaPoint(aViaPoint)
{
    mGroup = new KeyGroup("Image", aLabelWidth);
    {
        aPanel.addGroup(mGroup);

        // browse
        mBrowse = new BrowseItem(mGroup);
        mGroup->addItem("resource :", mBrowse);
        mBrowse->onButtonPushed = [=]()
        {
            auto resNode = this->mViaPoint.requireOneResource();
            if (resNode)
            {
                this->mBrowse->setValue(resNode->data().identifier());
                this->mAccessor.assignDefaultImageResource(*resNode);
            }
        };
        // offset
        mOffset = new Vector2DItem(mGroup);
        mOffset->setRange(core::Constant::transMin(), core::Constant::transMax());
        mOffset->onValueUpdated = [=](QVector2D, QVector2D aNext)
        {
            this->mAccessor.assignDefaultImageOffset(-aNext);
        };
        mGroup->addItem("center :", mOffset);

        // cell size
        mCellSize = new IntegerItem(mGroup);
        mCellSize->setRange(core::Constant::imageCellSizeMin(),
                            core::Constant::imageCellSizeMax());
        mCellSize->onValueUpdated = [=](int, int aNext)
        {
            this->mAccessor.assignDefaultImageCellSize(aNext);
        };
        mGroup->addItem("mesh cell size :", mCellSize);
    }
    setEnabled(false);
}

void DefaultKeyPanel::ImagePanel::setEnabled(bool aEnabled)
{
    mGroup->setEnabled(aEnabled);
}

void DefaultKeyPanel::ImagePanel::setKeyValue(const core::TimeLine& aLine)
{
    auto key = (const core::ImageKey*)aLine.defaultKey(core::TimeKeyType_Image);
    mGroup->setVisible((bool)key);
    if (key)
    {
        TIMEKEY_PTR_TYPE_ASSERT(key, Image);
        const core::ImageKey::Data& data = key->data();
        mBrowse->setValue(data.resource()->identifier());
        mOffset->setValue(-data.imageOffset());
        mCellSize->setValue(data.gridMesh().cellSize());
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
    , mDepthPanel()
    , mOpaPanel()
    , mImagePanel()
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

    updateKeyExists();
    updateKeyValue();
}

void DefaultKeyPanel::setPlayBackActivity(bool aIsActive)
{
    // resume
    if (!this->isEnabled() && !aIsActive)
    {
        updateKeyValue();
    }
    this->setEnabled(!aIsActive);
}

void DefaultKeyPanel::updateKey()
{
    updateKeyValue();
}

void DefaultKeyPanel::build()
{
    using core::Constant;

    mDepthPanel.reset(new DepthPanel(*this, mKeyAccessor, mLabelWidth));
    mOpaPanel.reset(new OpaPanel(*this, mKeyAccessor, mLabelWidth));
    mImagePanel.reset(new ImagePanel(*this, mKeyAccessor, mLabelWidth, mViaPoint));

    this->addStretch();
}

void DefaultKeyPanel::updateKeyExists()
{
    bool enable = mTarget && mTarget->timeLine();
    mDepthPanel->setEnabled(enable);
    mOpaPanel->setEnabled(enable);
    mImagePanel->setEnabled(enable);
}

void DefaultKeyPanel::updateKeyValue()
{
    if (mTarget && mTarget->timeLine())
    {
        auto& timeLine = *mTarget->timeLine();
        mDepthPanel->setKeyValue(timeLine);
        mOpaPanel->setKeyValue(timeLine);
        mImagePanel->setKeyValue(timeLine);
    }
}

} // namespace prop
} // namespace gui
