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
    class SRTPanel
    {
    public:
        SRTPanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
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
        Vector2DItem* mTrans;
        DecimalItem* mRotate;
        Vector2DItem* mScale;
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
        bool mKeyExists;
        ViaPoint& mViaPoint;
    };

    void build();
    void updateKeyExists();
    void updateKeyValue();

    static void assignDepth(core::Project&, core::ObjectNode*, float);
    static void assignBlendMode(core::Project&, core::ObjectNode*, img::BlendMode);
    static void assignClipped(core::Project&, core::ObjectNode*, bool);

    ViaPoint& mViaPoint;
    core::Project& mProject;
    core::ObjectNode* mTarget;
    KeyAccessor mKeyAccessor;
    int mLabelWidth;

    AttrGroup* mAttributes;
    DecimalItem* mDepth;
    ComboItem* mBlendMode;
    CheckItem* mClipped;

    QScopedPointer<SRTPanel> mSRTPanel;
    QScopedPointer<OpaPanel> mOpaPanel;
    QScopedPointer<PosePanel> mPosePanel;
    QScopedPointer<FFDPanel> mFFDPanel;
    QScopedPointer<ImagePanel> mImagePanel;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_OBJECTPANEL_H
