#ifndef GUI_PROP_OBJECTPANEL_H
#define GUI_PROP_OBJECTPANEL_H

#include "core/Project.h"
#include "core/ObjectNode.h"
#include "gui/prop/prop_Panel.h"
#include "gui/prop/prop_AttrGroup.h"
#include "gui/prop/prop_KeyGroup.h"
#include "gui/prop/prop_Items.h"

namespace gui {
namespace prop {

class ObjectPanel : public Panel
{
public:
    ObjectPanel(core::Project& aProject, const QString& aTitle, QWidget* aParent);
    void setTarget(core::ObjectNode* aTarget);
    void updateAttribute();
    void updateKey();
    void updateFrame();

private:
    void build();
    void updateKeyExists();
    void updateKeyValue();

    static void assignDepth(core::Project&, core::ObjectNode*, float);
    static void assignBlendMode(core::Project&, core::ObjectNode*, img::BlendMode);
    static void assignClipped(core::Project&, core::ObjectNode*, bool);

    static void assignSRTEasing(core::Project&, core::ObjectNode*, util::Easing::Param);
    static void assignSpline(core::Project&, core::ObjectNode*, int);
    static void assignTrans(core::Project&, core::ObjectNode*, const QVector2D&);
    static void assignRotate(core::Project&, core::ObjectNode*, float);
    static void assignScale(core::Project&, core::ObjectNode*, const QVector2D&);

    static void assignOpaEasing(core::Project&, core::ObjectNode*, util::Easing::Param);
    static void assignOpacity(core::Project&, core::ObjectNode*, float);

    static void assignPoseEasing(core::Project&, core::ObjectNode*, util::Easing::Param);

    static void assignFFDEasing(core::Project&, core::ObjectNode*, util::Easing::Param);

    static void knockNewSRT(core::Project&, core::ObjectNode*);
    static void knockNewOpacity(core::Project&, core::ObjectNode*);
    static void knockNewPose(core::Project&, core::ObjectNode*);
    static void knockNewFFD(core::Project&, core::ObjectNode*);

    core::Project& mProject;
    core::ObjectNode* mTarget;
    int mLabelWidth;

    AttrGroup* mAttributes;
    DecimalItem* mDepth;
    ComboItem* mBlendMode;
    CheckItem* mClipped;

    KeyGroup* mSRTKey;
    EasingItem* mSRTEasing;
    ComboItem* mSRTSpline;
    Vector2DItem* mSRTTrans;
    DecimalItem* mSRTRotate;
    Vector2DItem* mSRTScale;

    KeyGroup* mOpaKey;
    EasingItem* mOpaEasing;
    DecimalItem* mOpacity;

    KeyGroup* mPoseKey;
    EasingItem* mPoseEasing;

    KeyGroup* mFFDKey;
    EasingItem* mFFDEasing;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_OBJECTPANEL_H
