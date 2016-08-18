#ifndef GUI_TOOL_SRTPANEL_H
#define GUI_TOOL_SRTPANEL_H

#include <vector>
#include <functional>
#include <QGroupBox>
#include <QPushButton>
#include <QSlider>
#include <QButtonGroup>
#include "util/Signaler.h"
#include "ctrl/SRTParam.h"
#include "gui/GUIResourceSet.h"
#include "gui/tool/tool_Items.h"

namespace gui {
namespace tool {

class SRTPanel : public QGroupBox
{
public:
    SRTPanel(QWidget* aParent, GUIResourceSet& aResources);

    int updateGeometry(const QPoint& aPos, int aWidth);

    const ctrl::SRTParam& param() const { return mParam; }

    // boost like signals
    util::Signaler<void(bool)> onParamUpdated;

private:
    void createMode();
    void updateTypeParam(int aType);

    GUIResourceSet& mResources;
    ctrl::SRTParam mParam;
    QScopedPointer<SingleOutItem> mTypeGroup;
};

} // namespace tool
} // namespace gui

#endif // GUI_TOOL_SRTPANEL_H
