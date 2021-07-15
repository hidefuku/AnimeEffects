#ifndef CTRL_VIDEOFORMAT_H
#define CTRL_VIDEOFORMAT_H

#include <QString>
#include <QList>

namespace ctrl
{

class VideoCodec
{
public:
    VideoCodec();
    QString name;
    QString label;
    QString icodec;
    QStringList command;
    QStringList pixfmts;
    bool lossless;
    bool transparent;
    bool colorspace;
    bool gpuenc;
};

class VideoFormat
{
public:
    VideoFormat();
    QString name;
    QString label;
    QString icodec;
    QStringList command;
    QList<VideoCodec> codecs;
};

} // namespace ctrl

#endif // CTRL_VIDEOFORMAT_H
