#ifndef GUI_TOOL_POSEPANEL_H
#define GUI_TOOL_POSEPANEL_H

#include <QGroupBox>
#include <QPushButton>
#include <QButtonGroup>
#include "util/Signaler.h"
#include "ctrl/PoseParam.h"
#include "gui/GUIResources.h"
#include "gui/tool/tool_Items.h"

namespace gui {
namespace tool {

class PosePanel : public QGroupBox
{
    Q_OBJECT
public:
    PosePanel(QWidget* aParent, GUIResources& aResources);

    int updateGeometry(const QPoint& aPos, int aWidth);

    const ctrl::PoseParam& param() const { return mParam; }

    // boost like signals
    util::Signaler<void(bool)> onParamUpdated;

private:
    void createMode();
    void updateTypeParam(ctrl::PoseEditMode aType);

    GUIResources& mResources;
    ctrl::PoseParam mParam;
    QScopedPointer<SingleOutItem> mTypeGroup;
    QScopedPointer<SliderItem> mDIRadius;
    QScopedPointer<SliderItem> mDIPressure;
    QScopedPointer<SliderItem> mEIRadius;
    QScopedPointer<SliderItem> mEIPressure;
};

} // namespace tool
} // namespace gui

#endif // GUI_TOOL_POSEPANEL_H
