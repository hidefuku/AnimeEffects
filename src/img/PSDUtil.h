#ifndef IMG_PSDUTIL_H
#define IMG_PSDUTIL_H

#include <memory>
#include <QRect>
#include "XC.h"
#include "img/PSDFormat.h"

namespace img
{

class PSDUtil
{
public:

    enum ColorFormat
    {
        ColorFormat_RGB8,
        ColorFormat_RGBA8
    };

    static bool blendImage(
            uint8* aResult, const uint8* aBack, const QRect& aRectRB,
            const uint8* aFront, const QRect& aRectF,
            const std::string& aMode);

    static XCMemBlock makeClippedImage(
            const uint8* aTarget, const QRect& aRectT,
            const uint8* aBase, const QRect& aRectB);

    static size_t encodePackBits(const uint8* aSrc, uint8* aDst, size_t aLength);

    static XCMemBlock encodePlanePackBits(
            const uint8* aSrc, size_t aSrcLength,
            int aWidth, int aHeight, int aSrcStride);

    static bool makeChanneledImage(
            PSDFormat::Layer& aDst,
            const PSDFormat::Header& aHeader,
            const XCMemBlock& aSrc,
            ColorFormat aSrcFormat);

    static bool makeChanneledImage(
            PSDFormat::ImageData& aDst,
            const PSDFormat::Header& aHeader,
            const XCMemBlock& aSrc,
            ColorFormat aSrcFormat);

    static bool makeChanneledImage(
            PSDFormat::ChannelList& aDst,
            const PSDFormat::Header& aHeader,
            const XCMemBlock& aSrc,
            ColorFormat aSrcFormat,
            int aSrcWidth, int aSrcHeight);

    static bool decodePlanePackBits(
            uint8* aDst, size_t aDstLength,
            const uint8* aSrc, size_t aSrcLength,
            int aWidth, int aHeight, int aDstStride);

    static XCMemBlock makeInterleavedImage(
            const PSDFormat::Header& aHeader,
            const PSDFormat::Layer& aLayer,
            ColorFormat aFormat);

    static XCMemBlock makeInterleavedImage(
            const PSDFormat::Header& aHeader,
            const PSDFormat::ImageData& aImageData,
            ColorFormat aFormat);

    static XCMemBlock makeInterleavedImage(
            const PSDFormat::Header& aHeader,
            const PSDFormat::ChannelList& aChannels,
            ColorFormat aFormat, int aWidth, int aHeight);
};

} // namespace img

#endif // IMG_PSDUTIL_H
