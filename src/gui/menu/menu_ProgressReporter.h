#ifndef GUI_MENU_PROGRESSREPORTER_H
#define GUI_MENU_PROGRESSREPORTER_H

#include <QLabel>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include "util/IProgressReporter.h"
#include "ctrl/UILogger.h"
#include "gui/EasyDialog.h"

namespace gui {
namespace menu {

//-------------------------------------------------------------------------------------------------
class ProgressReporter : public util::IProgressReporter
{
public:
    ProgressReporter(bool aCancelable, QWidget* aParent);

    void cancel();

    virtual void setSection(const QString& aSection);
    virtual void setMaximum(int aMax);
    virtual void setProgress(int aValue);
    virtual bool wasCanceled() const;

private:
    QProgressDialog mDialog;
};

//-------------------------------------------------------------------------------------------------
class LoggableProgressDialog : public gui::EasyDialog
{
public:
    LoggableProgressDialog(bool aCancelable, QWidget* aParent);
    void cancel();
    void setLabelText(const QString& aText);
    void setMaximum(int aMax);
    void setValue(int aValue);
    void setLog(const QString& aMessage);
    bool wasCanceled() const;

private:
    void updateEvents();

    QLabel* mLabel;
    QProgressBar* mBar;
    QPlainTextEdit* mLog;
    bool mCanceled;
};

//-------------------------------------------------------------------------------------------------
class LoggableProgressReporter
        : public util::IProgressReporter
        , public ctrl::UILogger
{
public:
    LoggableProgressReporter(bool aCancelable, QWidget* aParent);

    void cancel();

    // IProgressReporter
    virtual void setSection(const QString& aSection);
    virtual void setMaximum(int aMax);
    virtual void setProgress(int aValue);
    virtual bool wasCanceled() const;

    // UILogger
    virtual void pushLog(const QString& aMessage, ctrl::UILogType aType);

private:
    LoggableProgressDialog mDialog;
};

} // namespace menu
} // namespace gui

#endif // GUI_MENU_PROGRESSREPORTER_H
