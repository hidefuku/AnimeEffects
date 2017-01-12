#ifndef GUI_PROP_CURRENTKEYPANEL_H
#define GUI_PROP_CURRENTKEYPANEL_H

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
class MoveKeyGroup : public KeyGroup
{
    Q_OBJECT
public:
    MoveKeyGroup(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
    void setKeyEnabled(bool);
    void setKeyExists(bool);
    void setKeyValue(const core::TimeKey* aKey);
    bool keyExists() const;

private:
    KeyAccessor& mAccessor;
    KeyKnocker* mKnocker;
    EasingItem* mEasing;
    ComboItem* mSpline;
    Vector2DItem* mPosition;
    Vector2DItem* mCentroid;
    bool mKeyExists;
};

//-------------------------------------------------------------------------------------------------
class RotateKeyGroup : public KeyGroup
{
    Q_OBJECT
public:
    RotateKeyGroup(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
    void setKeyEnabled(bool);
    void setKeyExists(bool);
    void setKeyValue(const core::TimeKey* aKey);
    bool keyExists() const;

private:
    KeyAccessor& mAccessor;
    KeyKnocker* mKnocker;
    EasingItem* mEasing;
    DecimalItem* mRotate;
    bool mKeyExists;
};

//-------------------------------------------------------------------------------------------------
class ScaleKeyGroup : public KeyGroup
{
    Q_OBJECT
public:
    ScaleKeyGroup(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
    void setKeyEnabled(bool);
    void setKeyExists(bool);
    void setKeyValue(const core::TimeKey* aKey);
    bool keyExists() const;

private:
    KeyAccessor& mAccessor;
    KeyKnocker* mKnocker;
    EasingItem* mEasing;
    Vector2DItem* mScale;
    bool mKeyExists;
};

//-------------------------------------------------------------------------------------------------
class DepthKeyGroup : public KeyGroup
{
    Q_OBJECT
public:
    DepthKeyGroup(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
    void setKeyEnabled(bool);
    void setKeyExists(bool);
    void setKeyValue(const core::TimeKey* aKey);
    bool keyExists() const;

private:
    KeyAccessor& mAccessor;
    KeyKnocker* mKnocker;
    EasingItem* mEasing;
    DecimalItem* mDepth;
    bool mKeyExists;
};

//-------------------------------------------------------------------------------------------------
class OpaKeyGroup : public KeyGroup
{
    Q_OBJECT
public:
    OpaKeyGroup(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
    void setKeyEnabled(bool);
    void setKeyExists(bool);
    void setKeyValue(const core::TimeKey* aKey);
    bool keyExists() const;

private:
    KeyAccessor& mAccessor;
    KeyKnocker* mKnocker;
    EasingItem* mEasing;
    DecimalItem* mOpacity;
    bool mKeyExists;
};

//-------------------------------------------------------------------------------------------------
class PoseKeyGroup : public KeyGroup
{
    Q_OBJECT
public:
    PoseKeyGroup(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
    void setKeyEnabled(bool);
    void setKeyExists(bool, bool);
    void setKeyValue(const core::TimeKey* aKey);
    bool keyExists() const;

private:
    KeyAccessor& mAccessor;
    KeyKnocker* mKnocker;
    EasingItem* mEasing;
    bool mKeyExists;
};

//-------------------------------------------------------------------------------------------------
class FFDKeyGroup : public KeyGroup
{
    Q_OBJECT
public:
    FFDKeyGroup(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
    void setKeyEnabled(bool);
    void setKeyExists(bool, bool);
    void setKeyValue(const core::TimeKey* aKey);
    bool keyExists() const;

private:
    KeyAccessor& mAccessor;
    KeyKnocker* mKnocker;
    EasingItem* mEasing;
    bool mKeyExists;
};

//-------------------------------------------------------------------------------------------------
class ImageKeyGroup : public KeyGroup
{
    Q_OBJECT
public:
    ImageKeyGroup(Panel& aPanel, KeyAccessor& aAccessor,
               int aLabelWidth, ViaPoint& aViaPoint);
    void setKeyEnabled(bool);
    void setKeyExists(bool, bool);
    void setKeyValue(const core::TimeKey* aKey);
    bool keyExists() const;

private:
    void knockNewKey();
    KeyAccessor& mAccessor;
    KeyKnocker* mKnocker;
    BrowseItem* mBrowse;
    Vector2DItem* mOffset;
    IntegerItem* mCellSize;
    bool mKeyExists;
    ViaPoint& mViaPoint;
};

//-------------------------------------------------------------------------------------------------
class CurrentKeyPanel : public Panel
{
    Q_OBJECT
public:
    CurrentKeyPanel(ViaPoint& aViaPoint, core::Project& aProject, const QString& aTitle, QWidget* aParent);
    void setTarget(core::ObjectNode* aTarget);
    void setPlayBackActivity(bool aIsActive);
    void updateKey();
    void updateFrame();

private:
    void build();
    void updateKeyExists();
    void updateKeyValue();

    ViaPoint& mViaPoint;
    core::Project& mProject;
    core::ObjectNode* mTarget;
    KeyAccessor mKeyAccessor;
    int mLabelWidth;

    QScopedPointer<MoveKeyGroup> mMovePanel;
    QScopedPointer<RotateKeyGroup> mRotatePanel;
    QScopedPointer<ScaleKeyGroup> mScalePanel;
    QScopedPointer<DepthKeyGroup> mDepthPanel;
    QScopedPointer<OpaKeyGroup> mOpaPanel;
    QScopedPointer<PoseKeyGroup> mPosePanel;
    QScopedPointer<FFDKeyGroup> mFFDPanel;
    QScopedPointer<ImageKeyGroup> mImagePanel;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_CURRENTKEYPANEL_H
