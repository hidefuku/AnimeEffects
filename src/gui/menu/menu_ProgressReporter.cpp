#include <QApplication>
#include "gui/menu/menu_ProgressReporter.h"

namespace gui {
namespace menu {

//-------------------------------------------------------------------------------------------------
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

void ProgressReporter::cancel()
{
    mDialog.cancel();
}

void ProgressReporter::setSection(const QString& aSection)
{
    mDialog.setLabelText(aSection);
}

void ProgressReporter::setMaximum(int aMax)
{
    mDialog.setMaximum(aMax);
}

void ProgressReporter::setProgress(int aValue)
{
    mDialog.setValue(aValue);
}

bool ProgressReporter::wasCanceled() const
{
    return mDialog.wasCanceled();
}

//-------------------------------------------------------------------------------------------------
LoggableProgressDialog::LoggableProgressDialog(bool aCancelable, QWidget* aParent)
    : EasyDialog("Operation in Progress...", aParent)
    , mLabel()
    , mBar()
    , mLog()
    , mCanceled()
{
    this->setWindowModality(Qt::WindowModal);

    mLabel = new QLabel("");
    mBar = new QProgressBar();
    mBar->setMinimum(0);
    mBar->setMaximum(100);
    mLog = new QPlainTextEdit();
    mLog->setUndoRedoEnabled(false);
    mLog->setMaximumBlockCount(32);
    mLog->setReadOnly(true);

    {
        auto layout = new QVBoxLayout();
        layout->addWidget(mLabel);
        layout->addWidget(mBar);
        layout->addWidget(mLog);
        layout->setAlignment(mLabel, Qt::AlignHCenter | Qt::AlignVCenter);
        this->setMainLayout(layout, false);
    }

    if (aCancelable)
    {
        this->setCancel([=](int)->bool { mCanceled = true; return true; });
    }
}

void LoggableProgressDialog::cancel()
{
    mCanceled = true;
    this->hide();
}

void LoggableProgressDialog::setLabelText(const QString& aText)
{
    mLabel->setText(aText);
}

void LoggableProgressDialog::setMaximum(int aMax)
{
    mBar->setMaximum(aMax);
}

void LoggableProgressDialog::setValue(int aValue)
{
    mBar->setValue(aValue);
    updateEvents();
}

void LoggableProgressDialog::setLog(const QString& aMessage)
{
    mLog->appendPlainText(aMessage);
    updateEvents();
}

void LoggableProgressDialog::updateEvents()
{
    QApplication::processEvents();

#ifdef Q_OS_MAC
    QApplication::flush();
#endif
}

bool LoggableProgressDialog::wasCanceled() const
{
    return mCanceled;
}

//-------------------------------------------------------------------------------------------------
LoggableProgressReporter::LoggableProgressReporter(bool aCancelable, QWidget* aParent)
    : mDialog(aCancelable, aParent)
{
    mDialog.show();
}

void LoggableProgressReporter::cancel()
{
    mDialog.cancel();
}

void LoggableProgressReporter::setSection(const QString& aSection)
{
    mDialog.setLabelText(aSection);
}

void LoggableProgressReporter::setMaximum(int aMax)
{
    mDialog.setMaximum(aMax);
}

void LoggableProgressReporter::setProgress(int aValue)
{
    mDialog.setValue(aValue);
}

bool LoggableProgressReporter::wasCanceled() const
{
    return mDialog.wasCanceled();
}

void LoggableProgressReporter::pushLog(const QString& aMessage, ctrl::UILogType)
{
    mDialog.setLog(aMessage);
}

} // namespace menu
} // namespace gui
