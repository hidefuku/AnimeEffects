#ifndef GUI_EXPORTDIALOG_H
#define GUI_EXPORTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QComboBox>
#include <QLabel>
#include "core/Project.h"
#include "ctrl/Exporter.h"
#include "ctrl/VideoFormat.h"
#include "gui/EasyDialog.h"

namespace gui
{

//-------------------------------------------------------------------------------------------------
class ExportDialog : public EasyDialog
{
    Q_OBJECT
public:
    ExportDialog(core::Project& aProject, const QString& aPath, QWidget* aParent);
    const ctrl::Exporter::CommonParam& commonParam() const { return mCommonParam; }
    ctrl::Exporter::CommonParam& commonParam() { return mCommonParam; }

protected:
    void pushSizeBox(QFormLayout& aLayout);
    void pushFrameBox(QFormLayout& aLayout);
    void pushFpsBox(QFormLayout& aLayout);

private:
    core::Project& mProject;
    ctrl::Exporter::CommonParam mCommonParam;
    QSize mSize;
    int mFrameMax;
    bool mFixAspect;
    bool mSizeUpdating;
};

//-------------------------------------------------------------------------------------------------
class ImageExportDialog : public ExportDialog
{
    Q_OBJECT
public:
    ImageExportDialog(
            core::Project& aProject, const QString& aDirPath,
            const QString& aSuffix, QWidget* aParent);
    const ctrl::Exporter::ImageParam& imageParam() const { return mImageParam; }

private:
    QLayout* createImageOption();

    ctrl::Exporter::ImageParam mImageParam;
};

//-------------------------------------------------------------------------------------------------
class GifExportDialog : public ExportDialog
{
    Q_OBJECT
public:
    GifExportDialog(
            core::Project& aProject, const QString& aFilePath,
            QWidget* aParent);
    const ctrl::Exporter::GifParam& gifParam() const { return mGifParam; }

private:
    QLayout* createGifOption();

    ctrl::Exporter::GifParam mGifParam;
};

//-------------------------------------------------------------------------------------------------
class VideoExportDialog : public ExportDialog
{
    Q_OBJECT
public:
    VideoExportDialog(
            core::Project& aProject, const QString& aFilePath,
            const ctrl::VideoFormat& aFormat, QWidget* aParent);
    const ctrl::Exporter::VideoParam& videoParam() const { return mVideoParam; }

private:
    QLayout* createVideoOption();
    void setColorspaceValidity(QComboBox* aBox, bool aIsValid);
    void updatePixelFormat(QComboBox* aBox, const QStringList& aPixfmts);
    void updateCommentLabel(QLabel* aLabel, bool aGPUEnc);

    ctrl::Exporter::VideoParam mVideoParam;
};

} // namespace gui

#endif // GUI_EXPORTDIALOG_H
