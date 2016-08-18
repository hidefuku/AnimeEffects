#include "gui/menu/menu_ProgressReporter.h"

namespace gui {
namespace menu {

ProgressReporter::ProgressReporter(bool aCancelable, QWidget* aParent)
    : mDialog("Operation in Progress...", "Cancel", 0, 100, aParent)
{
    mDialog.setWindowModality(Qt::WindowModal);
    mDialog.setAutoReset(false);
    mDialog.setMinimumDuration(0);

    if (!aCancelable)
    {
        mDialog.setCancelButton(nullptr);
    }
}

} // namespace menu
} // namespace gui
