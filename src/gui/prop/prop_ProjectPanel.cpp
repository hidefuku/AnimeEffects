#include "gui/prop/prop_ProjectPanel.h"

namespace gui {
namespace prop {

ProjectPanel::ProjectPanel(core::Project& aProject, QWidget* aParent)
    : Panel("Project", aParent)
    , mProject(aProject)
    , mAttributes(new AttrGroup("Time", 0))

{
}

} // namespace prop
} // namespace gui
