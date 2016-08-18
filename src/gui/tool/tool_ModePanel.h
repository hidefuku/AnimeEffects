#ifndef GUI_TOOL_MODEPANEL_H
#define GUI_TOOL_MODEPANEL_H

#include <vector>
#include <functional>
#include <QGroupBox>
#include <QPushButton>
#include <QButtonGroup>
#include "ctrl/ToolType.h"
#include "gui/GUIResourceSet.h"
#include "gui/tool/tool_FlowLayout.h"

namespace gui {
namespace tool {

class ModePanel : public QGroupBox
{
public:
    typedef std::function<void(ctrl::ToolType, bool)> PushDelegate;

    ModePanel(QWidget* aParent, GUIResourceSet& aResources);

    void addButton(ctrl::ToolType aType, const QString& aIconName,
                   const PushDelegate& aDelegate, const QString& aToolTip);

    int updateGeometry(const QPoint& aPos, int aWidth);
    QPushButton* button(int aId) { return mButtons[aId]; }
    const QPushButton* button(int aId) const { return mButtons[aId]; }

private:
    GUIResourceSet& mResources;
    QButtonGroup* mGroup;
    std::vector<QPushButton*> mButtons;
    FlowLayout mLayout;
};

} // namespace tool
} // namespace gui

#endif // GUI_TOOL_MODEPANEL_H
