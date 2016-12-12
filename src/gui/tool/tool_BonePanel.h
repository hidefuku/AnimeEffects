#ifndef GUI_TOOL_BONEPANEL_H
#define GUI_TOOL_BONEPANEL_H

#include <QGroupBox>
#include <QPushButton>
#include <QButtonGroup>
#include "util/Signaler.h"
#include "ctrl/BoneParam.h"
#include "gui/GUIResources.h"
#include "gui/tool/tool_Items.h"

namespace gui {
namespace tool {

class BonePanel : public QGroupBox
{
    Q_OBJECT
public:
    BonePanel(QWidget* aParent, GUIResources& aResources);

    int updateGeometry(const QPoint& aPos, int aWidth);

    const ctrl::BoneParam& param() const { return mParam; }

    // boost like signals
    util::Signaler<void(bool)> onParamUpdated;

private:
    void createMode();
    void updateTypeParam(ctrl::BoneEditMode aType);

    GUIResources& mResources;
    ctrl::BoneParam mParam;
    QScopedPointer<SingleOutItem> mTypeGroup;
    QScopedPointer<SliderItem> mPIRadius;
    QScopedPointer<SliderItem> mPIPressure;
    QScopedPointer<SliderItem> mEIRadius;
    QScopedPointer<SliderItem> mEIPressure;
};

} // namespace tool
} // namespace gui

#endif // GUI_TOOL_BONEPANEL_H
