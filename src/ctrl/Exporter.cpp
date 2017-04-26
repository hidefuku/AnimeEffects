#include <QFileInfo>
#include <QBuffer>
#include "util/SelectArgs.h"
#include "gl/Global.h"
#include "gl/Util.h"
#include "ctrl/Exporter.h"

namespace ctrl
{
//-------------------------------------------------------------------------------------------------
Exporter::Result::Result()
    : code(ResultCode_TERM)
{
}

Exporter::Result::Result(ResultCode aCode, const QString& aMessage)
    : code(aCode)
    , message(aMessage)
{
}

//-------------------------------------------------------------------------------------------------
Exporter::CommonParam::CommonParam()
    : path()
    , size()
    , frame()
    , fps()
{
}

bool Exporter::CommonParam::isValid() const
{
    if (frame.diff() < 0)
    {
        return false;
    }

    if (fps <= 0)
    {
        return false;
    }

    if (size.width() <= 0 || size.height() <= 0)
    {
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
Exporter::ImageParam::ImageParam()
    : name()
    , suffix("png")
    , quality(-1)
{
}

//-------------------------------------------------------------------------------------------------
Exporter::GifParam::GifParam()
    : optimizePalette()
    , intermediateBps()
{
}

//-------------------------------------------------------------------------------------------------
Exporter::VideoParam::VideoParam()
    : format()
    , codecIndex(-1)
    , colorIndex()
    , bps()
    , pixfmt()
{
}

//-------------------------------------------------------------------------------------------------
Exporter::FFMpeg::FFMpeg()
    : mProcess()
    , mFinished()
    , mErrorOccurred()
    , mErrorString()
    , mErrorCode()
    , mLogs()
{
}

bool Exporter::FFMpeg::start(const QString& aArgments)
{
#if defined(Q_OS_WIN)
    const QFileInfo localEncoderInfo("./tools/ffmpeg.exe");
    const bool hasLocalEncoder = localEncoderInfo.exists() && localEncoderInfo.isExecutable();
    const QString program = hasLocalEncoder ? QString(".\\tools\\ffmpeg") : QString("ffmpeg");
#else
    const QFileInfo localEncoderInfo("./tools/ffmpeg");
    const bool hasLocalEncoder = localEncoderInfo.exists() && localEncoderInfo.isExecutable();
    const QString program = hasLocalEncoder ? QString("./tools/ffmpeg") : QString("ffmpeg");
#endif

    mFinished = false;
    mErrorOccurred = false;
    mErrorString.clear();
    mProcess.reset(new QProcess(nullptr));
    mProcess->setProcessChannelMode(QProcess::MergedChannels);

    auto process = mProcess.data();
    mProcess->connect(process, &QProcess::readyRead, [=]()
    {
        if (this->mProcess)
        {
            this->mLogs.push_back(QString(this->mProcess->readAll().data()));
            //qDebug() << QString(process->readAll().data());
        }
    });
    mProcess->connect(process, &QProcess::errorOccurred, [=](QProcess::ProcessError aCode)
    {
        this->mErrorOccurred = true;
        this->mErrorCode = aCode;
        if (this->mProcess)
        {
            this->mErrorString = this->mProcess->errorString();
        }
    });
    mProcess->connect(process, util::SelectArgs<int, QProcess::ExitStatus>::from(&QProcess::finished), [=](int, QProcess::ExitStatus)
    {
        this->mFinished = true;
    });

    mProcess->start(program + " " + aArgments, QIODevice::ReadWrite);

    return !mErrorOccurred;
}

void Exporter::FFMpeg::write(const QByteArray& aBytes)
{
    XC_ASSERT(mProcess);
    mProcess->write(aBytes);
}

bool Exporter::FFMpeg::finish(const std::function<bool()>& aWaiter)
{
    static const int kMSec = 100;
    XC_ASSERT(mProcess);

    mProcess->closeWriteChannel();

    while (!mProcess->waitForFinished(kMSec))
    {
        if (!aWaiter() || mFinished || mErrorOccurred) break;
    }

    auto exitStatus = mProcess->exitStatus();
    //qDebug() << "exit status" << exitStatus;
    //qDebug() << "exit code" << mProcess->exitCode();

    mProcess.reset();
    return (exitStatus == QProcess::NormalExit);
}

bool Exporter::FFMpeg::execute(const QString& aArgments,
                               const std::function<bool()>& aWaiter)
{
    if (!start(aArgments)) return false;
    if (!finish(aWaiter)) return false;
    return true;
}

QString Exporter::FFMpeg::popLog()
{
    QString log = mLogs.join("");
    mLogs.clear();
    return log;
}

//-------------------------------------------------------------------------------------------------
Exporter::Exporter(core::Project& aProject)
    : mProject(aProject)
    , mFramebuffers()
    , mClippingFrame()
    , mDestinationTexturizer()
    , mTextureDrawer()
    , mOriginTimeInfo()
    , mOverwriteConfirmer()
    , mOverwriteConfirmation()
    , mProgressReporter()
    , mUILogger()
    , mCommonParam()
    , mImageParam()
    , mVideoInCodec()
    , mVideoInCodecQuality()
    , mVideoExporting()
    , mFFMpeg()
    , mExporting(false)
    , mIndex(0)
    , mDigitCount(0)
    , mProgress(0.0f)
    , mLog()
    , mIsCanceled()
{
}

Exporter::~Exporter()
{
    finish();

    // kill buffer
    gl::Global::makeCurrent();
    destroyFramebuffers();
}

void Exporter::setOverwriteConfirmer(const OverwriteConfirmer& aConfirmer)
{
    mOverwriteConfirmer = aConfirmer;
}

void Exporter::setProgressReporter(util::IProgressReporter& aReporter)
{
    mProgressReporter = &aReporter;
}

void Exporter::setUILogger(UILogger& aLogger)
{
    mUILogger = &aLogger;
}

Exporter::Result Exporter::execute(const CommonParam& aCommon, const ImageParam& aImage)
{
    // check param
    if (!aCommon.isValid())
    {
        mLog = "Invalid common parameters.";
        return Result(ResultCode_InvalidOperation, mLog);
    }

    // check directory
    QFileInfo path(aCommon.path);
    if (!path.exists() || !path.isDir())
    {
        mLog = "Invalid directory path.";
        return Result(ResultCode_InvalidOperation, mLog);
    }

    mCommonParam = aCommon;
    mImageParam = aImage;
    mVideoExporting = false;
    mOriginTimeInfo = mProject.currentTimeInfo();
    mOverwriteConfirmation = false;
    mLog.clear();
    mIsCanceled = false;

    return execute();
}

Exporter::Result Exporter::execute(const CommonParam& aCommon, const GifParam& aGif)
{
    const QString outFile = QFileInfo(aCommon.path).absoluteFilePath();
    const QString workFile = outFile + "videocache.mp4";
    const QString palette = outFile + "palettecache.png";

    CommonParam commonParam = aCommon;
    VideoParam videoParam;

    if (aGif.optimizePalette)
    {
        commonParam.path = workFile;
        videoParam.bps = aGif.intermediateBps;
    }
    else
    {
        videoParam.bps = 0;
    }

    {
        auto result = execute(commonParam, videoParam);
        if (!result) return result;
    }

    if (aGif.optimizePalette)
    {

        auto waiter = [=]()->bool { return true; };
        if (mFFMpeg.execute(" -i " + workFile + " -vf palettegen -y " + palette, waiter))
        {
            mFFMpeg.execute(" -i " + workFile + " -i " + palette + " -lavfi paletteuse -y " + outFile, waiter);
        }

        QFile::remove(workFile);
        QFile::remove(palette);

        if (mFFMpeg.errorOccurred())
        {
            mLog = "FFmpeg error occurred.\n" + mFFMpeg.errorString();
            return Result(ResultCode_FFMpegError, mLog);
        }
    }
    return Result(ResultCode_Success, "Success.");
}

Exporter::Result Exporter::execute(const CommonParam& aCommon, const VideoParam& aVideo)
{
    // check param
    if (!aCommon.isValid())
    {
        mLog = "Invalid common parameters.";
        return Result(ResultCode_InvalidOperation, mLog);
    }

    // check file overwriting
    mOverwriteConfirmation = false;
    QFileInfo filePath(aCommon.path);
    if (!checkOverwriting(filePath))
    {
        mLog = "Exporting was canceled.";
        mIsCanceled = true;
        return Result(ResultCode_Canceled, mLog);
    }

    mCommonParam = aCommon;
    mVideoExporting = true;
    mOriginTimeInfo = mProject.currentTimeInfo();
    mLog.clear();
    mIsCanceled = false;

#if 0
    {
        const QString out = " \"" + filePath.absoluteFilePath() + "\"";
        //const QString ifps = " -r " + QString::number(mCommonParam.fps);
        //const QString ofps = ifps;
        const QString ifps = " -framerate " + QString::number(mCommonParam.fps);
        const QString ofps = " -r " + QString::number(mCommonParam.fps);
        const QString icodec = " -vcodec png";
        //const QString icodec = " -vcodec ppm";
        const QString ocodec =
                !aVideo.codec.isEmpty() ? " -vcodec " + aVideo.codec : "";
        const QString obps =
                aVideo.bps > 0 ? " -b:v " + QString::number(aVideo.bps) : "";
        const QString thrs = ""; //" -threads 8";

        QString argments =
                QString(" -y") + /// overwrite files without asking
                " -f image2pipe" +
                ifps + icodec + " -i -" +
                obps + ofps + ocodec + thrs + out;

        if (!mFFMpeg.start(argments))
        {
            mLog = "FFmpeg error occurred.\n" + mFFMpeg.errorString();
            return mFFMpeg.errorCode() == QProcess::FailedToStart ?
                        Result(ResultCode_FFMpegFailedToStart, mLog) :
                        Result(ResultCode_FFMpegError, mLog);
        }
    }
#else
    {
        const QString outPath = "\"" + filePath.absoluteFilePath() + "\"";

        VideoCodec videoCodec;
        if (aVideo.codecIndex != -1)
        {
            videoCodec = aVideo.format.codecs.at(aVideo.codecIndex);
        }
        else
        {
            videoCodec.icodec = aVideo.format.icodec;
        }
        auto colorIndex = videoCodec.colorspace ? aVideo.colorIndex : 0;

        mVideoInCodecQuality = -1;
        if (videoCodec.icodec == "png")
        {
            mVideoInCodec = "PNG";
            mVideoInCodecQuality = 90;
        }
        else if (videoCodec.icodec == "ppm")
        {
            mVideoInCodec = "PPM";
        }
        else if (videoCodec.icodec == "jpg")
        {
            mVideoInCodec = "JPG";
        }
        else if (videoCodec.icodec == "jpeg")
        {
            mVideoInCodec = "JPEG";
        }
        else if (videoCodec.icodec == "bmp")
        {
            mVideoInCodec = "BMP";
        }
        else
        {
            videoCodec.icodec = "png";
            mVideoInCodec = "PNG";
            mVideoInCodecQuality = 90;
        }

        if (videoCodec.command.isEmpty())
        {
            videoCodec.command = "-y -f image2pipe -framerate $ifps -vcodec $icodec -i - -b:v $obps -r $ofps $opath";
        }
        videoCodec.command.replace(QRegExp("\\$ifps(\\s|$)"), QString::number(mCommonParam.fps) + "\\1");
        videoCodec.command.replace(QRegExp("\\$icodec(\\s|$)"), videoCodec.icodec + "\\1");
        videoCodec.command.replace(QRegExp("\\$obps(\\s|$)"), QString::number(aVideo.bps) + "\\1");
        videoCodec.command.replace(QRegExp("\\$ofps(\\s|$)"), QString::number(mCommonParam.fps) + "\\1");
        videoCodec.command.replace(QRegExp("\\$ocodec(\\s|$)"), videoCodec.name + "\\1");
        videoCodec.command.replace(QRegExp("\\$opath(\\s|$)"), outPath + "\\1");
        videoCodec.command.replace(QRegExp("\\$pixfmt(\\s|$)"), aVideo.pixfmt + "\\1");
        videoCodec.command.replace(QRegExp("\\$arg_colorfilter(\\s|$)"), (colorIndex == 0 ? QString("-vf colormatrix=bt601:bt709") : QString("")) + "\\1");
        videoCodec.command.replace(QRegExp("\\$arg_colorspace(\\s|$)"), QString("-colorspace ") + (colorIndex == 0 ? QString("bt709") : QString("smpte170m")) + "\\1");

        qDebug() << videoCodec.command;

        if (!mFFMpeg.start(videoCodec.command))
        {
            mLog = "FFmpeg error occurred.\n" + mFFMpeg.errorString();
            return mFFMpeg.errorCode() == QProcess::FailedToStart ?
                        Result(ResultCode_FFMpegFailedToStart, mLog) :
                        Result(ResultCode_FFMpegError, mLog);
        }
    }
#endif

    return execute();
}

Exporter::Result Exporter::execute()
{
    if (mProgressReporter)
    {
        mProgressReporter->setSection("Exporting...");
        mProgressReporter->setMaximum(100);
    }

    // start
    {
        auto result = start();
        if (!result) return result;
    }

    while (1)
    {
        if (!update()) break;

        if (mProgressReporter)
        {
            mProgressReporter->setProgress((int)(100 * mProgress));

            if (mProgressReporter->wasCanceled())
            {
                finish();
                mLog = "Export was canceled.";
                mIsCanceled = true;
                return Result(ResultCode_Canceled, mLog);
            }
        }
    }

    if (mFFMpeg.errorOccurred())
    {
        mLog = "FFmpeg error occurred.\n" + mFFMpeg.errorString();
        return mFFMpeg.errorCode() == QProcess::FailedToStart ?
                    Result(ResultCode_FFMpegFailedToStart, mLog) :
                    Result(ResultCode_FFMpegError, mLog);
    }

    return finish();
}

Exporter::Result Exporter::start()
{
    // reset value
    mIndex = 0;
    mProgress = 0.0f;
    mDigitCount = getDigitCount(
                mCommonParam.frame,
                mCommonParam.fps,
                mOriginTimeInfo.fps);

    // initialize graphics
    {
        gl::Global::makeCurrent();

        // texture drawer
        if (!mTextureDrawer.init())
        {
            mLog = "Failed to initialize TextureDrawer";
            Result(ResultCode_UnclassfiedError, mLog);
        }

        // framebuffers
        createFramebuffers(mProject.attribute().imageSize(), mCommonParam.size);

        // clipping frame
        mClippingFrame.reset(new core::ClippingFrame());
        mClippingFrame->resize(mProject.attribute().imageSize());

        // create texturizer for destination colors of the framebuffer
        mDestinationTexturizer.reset(new core::DestinationTexturizer());
        mDestinationTexturizer->resize(mProject.attribute().imageSize());
    }

    mExporting = true;
    return Result(ResultCode_Success, "Success.");
}

bool Exporter::updateTime(core::TimeInfo& aDst)
{
    aDst = mOriginTimeInfo;

    const int range = mCommonParam.frame.diff() + 1;
    const double current = (mIndex * mOriginTimeInfo.fps) / (double)mCommonParam.fps;
    const double frame = mCommonParam.frame.min() + current;

    // end of export
    if (0 < mIndex && mCommonParam.frame.max() < frame)
    {
        return false;
    }
    if (mOriginTimeInfo.frameMax < (int)frame)
    {
        return false;
    }

    aDst.frame = core::Frame::fromDecimal(frame);
    mProgress = (float)current / range;

    // to next index
    ++mIndex;

    return true;
}

bool Exporter::update()
{
    if (!mExporting)
    {
        return false;
    }

    const int currentIndex = mIndex;

    // setup parameter
    core::TimeInfo timeInfo;
    if (!updateTime(timeInfo))
    {
        return false;
    }

    // begin rendering
    gl::Global::makeCurrent();
    gl::Global::Functions& ggl = gl::Global::functions();
    const QSize originSize = mProject.attribute().imageSize();

    // clear clipping
    mClippingFrame->clearTexture();
    mClippingFrame->resetClippingId();

    // clear destination texture
    mDestinationTexturizer->clearTexture();

    // bind framebuffer
    if (!mFramebuffers.front()->bind())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to bind framebuffer.", "");
    }

    // setup
    gl::Util::setViewportAsActualPixels(originSize);
    gl::Util::clearColorBuffer(0.0, 0.0, 0.0, 0.0);
    gl::Util::resetRenderState();

    // render
    core::RenderInfo renderInfo;
    renderInfo.camera.reset(originSize, 1.0, originSize, QPoint());
    renderInfo.time = timeInfo;
    renderInfo.framebuffer = mFramebuffers.front()->handle();
    renderInfo.dest = mFramebuffers.front()->texture();
    renderInfo.isGrid = false;
    renderInfo.clippingId = 0;
    renderInfo.clippingFrame = mClippingFrame.data();
    renderInfo.destTexturizer = mDestinationTexturizer.data();

    XC_ASSERT(renderInfo.framebuffer != 0);
    XC_ASSERT(renderInfo.dest != 0);
    mProject.objectTree().render(renderInfo, true);

    // unbind framebuffer
    if (!mFramebuffers.front()->release())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to bind framebuffer.", "");
    }

    // scaling
    {
        QOpenGLFramebufferObject* prev = nullptr;
        for (auto& fbo : mFramebuffers)
        {
            if (prev)
            {
                fbo->bind();

                const QSize size = fbo->size();
                gl::Util::setViewportAsActualPixels(size);
                gl::Util::clearColorBuffer(0.0, 0.0, 0.0, 0.0);

                mTextureDrawer.draw(prev->texture());

                fbo->release();
            }
            prev = fbo.get();
        }
    }

    {
        // create image
        auto outImage = mFramebuffers.back()->toImage();

        // flush
        ggl.glFlush();

        // update log if necessary
        updateLog();

        // export
        if (!exportImage(outImage, currentIndex))
        {
            return false;
        }
    }

    return true;
}

bool Exporter::exportImage(const QImage& aFboImage, int aIndex)
{
    // decide file path
    QFileInfo filePath;
    if (!decideImagePath(aIndex, filePath))
    {
        return false;
    }

    if (mVideoExporting)
    {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::ReadWrite);
        aFboImage.save(&buffer, mVideoInCodec, mVideoInCodecQuality);
        //aFboImage.save(&buffer, "PPM");
        buffer.close();
        mFFMpeg.write(byteArray);

        if (mFFMpeg.errorOccurred())
        {
            mLog = "FFmpeg error occurred.\n" + mFFMpeg.errorString();
            return false;
        }
    }
    else
    {
        //QImage image(aFboImage.constBits(), aFboImage.width(),
        //             aFboImage.height(), QImage::Format_ARGB32);
        //image.save(aFilePath);
        aFboImage.save(filePath.filePath(), Q_NULLPTR, mImageParam.quality);
    }

    return true;
}

Exporter::Result Exporter::finish()
{
    Result result(ResultCode_Success, "Success.");

    if (mExporting)
    {
        if (mVideoExporting)
        {
            auto success = mFFMpeg.finish([=]()->bool
            {
                // update log if necessary
                this->updateLog();
                return true;
            });
            if (!success)
            {
                mLog = "FFmpeg error occurred.\n" + mFFMpeg.errorString();
                result = Result(ResultCode_FFMpegError, mLog);
            }
        }

        mExporting = false;
    }
    return result;
}

void Exporter::destroyFramebuffers()
{
    for (auto& fbo : mFramebuffers)
    {
        fbo.reset();
    }
}

void Exporter::createFramebuffers(const QSize& aOriginSize, const QSize& aExportSize)
{
    destroyFramebuffers();

    mFramebuffers.emplace_back(
                FramebufferPtr(new QOpenGLFramebufferObject(aOriginSize)));

    // setup buffers for scaling
    if (aOriginSize != aExportSize)
    {
        static const int kMaxCount = 3;
        QSize size = aOriginSize;

        for (int i = 0; i < kMaxCount; ++i)
        {
            const double scaleX = aExportSize.width() / (double)size.width();
            const double scaleY = aExportSize.height() / (double)size.height();
            const double scaleMax = std::max(scaleX, scaleY);

            if (scaleMax >= 0.5 || i == kMaxCount - 1)
            {
                mFramebuffers.emplace_back(
                            FramebufferPtr(new QOpenGLFramebufferObject(aExportSize)));
                break;
            }
            else
            {
                size.setWidth((int)(size.width() * 0.5));
                size.setHeight((int)(size.height() * 0.5));
                mFramebuffers.emplace_back(
                            FramebufferPtr(new QOpenGLFramebufferObject(size)));
            }
        }
    }

    // setup texture
    for (auto& fbo : mFramebuffers)
    {
        setTextureParam(*fbo);
    }
}

void Exporter::setTextureParam(QOpenGLFramebufferObject& aFbo)
{
    auto id = aFbo.texture();
    gl::Global::Functions& ggl = gl::Global::functions();
    ggl.glBindTexture(GL_TEXTURE_2D, id);
    ggl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    ggl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    ggl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    ggl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    ggl.glBindTexture(GL_TEXTURE_2D, 0);
}

bool Exporter::decideImagePath(int aIndex, QFileInfo& aPath)
{
    const QString number = QString("%1").arg(aIndex, mDigitCount, 10, QChar('0'));
    QFileInfo filePath(mCommonParam.path + "/" + mImageParam.name +
                       number + "." + mImageParam.suffix);

    // check overwrite
    if (!checkOverwriting(filePath))
    {
        return false;
    }

    aPath = filePath;
    return true;
}

bool Exporter::checkOverwriting(const QFileInfo& aPath)
{
    // check overwrite
    if (!mOverwriteConfirmation && aPath.exists())
    {
        if (aPath.isDir() ||
                !mOverwriteConfirmer ||
                !mOverwriteConfirmer(aPath.filePath()))
        {
            return false;
        }
        mOverwriteConfirmation = true;
    }
    return true;
}

int Exporter::getDigitCount(const util::Range& aRange, int aFps, int aFpsOrigin)
{
    int digitCount = 1;
    double count = (aFps * (aRange.diff() + 1)) / (double)aFpsOrigin;
    for (; count >= 10.0; count *= 0.1) ++digitCount;
    return std::max(digitCount, 4);
}

void Exporter::updateLog()
{
    if (mUILogger && mVideoExporting)
    {
        auto log = mFFMpeg.popLog();
        if (!log.isEmpty())
        {
            mUILogger->pushLog(log, ctrl::UILogType_Info);
        }
    }
}

} // namespace ctrl
