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
DefaultDepthGroup::DefaultDepthGroup(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
    : KeyGroup(tr("Depth"), aLabelWidth)
    , mAccessor(aAccessor)
    , mDepth()
{
    {
        aPanel.addGroup(this);

        // depth
        mDepth = new DecimalItem(this);
        mDepth->setRange(core::Constant::transMin(), core::Constant::transMax());
        mDepth->onValueUpdated = [=](double, double aNext)
        {
            this->mAccessor.assignDefaultDepth(aNext);
        };
        this->addItem(tr("position :"), mDepth);
    }
    this->setEnabled(false);
}

void DefaultDepthGroup::setKeyValue(const core::TimeLine& aLine)
{
    auto key = (core::DepthKey*)aLine.defaultKey(core::TimeKeyType_Depth);
    this->setVisible((bool)key);
    if (key)
    {
        mDepth->setValue(key->depth());
    }
}

//-------------------------------------------------------------------------------------------------
DefaultOpaGroup::DefaultOpaGroup(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth)
    : KeyGroup(tr("Opacity"), aLabelWidth)
    , mAccessor(aAccessor)
    , mOpacity()
{
    {
        aPanel.addGroup(this);

        // opacity
        mOpacity = new DecimalItem(this);
        mOpacity->setRange(0.0f, 1.0f);
        mOpacity->onValueUpdated = [=](double, double aNext)
        {
            this->mAccessor.assignDefaultOpacity(aNext);
        };
        this->addItem(tr("rate :"), mOpacity);
    }
    this->setEnabled(false);
}

void DefaultOpaGroup::setKeyValue(const core::TimeLine& aLine)
{
    auto key = (core::OpaKey*)aLine.defaultKey(core::TimeKeyType_Opa);
    this->setVisible((bool)key);
    if (key)
    {
        mOpacity->setValue(key->opacity());
    }
}

//-------------------------------------------------------------------------------------------------
DefaultImageGroup::DefaultImageGroup(
        Panel& aPanel, KeyAccessor& aAccessor,
        int aLabelWidth, ViaPoint& aViaPoint)
    : KeyGroup(tr("Image"), aLabelWidth)
    , mAccessor(aAccessor)
    , mBrowse()
    , mOffset()
    , mCellSize()
    , mViaPoint(aViaPoint)
{
    {
        aPanel.addGroup(this);

        // browse
        mBrowse = new BrowseItem(this);
        mBrowse->onButtonPushed = [=]()
        {
            auto resNode = this->mViaPoint.requireOneResource();
            if (resNode)
            {
                this->mBrowse->setValue(resNode->data().identifier());
                this->mAccessor.assignDefaultImageResource(*resNode);
            }
        };
        this->addItem(tr("resource :"), mBrowse);

        // offset
        mOffset = new Vector2DItem(this);
        mOffset->setRange(core::Constant::transMin(), core::Constant::transMax());
        mOffset->onValueUpdated = [=](QVector2D, QVector2D aNext)
        {
            this->mAccessor.assignDefaultImageOffset(-aNext);
        };
        this->addItem(tr("center :"), mOffset);

        // cell size
        mCellSize = new IntegerItem(this);
        mCellSize->setRange(core::Constant::imageCellSizeMin(),
                            core::Constant::imageCellSizeMax());
        mCellSize->onValueUpdated = [=](int, int aNext)
        {
            this->mAccessor.assignDefaultImageCellSize(aNext);
        };
        this->addItem(tr("mesh cell :"), mCellSize);
    }
    setEnabled(false);
}

void DefaultImageGroup::setKeyValue(const core::TimeLine& aLine)
{
    auto key = (const core::ImageKey*)aLine.defaultKey(core::TimeKeyType_Image);
    this->setVisible((bool)key);
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
    mLabelWidth = this->fontMetrics().boundingRect(tr("MaxTextWidth :")).width();

    build();
    this->hide();
}

void DefaultKeyPanel::setTarget(core::ObjectNode* aTarget)
{
    mTarget = aTarget;
    mKeyAccessor.setTarget(aTarget);

    if (mTarget)
    {
        this->setTitle(mTarget->name() + " : " + tr("Default Keys"));
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

    mDepthPanel.reset(new DefaultDepthGroup(*this, mKeyAccessor, mLabelWidth));
    mOpaPanel.reset(new DefaultOpaGroup(*this, mKeyAccessor, mLabelWidth));
    mImagePanel.reset(new DefaultImageGroup(*this, mKeyAccessor, mLabelWidth, mViaPoint));

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
