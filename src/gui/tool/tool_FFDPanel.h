#ifndef GUI_TOOL_FFDPANEL_H
#define GUI_TOOL_FFDPANEL_H

#include <vector>
#include <functional>
#include <QGroupBox>
#include <QPushButton>
#include <QSlider>
#include <QButtonGroup>
#include "util/Signaler.h"
#include "ctrl/FFDParam.h"
#include "gui/GUIResources.h"
#include "gui/tool/tool_Items.h"

namespace gui {
namespace tool {

class FFDPanel : public QGroupBox
{
public:
    FFDPanel(QWidget* aParent, GUIResources& aResources);

    int updateGeometry(const QPoint& aPos, int aWidth);

    const ctrl::FFDParam& param() const { return mParam; }

    // boost like signals
    util::Signaler<void(bool)> onParamUpdated;

private:
    void createBrush();
    void updateTypeParam(ctrl::FFDParam::Type aType);

    GUIResources& mResources;
    ctrl::FFDParam mParam;
    QScopedPointer<SingleOutItem> mTypeGroup;
    QScopedPointer<SingleOutItem> mHardnessGroup;
    QScopedPointer<SliderItem> mRadius;
    QScopedPointer<SliderItem> mPressure;
    QScopedPointer<SliderItem> mBlur;
    QScopedPointer<SliderItem> mEraseRadius;
    QScopedPointer<SliderItem> mErasePressure;
};

} // namespace tool
} // namespace gui

#endif // GUI_TOOL_FFDPANEL_H
