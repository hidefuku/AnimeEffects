#ifndef GUI_PROP_OBJECTPANEL_H
#define GUI_PROP_OBJECTPANEL_H

#include "core/Project.h"
#include "core/ObjectNode.h"
#include "gui/ViaPoint.h"
#include "gui/prop/prop_Panel.h"
#include "gui/prop/prop_AttrGroup.h"
#include "gui/prop/prop_KeyGroup.h"
#include "gui/prop/prop_KeyKnocker.h"
#include "gui/prop/prop_Items.h"
#include "gui/prop/prop_KeyAccessor.h"

namespace gui {
namespace prop {

class ObjectPanel : public Panel
{
public:
    ObjectPanel(ViaPoint& aViaPoint, core::Project& aProject, const QString& aTitle, QWidget* aParent);
    void setTarget(core::ObjectNode* aTarget);
    void setPlayBackActivity(bool aIsActive);
    void updateAttribute();
    void updateKey();
    void updateFrame();

private:
    class DefaultPanel
    {
    public:
        DefaultPanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
        void setEnabled(bool);
        void setKeyValue(const core::TimeLine& aLine);

    private:
        KeyAccessor& mAccessor;
        KeyGroup* mGroup;
        DecimalItem* mDepth;
        DecimalItem* mOpacity;
    };

    class MovePanel
    {
    public:
        MovePanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
        void setEnabled(bool);
        void setKeyExists(bool);
        void setKeyValue(const core::TimeKey* aKey);
        bool keyExists() const;

    private:
        KeyAccessor& mAccessor;
        KeyKnocker* mKnocker;
        KeyGroup* mGroup;
        EasingItem* mEasing;
        ComboItem* mSpline;
        Vector2DItem* mMove;
        bool mKeyExists;
    };

    class RotatePanel
    {
    public:
        RotatePanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
        void setEnabled(bool);
        void setKeyExists(bool);
        void setKeyValue(const core::TimeKey* aKey);
        bool keyExists() const;

    private:
        KeyAccessor& mAccessor;
        KeyKnocker* mKnocker;
        KeyGroup* mGroup;
        EasingItem* mEasing;
        DecimalItem* mRotate;
        bool mKeyExists;
    };

    class ScalePanel
    {
    public:
        ScalePanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
        void setEnabled(bool);
        void setKeyExists(bool);
        void setKeyValue(const core::TimeKey* aKey);
        bool keyExists() const;

    private:
        KeyAccessor& mAccessor;
        KeyKnocker* mKnocker;
        KeyGroup* mGroup;
        EasingItem* mEasing;
        Vector2DItem* mScale;
        bool mKeyExists;
    };

    class DepthPanel
    {
    public:
        DepthPanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
        void setEnabled(bool);
        void setKeyExists(bool);
        void setKeyValue(const core::TimeKey* aKey);
        bool keyExists() const;

    private:
        KeyAccessor& mAccessor;
        KeyKnocker* mKnocker;
        KeyGroup* mGroup;
        EasingItem* mEasing;
        DecimalItem* mDepth;
        bool mKeyExists;
    };

    class OpaPanel
    {
    public:
        OpaPanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
        void setEnabled(bool);
        void setKeyExists(bool);
        void setKeyValue(const core::TimeKey* aKey);
        bool keyExists() const;

    private:
        KeyAccessor& mAccessor;
        KeyKnocker* mKnocker;
        KeyGroup* mGroup;
        EasingItem* mEasing;
        DecimalItem* mOpacity;
        bool mKeyExists;
    };

    class PosePanel
    {
    public:
        PosePanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
        void setEnabled(bool);
        void setKeyExists(bool, bool);
        void setKeyValue(const core::TimeKey* aKey);
        bool keyExists() const;

    private:
        KeyAccessor& mAccessor;
        KeyKnocker* mKnocker;
        KeyGroup* mGroup;
        EasingItem* mEasing;
        bool mKeyExists;
    };

    class FFDPanel
    {
    public:
        FFDPanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
        void setEnabled(bool);
        void setKeyExists(bool, bool);
        void setKeyValue(const core::TimeKey* aKey);
        bool keyExists() const;

    private:
        KeyAccessor& mAccessor;
        KeyKnocker* mKnocker;
        KeyGroup* mGroup;
        EasingItem* mEasing;
        bool mKeyExists;
    };

    class ImagePanel
    {
    public:
        ImagePanel(Panel& aPanel, KeyAccessor& aAccessor,
                   int aLabelWidth, ViaPoint& aViaPoint);
        void setEnabled(bool);
        void setKeyExists(bool, bool);
        void setKeyValue(const core::TimeKey* aKey);
        bool keyExists() const;

    private:
        void knockNewKey();
        KeyAccessor& mAccessor;
        KeyKnocker* mKnocker;
        KeyGroup* mGroup;
        BrowseItem* mBrowse;
        Vector2DItem* mOffset;
        bool mKeyExists;
        ViaPoint& mViaPoint;
    };

    void build();
    void updateKeyExists();
    void updateKeyValue();

    static void assignBlendMode(core::Project&, core::ObjectNode*, img::BlendMode);
    static void assignClipped(core::Project&, core::ObjectNode*, bool);

    ViaPoint& mViaPoint;
    core::Project& mProject;
    core::ObjectNode* mTarget;
    KeyAccessor mKeyAccessor;
    int mLabelWidth;

    AttrGroup* mAttributes;
    ComboItem* mBlendMode;
    CheckItem* mClipped;

    QScopedPointer<DefaultPanel> mDefaultPanel;

    QScopedPointer<MovePanel> mMovePanel;
    QScopedPointer<RotatePanel> mRotatePanel;
    QScopedPointer<ScalePanel> mScalePanel;
    QScopedPointer<DepthPanel> mDepthPanel;
    QScopedPointer<OpaPanel> mOpaPanel;
    QScopedPointer<PosePanel> mPosePanel;
    QScopedPointer<FFDPanel> mFFDPanel;
    QScopedPointer<ImagePanel> mImagePanel;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_OBJECTPANEL_H
