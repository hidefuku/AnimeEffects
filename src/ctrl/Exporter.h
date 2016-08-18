#ifndef CTRL_EXPORTER_H
#define CTRL_EXPORTER_H

#include <list>
#include <memory>
#include <functional>
#include <QString>
#include <QSize>
#include <QFileInfo>
#include <QProcess>
#include <QOpenGLFramebufferObject>
#include "util/Range.h"
#include "util/IProgressReporter.h"
#include "gl/EasyTextureDrawer.h"
#include "core/Project.h"
#include "core/TimeInfo.h"
#include "core/TimeKeyBlender.h"
#include "core/ClippingFrame.h"

namespace ctrl
{

class Exporter
{
public:
    typedef std::function<bool(const QString&)> OverwriteConfirmer;

    struct CommonParam
    {
        CommonParam();
        QString path;
        QSize size;
        util::Range frame;
        int fps;
        bool isValid() const;
    };

    struct VideoParam
    {
        VideoParam();
        QString format;
        QString codec;
        int bps;
    };

    struct PngParam
    {
        PngParam();
        QString name;
    };

    Exporter(core::Project& aProject);
    ~Exporter();

    void setOverwriteConfirmer(const OverwriteConfirmer& aConfirmer);
    void setProgressReporter(util::IProgressReporter& aReporter);

    bool execute(const CommonParam& aCommon, const PngParam& aPng);
    bool execute(const CommonParam& aCommon, const VideoParam& aVideo);

private:
    typedef std::unique_ptr<QOpenGLFramebufferObject> FramebufferPtr;
    typedef std::list<FramebufferPtr> FramebufferList;

    bool execute();
    bool start();
    bool update();
    bool finish();
    bool updateTime(core::TimeInfo& aDst);
    bool exportImage(const QImage& aFboImage, int aIndex);
    void destroyFramebuffers();
    void createFramebuffers(const QSize& aOriginSize, const QSize& aExportSize);
    void setTextureParam(QOpenGLFramebufferObject& aFbo);
    static int getDigitCount(const util::Range& aRange, int aFps, int aFpsOrigin);
    bool decidePngPath(int aIndex, QFileInfo& aPath);
    bool checkOverwriting(const QFileInfo& aPath);

    core::Project& mProject;
    FramebufferList mFramebuffers;
    QScopedPointer<core::ClippingFrame> mClippingFrame;
    gl::EasyTextureDrawer mTextureDrawer;
    core::TimeInfo mOriginTimeInfo;
    OverwriteConfirmer mOverwriteConfirmer;
    bool mOverwriteConfirmation;
    util::IProgressReporter* mProgressReporter;

    CommonParam mCommonParam;
    PngParam mPngParam;
    VideoParam mVideoParam;
    bool mVideoExporting;
    QScopedPointer<QProcess> mFFMpeg;

    bool mExporting;
    int mIndex;
    int mDigitCount;
    float mProgress;
};

} // namespace ctrl

#endif // CTRL_EXPORTER_H
