#ifndef GUI_TOOL_MESHPANEL_H
#define GUI_TOOL_MESHPANEL_H

#include <QGroupBox>
#include <QPushButton>
#include <QButtonGroup>
#include "util/Signaler.h"
#include "ctrl/MeshParam.h"
#include "gui/GUIResources.h"
#include "gui/tool/tool_Items.h"

namespace gui {
namespace tool {

class MeshPanel : public QGroupBox
{
public:
    MeshPanel(QWidget* aParent, GUIResources& aResources);

    int updateGeometry(const QPoint& aPos, int aWidth);

    const ctrl::MeshParam& param() const { return mParam; }

    // boost like signals
    util::Signaler<void(bool)> onParamUpdated;

private:
    void createMode();

    GUIResources& mResources;
    ctrl::MeshParam mParam;
    QScopedPointer<SingleOutItem> mTypeGroup;
};

} // namespace tool
} // namespace gui

#endif // GUI_TOOL_MESHPANEL_H
