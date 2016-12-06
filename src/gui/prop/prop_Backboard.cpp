#include "gui/prop/prop_Backboard.h"

namespace gui {
namespace prop {

Backboard::Backboard(ViaPoint& aViaPoint, QWidget* aParent)
    : QWidget(aParent)
    , mViaPoint(aViaPoint)
    , mProject()
    , mLayout(new QVBoxLayout())
    , mConstantPanel()
    , mDefaultKeyPanel()
    , mCurrentKeyPanel()
{
    mLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    this->setLayout(mLayout);
}

void Backboard::setProject(core::Project* aProject)
{
    mConstantPanel.reset();
    mDefaultKeyPanel.reset();
    mCurrentKeyPanel.reset();

    // reset layout
    {
        delete mLayout;
        mLayout = new QVBoxLayout();
        mLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        this->setLayout(mLayout);
    }

    mProject = aProject;

    if (mProject)
    {
        mConstantPanel.reset(new ConstantPanel(mViaPoint, *mProject, "Null", this));
        mLayout->addWidget(mConstantPanel.data());
        mLayout->setAlignment(mConstantPanel.data(), Qt::AlignTop);

        mDefaultKeyPanel.reset(new DefaultKeyPanel(mViaPoint, *mProject, "Null", this));
        mLayout->addWidget(mDefaultKeyPanel.data());
        mLayout->setAlignment(mDefaultKeyPanel.data(), Qt::AlignTop);

        mCurrentKeyPanel.reset(new CurrentKeyPanel(mViaPoint, *mProject, "Null", this));
        mLayout->addWidget(mCurrentKeyPanel.data());
        mLayout->setAlignment(mCurrentKeyPanel.data(), Qt::AlignTop);
    }
    mLayout->addStretch();
}

void Backboard::setTarget(core::ObjectNode* aNode)
{
    if (mConstantPanel)
    {
        mConstantPanel->setTarget(aNode);
    }
    if (mDefaultKeyPanel)
    {
        mDefaultKeyPanel->setTarget(aNode);
    }
    if (mCurrentKeyPanel)
    {
        mCurrentKeyPanel->setTarget(aNode);
    }
}

void Backboard::setPlayBackActivity(bool aIsActive)
{
    this->setEnabled(!aIsActive);

    if (mConstantPanel)
    {
        mConstantPanel->setPlayBackActivity(aIsActive);
    }
    if (mDefaultKeyPanel)
    {
        mDefaultKeyPanel->setPlayBackActivity(aIsActive);
    }
    if (mCurrentKeyPanel)
    {
        mCurrentKeyPanel->setPlayBackActivity(aIsActive);
    }
}

void Backboard::updateAttribute()
{
    if (mConstantPanel)
    {
        mConstantPanel->updateAttribute();
    }
}

void Backboard::updateKey()
{
    if (mDefaultKeyPanel)
    {
        mDefaultKeyPanel->updateKey();
    }
    if (mCurrentKeyPanel)
    {
        mCurrentKeyPanel->updateKey();
    }
}

void Backboard::updateFrame()
{
    if (mCurrentKeyPanel)
    {
        mCurrentKeyPanel->updateFrame();
    }
}

} // namespace prop
} // namespace gui
