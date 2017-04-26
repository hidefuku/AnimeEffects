#include "ctrl/VideoFormat.h"

namespace ctrl
{

VideoCodec::VideoCodec()
    : name()
    , label()
    , icodec()
    , command()
    , pixfmts()
    , lossless()
    , transparent()
    , colorspace()
    , gpuenc()
{
}

VideoFormat::VideoFormat()
    : name()
    , label()
    , icodec()
    , command()
    , codecs()
{
}

} // namespace ctrl
