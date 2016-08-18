#ifndef GUI_MENU_PROGRESSREPORTER_H
#define GUI_MENU_PROGRESSREPORTER_H

#include <QProgressDialog>
#include "util/IProgressReporter.h"

namespace gui {
namespace menu {

class ProgressReporter : public util::IProgressReporter
{
    QProgressDialog mDialog;

public:
    ProgressReporter(bool aCancelable, QWidget* aParent);

    QProgressDialog& dialog() { return mDialog; }
    const QProgressDialog& dialog() const { return mDialog; }

    virtual void setSection(const QString& aSection)
    {
        mDialog.setLabelText(aSection);
    }

    virtual void setMaximum(int aMax)
    {
        mDialog.setMaximum(aMax);
    }

    virtual void setProgress(int aValue)
    {
        mDialog.setValue(aValue);
    }

    virtual bool wasCanceled() const
    {
        return mDialog.wasCanceled();
    }
};

} // namespace menu
} // namespace gui

#endif // GUI_MENU_PROGRESSREPORTER_H
