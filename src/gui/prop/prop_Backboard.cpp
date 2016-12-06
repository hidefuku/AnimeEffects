#include "gui/prop/prop_Backboard.h"

namespace gui {
namespace prop {

Backboard::Backboard(ViaPoint& aViaPoint, QWidget* aParent)
    : QWidget(aParent)
    , mViaPoint(aViaPoint)
    , mProject()
    , mLayout(new QVBoxLayout())
    , mProjectPanel()
    , mObjectPanel()
{
    mLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    this->setLayout(mLayout);
}

void Backboard::setProject(core::Project* aProject)
{
    mProjectPanel.reset();
    mObjectPanel.reset();

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
        mObjectPanel.reset(new ObjectPanel(mViaPoint, *mProject, "Null", this));
        mLayout->addWidget(mObjectPanel.data());
        mLayout->setAlignment(mObjectPanel.data(), Qt::AlignTop);
    }
    mLayout->addStretch();
}

void Backboard::setTarget(core::ObjectNode* aNode)
{
    if (mObjectPanel)
    {
        mObjectPanel->setTarget(aNode);
    }
}

void Backboard::setPlayBackActivity(bool aIsActive)
{
    this->setEnabled(!aIsActive);

    if (mObjectPanel)
    {
        mObjectPanel->setPlayBackActivity(aIsActive);
    }
}

void Backboard::updateAttribute()
{
    if (mObjectPanel)
    {
        mObjectPanel->updateAttribute();
    }
}

void Backboard::updateKey()
{
    if (mObjectPanel)
    {
        mObjectPanel->updateKey();
    }
}

void Backboard::updateFrame()
{
    if (mObjectPanel)
    {
        mObjectPanel->updateFrame();
    }
}

} // namespace prop
} // namespace gui
