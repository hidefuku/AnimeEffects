#ifndef GUI_PROP_DEFAULTKEYPANEL_H
#define GUI_PROP_DEFAULTKEYPANEL_H

#include "core/Project.h"
#include "core/ObjectNode.h"
#include "gui/ViaPoint.h"
#include "gui/prop/prop_Panel.h"
#include "gui/prop/prop_KeyGroup.h"
#include "gui/prop/prop_KeyKnocker.h"
#include "gui/prop/prop_Items.h"
#include "gui/prop/prop_KeyAccessor.h"

namespace gui {
namespace prop {

//-------------------------------------------------------------------------------------------------
class DefaultDepthGroup : public KeyGroup
{
    Q_OBJECT
public:
    DefaultDepthGroup(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
    void setKeyValue(const core::TimeLine& aLine);
private:
    KeyAccessor& mAccessor;
    DecimalItem* mDepth;
};

//-------------------------------------------------------------------------------------------------
class DefaultOpaGroup : public KeyGroup
{
    Q_OBJECT
public:
    DefaultOpaGroup(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
    void setKeyValue(const core::TimeLine& aLine);
private:
    KeyAccessor& mAccessor;
    DecimalItem* mOpacity;
};

//-------------------------------------------------------------------------------------------------
class DefaultImageGroup : public KeyGroup
{
    Q_OBJECT
public:
    DefaultImageGroup(Panel& aPanel, KeyAccessor& aAccessor,
                      int aLabelWidth, ViaPoint& aViaPoint);
    void setKeyValue(const core::TimeLine& aLine);
private:
    KeyAccessor& mAccessor;
    BrowseItem* mBrowse;
    Vector2DItem* mOffset;
    IntegerItem* mCellSize;
    ViaPoint& mViaPoint;
};

//-------------------------------------------------------------------------------------------------
class DefaultKeyPanel : public Panel
{
    Q_OBJECT
public:
    DefaultKeyPanel(ViaPoint& aViaPoint, core::Project& aProject, const QString& aTitle, QWidget* aParent);
    void setTarget(core::ObjectNode* aTarget);
    void setPlayBackActivity(bool aIsActive);
    void updateKey();

private:
    void build();
    void updateKeyExists();
    void updateKeyValue();

    ViaPoint& mViaPoint;
    core::Project& mProject;
    core::ObjectNode* mTarget;
    KeyAccessor mKeyAccessor;
    int mLabelWidth;

    QScopedPointer<DefaultDepthGroup> mDepthPanel;
    QScopedPointer<DefaultOpaGroup> mOpaPanel;
    QScopedPointer<DefaultImageGroup> mImagePanel;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_DEFAULTKEYPANEL_H
