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
#include "ctrl/UILogger.h"
#include "gl/EasyTextureDrawer.h"
#include "core/Project.h"
#include "core/TimeInfo.h"
#include "core/TimeKeyBlender.h"
#include "core/ClippingFrame.h"
#include "core/DestinationTexturizer.h"

namespace ctrl
{

class Exporter
{
public:
    typedef std::function<bool(const QString&)> OverwriteConfirmer;

    enum ResultCode
    {
        ResultCode_Success,
        ResultCode_Canceled,
        ResultCode_InvalidOperation,
        ResultCode_FFMpegFailedToStart,
        ResultCode_FFMpegError,
        ResultCode_UnclassfiedError,
        ResultCode_TERM
    };

    struct Result
    {
        Result();
        Result(ResultCode aCode, const QString& aMessage);
        explicit operator bool() const { return code == ResultCode_Success; }
        ResultCode code;
        QString message;
    };

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
        QString codec;
        int bps;
    };

    struct GifParam
    {
        GifParam();
        bool optimizePalette;
        int intermediateBps;
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
    void setUILogger(ctrl::UILogger& aLogger);

    Result execute(const CommonParam& aCommon, const PngParam& aPng);
    Result execute(const CommonParam& aCommon, const GifParam& aGif);
    Result execute(const CommonParam& aCommon, const VideoParam& aVideo);

    const QString& log() const { return mLog; }
    bool isCanceled() const { return mIsCanceled; }

private:
    typedef std::unique_ptr<QOpenGLFramebufferObject> FramebufferPtr;
    typedef std::list<FramebufferPtr> FramebufferList;

    class FFMpeg
    {
    public:
        FFMpeg();
        bool start(const QString& aArgments);
        void write(const QByteArray& aBytes);
        bool finish(const std::function<bool()>& aWaiter);
        bool execute(const QString& aArgments,
                     const std::function<bool()>& aWaiter);
        bool errorOccurred() const { return mErrorOccurred; }
        QString errorString() const { return mErrorString; }
        QProcess::ProcessError errorCode() const { return mErrorCode; }
        QString popLog();
    private:
        QScopedPointer<QProcess> mProcess;
        bool mFinished;
        bool mErrorOccurred;
        QString mErrorString;
        QProcess::ProcessError mErrorCode;
        QStringList mLogs;
    };

    Result execute();
    Result start();
    bool update();
    Result finish();
    bool updateTime(core::TimeInfo& aDst);
    bool exportImage(const QImage& aFboImage, int aIndex);
    void destroyFramebuffers();
    void createFramebuffers(const QSize& aOriginSize, const QSize& aExportSize);
    void setTextureParam(QOpenGLFramebufferObject& aFbo);
    static int getDigitCount(const util::Range& aRange, int aFps, int aFpsOrigin);
    bool decidePngPath(int aIndex, QFileInfo& aPath);
    bool checkOverwriting(const QFileInfo& aPath);
    void updateLog();

    core::Project& mProject;
    FramebufferList mFramebuffers;
    QScopedPointer<core::ClippingFrame> mClippingFrame;
    QScopedPointer<core::DestinationTexturizer> mDestinationTexturizer;
    gl::EasyTextureDrawer mTextureDrawer;
    core::TimeInfo mOriginTimeInfo;
    OverwriteConfirmer mOverwriteConfirmer;
    bool mOverwriteConfirmation;
    util::IProgressReporter* mProgressReporter;
    ctrl::UILogger* mUILogger;

    CommonParam mCommonParam;
    QString mPngName;
    bool mVideoExporting;

    FFMpeg mFFMpeg;
    bool mExporting;
    int mIndex;
    int mDigitCount;
    float mProgress;
    QString mLog;
    bool mIsCanceled;
};

} // namespace ctrl

#endif // CTRL_EXPORTER_H
