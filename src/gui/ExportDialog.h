#ifndef GUI_EXPORTDIALOG_H
#define GUI_EXPORTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include "core/Project.h"
#include "ctrl/Exporter.h"
#include "gui/EasyDialog.h"

namespace gui
{

class ExportDialog : public EasyDialog
{
    Q_OBJECT
public:
    ExportDialog(
            core::Project& aProject,
            const QString& aPath,
            bool aVideoExporting,
            QWidget* aParent);
    const ctrl::Exporter::CommonParam& commonParam() const { return mCommonParam; }
    const ctrl::Exporter::VideoParam& videoParam() const { return mVideoParam; }
    const ctrl::Exporter::PngParam& pngParam() const { return mPngParam; }

private:
    void initializeParameter(const QString& aPath);
    QLayout* createVideoOption();
    QLayout* createPngOption();
    void pushSizeBox(QFormLayout& aLayout);
    void pushFrameBox(QFormLayout& aLayout);
    void pushFpsBox(QFormLayout& aLayout);

    core::Project& mProject;
    ctrl::Exporter::CommonParam mCommonParam;
    ctrl::Exporter::VideoParam mVideoParam;
    ctrl::Exporter::PngParam mPngParam;
    bool mVideoExporting;
    QSize mSize;
    int mFrameMax;
    bool mFixAspect;
    bool mSizeUpdating;
};

} // namespace gui

#endif // GUI_EXPORTDIALOG_H
