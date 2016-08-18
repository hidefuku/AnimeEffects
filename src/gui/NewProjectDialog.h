#ifndef GUI_NEWPROJECTDIALOG_H
#define GUI_NEWPROJECTDIALOG_H

#include "core/Project.h"
#include "gui/EasyDialog.h"

namespace gui
{

class NewProjectDialog : public EasyDialog
{
public:
    NewProjectDialog(const QString& aPath, QWidget* aParent);
    const core::Project::Attribute& attribute() const { return mAttribute; }

private:
    QWidget* createOption();
    core::Project::Attribute mAttribute;
};

} // namespace gui

#endif // GUI_NEWPROJECTDIALOG_H
