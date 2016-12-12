#ifndef GUI_NEWPROJECTDIALOG_H
#define GUI_NEWPROJECTDIALOG_H

#include "core/Project.h"
#include "gui/EasyDialog.h"

namespace gui
{

class NewProjectDialog : public EasyDialog
{
    Q_OBJECT
public:
    NewProjectDialog(QWidget* aParent);
    QString fileName() const { return mFileName; }
    const core::Project::Attribute& attribute() const { return mAttribute; }
    bool specifiesCanvasSize() const { return mSpecifiesCanvasSize; }

private:
    QWidget* createOption();
    QString mFileName;
    core::Project::Attribute mAttribute;
    bool mSpecifiesCanvasSize;
};

} // namespace gui

#endif // GUI_NEWPROJECTDIALOG_H
