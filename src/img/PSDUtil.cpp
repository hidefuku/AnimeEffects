#include <vector>
#include <iostream>
#include "XC.h"
#include "img/PSDUtil.h"


//#define PSDUTIL_DUMP(...) XC_DEBUG_REPORT(__VA_ARGS__)
#define PSDUTIL_DUMP(...)

///@todo following formulas have some mistakes?
///
#define ChannelBlend_Normal(A,B)     ((uint8)(B))
#define ChannelBlend_Lighten(A,B)    ((uint8)((A > B) ? A:B))
#define ChannelBlend_Darken(A,B)     ((uint8)((A > B) ? B:A))
#define ChannelBlend_Multiply(A,B)   ((uint8)((A * B) / 255))
#define ChannelBlend_Average(A,B)    ((uint8)((A + B) / 2))
#define ChannelBlend_Add(A,B)        ((uint8)(std::min(255, (A + B))))
#define ChannelBlend_Subtract(A,B)   ((uint8)(std::max(0, A - B)))
#define ChannelBlend_Difference(A,B) ((uint8)(std::abs(A - B)))
#define ChannelBlend_Divide(A,B)     ((uint8)((B == 0) ? B : std::min(255, 255 * A / B)))
#define ChannelBlend_Screen(A,B)     ((uint8)(255 - (((255 - A) * (255 - B)) >> 8)))
#define ChannelBlend_Exclusion(A,B)  ((uint8)(A + B - 2 * A * B / 255))
#define ChannelBlend_Overlay(A,B)    ((uint8)((A < 128) ? (2 * A * B / 255):(255 - 2 * (255 - A) * (255 - B) / 255)))
#define ChannelBlend_SoftLight(A,B)  ((uint8)((A < 128) ? ((2*A*B+A*A*(255-2*B)/255)/255) : (std::sqrt(A)*(2*B-255)+(2*A)*(255-B))))
#define ChannelBlend_HardLight(A,B)  (ChannelBlend_Overlay(B,A))
#define ChannelBlend_ColorDodge(A,B) ((uint8)((B == 255) ? B:std::min(255, ((A << 8 ) / (255 - B)))))
#define ChannelBlend_ColorBurn(A,B)  ((uint8)((B == 0) ? B:std::max(0, (255 - ((255 - A) << 8 ) / B))))
#define ChannelBlend_LinearDodge(A,B)(ChannelBlend_Add(A,B))
#define ChannelBlend_LinearBurn(A,B) ((uint8)(std::max(0, A + B - 255)))
#define ChannelBlend_LinearLight(A,B)((uint8)(B < 128)?ChannelBlend_LinearBurn(A,(2 * B)):ChannelBlend_LinearDodge(A,(2 * (B - 128))))
#define ChannelBlend_VividLight(A,B) ((uint8)(B < 128)?ChannelBlend_ColorBurn(A,(2 * B)):ChannelBlend_ColorDodge(A,(2 * (B - 128))))
#define ChannelBlend_PinLight(A,B)   ((uint8)(B < 128)?ChannelBlend_Darken(A,(2 * B)):ChannelBlend_Lighten(A,(2 * (B - 128))))
#define ChannelBlend_HardMix(A,B)    ((uint8)((ChannelBlend_VividLight(A,B) < 128) ? 0:255))

#define PixelBlend_Function(RSLT, FRNT, BACK, FUNC) \
{ \
    const uint8 a0 = (uint8)((FRNT[3] * BACK[3]) / 255); \
    const uint8 a1 = (uint8)((FRNT[3] * (0xff - BACK[3])) / 255); \
    const uint8 a2 = (uint8)((BACK[3] * (0xff - FRNT[3])) / 255); \
    RSLT[3] = a0 + a1 + a2; \
    const uint32 div = (RSLT[3] == 0 ? 1 : RSLT[3]); \
    RSLT[0] = (uint8)((a0 * FUNC(BACK[0], FRNT[0]) + a1 * FRNT[0] + a2 * BACK[0]) / div); \
    RSLT[1] = (uint8)((a0 * FUNC(BACK[1], FRNT[1]) + a1 * FRNT[1] + a2 * BACK[1]) / div); \
    RSLT[2] = (uint8)((a0 * FUNC(BACK[2], FRNT[2]) + a1 * FRNT[2] + a2 * BACK[2]) / div); \
}


namespace img
{

bool PSDUtil::blendImage(
        uint8* aResult, const uint8* aBack, const QRect& aRectRB,
        const uint8* aFront, const QRect& aRectF,
        const std::string& aMode)
{    
    const int compo = 4;
    const int offsRX = aRectF.left() - aRectRB.left();
    const int offsRY = aRectF.top() - aRectRB.top();
    const int widthR = aRectRB.width();
    const int widthF = aRectF.width();

    QRect rangeF = aRectRB.intersected(aRectF);
    rangeF.translate(-aRectF.topLeft());

#define ImageBlendFunction(FUNC) \
    for (int y = rangeF.top(); y < rangeF.bottom(); ++y) \
    { \
        for (int x = rangeF.left(); x < rangeF.right(); ++x) \
        { \
            const int indexR = ((x + offsRX) + (y + offsRY) * widthR) * compo; \
            uint8* r = aResult + indexR; \
            const uint8* b = aBack + indexR; \
            const uint8* f = aFront + (x + y * widthF) * compo; \
            PixelBlend_Function(r, f, b, FUNC); \
        } \
    }

    if      (aMode == "norm") { ImageBlendFunction(ChannelBlend_Normal); return true; }
    else if (aMode == "dark") { ImageBlendFunction(ChannelBlend_Darken); return true; }
    else if (aMode == "mul ") { ImageBlendFunction(ChannelBlend_Multiply); return true; }
    else if (aMode == "idiv") { ImageBlendFunction(ChannelBlend_ColorBurn); return true; }
    else if (aMode == "lbrn") { ImageBlendFunction(ChannelBlend_LinearBurn); return true; }
    else if (aMode == "lite") { ImageBlendFunction(ChannelBlend_Lighten); return true; }
    else if (aMode == "scrn") { ImageBlendFunction(ChannelBlend_Screen); return true; }
    else if (aMode == "div ") { ImageBlendFunction(ChannelBlend_ColorDodge); return true; }
    else if (aMode == "lddg") { ImageBlendFunction(ChannelBlend_LinearDodge); return true; }
    else if (aMode == "over") { ImageBlendFunction(ChannelBlend_Overlay); return true; }
    else if (aMode == "sLit") { ImageBlendFunction(ChannelBlend_SoftLight); return true; }
    else if (aMode == "hLit") { ImageBlendFunction(ChannelBlend_HardLight); return true; }
    else if (aMode == "vLit") { ImageBlendFunction(ChannelBlend_VividLight); return true; }
    else if (aMode == "lLit") { ImageBlendFunction(ChannelBlend_LinearLight); return true; }
    else if (aMode == "pLit") { ImageBlendFunction(ChannelBlend_PinLight); return true; }
    else if (aMode == "hMix") { ImageBlendFunction(ChannelBlend_HardMix); return true; }
    else if (aMode == "diff") { ImageBlendFunction(ChannelBlend_Difference); return true; }
    else if (aMode == "smud") { ImageBlendFunction(ChannelBlend_Exclusion); return true; }
    else if (aMode == "fsub") { ImageBlendFunction(ChannelBlend_Subtract); return true; }
    else if (aMode == "fdiv") { ImageBlendFunction(ChannelBlend_Divide); return true; }

    return false;

#undef ImageBlendFunction

    // all of the mode
    // 'pass' = pass through, 'norm' = normal, 'diss' = dissolve, 'dark' = darken,
    // 'mul ' = multiply, 'idiv' = color burn, 'lbrn' = linear burn, 'dkCl' = darker color,
    // 'lite' = lighten, 'scrn' = screen, 'div ' = color dodge, 'lddg' = linear dodge,
    // 'lgCl' = lighter color, 'over' = overlay, 'sLit' = soft light, 'hLit' = hard light,
    // 'vLit' = vivid light, 'lLit' = linear light, 'pLit' = pin light, 'hMix' = hard mix,
    // 'diff' = difference, 'smud' = exclusion, 'fsub' = subtract, 'fdiv' = divide,
    // 'hue ' = hue, 'sat ' = saturation, 'colr' = color, 'lum ' = luminosity,

    // unsupported = pass, diss, dkCl, lgCl, hue , sat , colr, lum ,
}

//------------------------------------------------------------//
XCMemBlock PSDUtil::makeClippedImage(
        const uint8* aTarget, const QRect& aRectT,
        const uint8* aBase, const QRect& aRectB)
{
    const int compo = 4;
    const int alpha = 3;

    const size_t length = aRectT.width() * aRectT.height() * compo;
    XCMemBlock clipped(new uint8[length], length);
    memcpy(clipped.data, aTarget, length);

    for (size_t i = alpha; i < length; i += compo)
    {
        clipped.data[i] = 0x00;
    }

    const int offsBX = aRectT.left() - aRectB.left();
    const int offsBY = aRectT.top() - aRectB.top();
    const int widthB = aRectB.width();
    const int widthT = aRectT.width();

    QRect rangeT = aRectB.intersected(aRectT);
    rangeT.translate(-aRectT.topLeft());

    for (int y = rangeT.top(); y < rangeT.bottom(); ++y)
    {
        for (int x = rangeT.left(); x < rangeT.right(); ++x)
        {
            const int indexB = ((x + offsBX) + (y + offsBY) * widthB) * compo + alpha;
            const int indexT = (x + y * widthT) * compo + alpha;
            //clipped.data[indexT] = std::min(aBase[indexB], aTarget[indexT]);
            clipped.data[indexT] = (aBase[indexB] * aTarget[indexT]) / 255;
        }
    }
    return clipped;
}

//------------------------------------------------------------//
size_t PSDUtil::encodePackBits(const uint8* aSrc, uint8* aDst, size_t aLength)
{
    const uint8* end = aSrc + aLength;

    const uint8* p = aSrc;
    uint8* dp = aDst;
    long long int remains = aLength;

    while (remains > 0)
    {
        const uint8* mark = p;
        const uint8* markMax = mark + (remains < 128 ? remains : 128);

        // found three duplicated value
        if (mark <= (end - 3) && mark[0] == mark[1] && mark[1] == mark[2])
        {
            // scan
            p += 3;
            while (p < markMax && *p == mark[0]) ++p;

            const int count = (int)(p - mark);
            *dp++ = 1 + 256 - count;
            *dp++ = mark[0];
            remains -= count;
        }
        else
        {
            while (p < markMax)
            {
                if(p <= (end - 3) && p[0] == p[1] && p[1] == p[2]) break;
                ++p;
            }
            const int count = (int)(p - mark);
            *dp++ = count - 1;
            memcpy(dp, mark, count);
            dp += count;
            remains -= count;
        }
    }
    return dp - aDst;
}

//------------------------------------------------------------//
XCMemBlock PSDUtil::encodePlanePackBits(
        const uint8* aSrc, size_t aSrcLength,
        int aWidth, int aHeight, int aSrcStride)
{
    if ((int)aSrcLength < aWidth * aHeight * (aSrcStride - 1) + 1)
    {
        return XCMemBlock();
    }

    const size_t headerLength = sizeof(uint16) * aHeight;
    const size_t totalLength = headerLength + aWidth * aHeight * 2;
    std::unique_ptr<uint8[]> work(new uint8[totalLength]);
    std::unique_ptr<uint8[]> srcLine(new uint8[aWidth]);

    size_t currentLength = headerLength;

    for (int y = 0; y < aHeight; ++y)
    {
        XC_ASSERT(currentLength <= totalLength);

        for (int x = 0; x < aWidth; ++x)
        {
            srcLine[x] = aSrc[(x + y * aWidth) * aSrcStride];
        }

        // encode data
        const size_t lineLength = encodePackBits(srcLine.get(), work.get() + currentLength, aWidth);
        XC_ASSERT((int)lineLength <= aWidth * 2);
        // write header
        *(uint16*)(work.get() + y * sizeof(uint16)) = XC_TO_BIG_ENDIAN((uint16)lineLength);
        // update current length
        currentLength += lineLength;
    }

    XCMemBlock dst(new uint8[currentLength], currentLength);
    memcpy(dst.data, work.get(), currentLength);
    return dst;
}

bool PSDUtil::makeChanneledImage(
        PSDFormat::Layer& aDst,
        const PSDFormat::Header& aHeader,
        const XCMemBlock& aSrc,
        ColorFormat aSrcFormat)
{
    return makeChanneledImage(aDst.channels, aHeader, aSrc, aSrcFormat, aDst.rect.width(), aDst.rect.height());
}

bool PSDUtil::makeChanneledImage(
        PSDFormat::ImageData& aDst,
        const PSDFormat::Header& aHeader,
        const XCMemBlock& aSrc,
        ColorFormat aSrcFormat)
{
    return makeChanneledImage(aDst.channels, aHeader, aSrc, aSrcFormat, aHeader.width, aHeader.height);
}

bool PSDUtil::makeChanneledImage(
        PSDFormat::ChannelList& aDst,
        const PSDFormat::Header& aHeader,
        const XCMemBlock& aSrc,
        ColorFormat aSrcFormat,
        int aSrcWidth, int aSrcHeight)
{
    PSDUTIL_DUMP("begin make channeled image");

    if (aHeader.mode != PSDFormat::ColorMode_RGB)
    {
        return false;
    }
    if (aHeader.depth != 8)
    {
        return false;
    }

    const int w = aSrcWidth;
    const int h = aSrcHeight;

    typedef std::pair<sint16, int> Connector;
    std::vector<Connector> connectors;

    PSDUTIL_DUMP("analyze color format");

    switch (aSrcFormat)
    {
    case ColorFormat_RGB8:
        connectors.push_back(Connector( 0, 0));
        connectors.push_back(Connector( 1, 1));
        connectors.push_back(Connector( 2, 2));
        break;
    case ColorFormat_RGBA8:
        connectors.push_back(Connector( 0, 0));
        connectors.push_back(Connector( 1, 1));
        connectors.push_back(Connector( 2, 2));
        connectors.push_back(Connector(-1, 3));
        if (aSrc.size != (size_t)(w * h * 4)) return false;
        break;
    }

    PSDUTIL_DUMP("check color format");

    if (connectors.empty())
    {
        return false;
    }
    const int compoNum = (int)connectors.size();
    if (aSrc.size != (size_t)(w * h * compoNum)) return false;

    PSDUTIL_DUMP("to interleaved");

    for (PSDFormat::ChannelPtr& chan : aDst)
    {
        int index = -1;
        for (Connector connect : connectors)
        {
            if (chan->id == connect.first)
            {
                index = connect.second;
                break;
            }
        }
        if (index == -1) continue;

        if (chan->compressionId == 0)
        {
            if (chan->dataLength != (size_t)(w * h))
            {
                return false;
            }

            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    (chan->data.get())[x + y * w] =
                            (aSrc.data)[(x + y * w) * compoNum + index];
                }
            }
        }
        else if (chan->compressionId == 1)
        {
            XCMemBlock block = encodePlanePackBits(aSrc.data + index, aSrc.size, w, h, compoNum);
            if (!block.data)
            {
                return false;
            }
            chan->data.reset(block.data);
            chan->dataLength = static_cast<uint32>(block.size);
        }
        else
        {
            return false;
        }
    }

    PSDUTIL_DUMP("finish make channeled image");
    return true;
}

//------------------------------------------------------------//
// RLE compress plane data(TIFF Standard) to interleaved
//------------------------------------------------------------//
bool PSDUtil::decodePlanePackBits(
        uint8* aDst, size_t aDstLength,
        const uint8* aSrc, size_t aSrcLength,
        int aWidth, int aHeight, int aDstStride)
{
    typedef uint16 LineLengthType;

    if (!aDst || !aSrc || aWidth < 1 || aHeight < 1 || aDstStride < 1)
    {
        PSDUTIL_DUMP("decode packbits error 1");
        return false;
    }

    const int headerSize = sizeof(LineLengthType) * aHeight;
    const LineLengthType* header = (const LineLengthType*)aSrc;
    const uint8* srcData = aSrc + headerSize;


    // too short
    if (aDstLength < (unsigned int)(aWidth * aHeight * (aDstStride - 1) + 1))
    {
        PSDUTIL_DUMP("decode packbits error 2");
        return false;
    }

    // too short
    if (aSrcLength < (unsigned int)headerSize)
    {
        PSDUTIL_DUMP("decode packbits error 3");
        return false;
    }

    // decompress RLE for each scanlines
    for(int y = 0; y < aHeight; ++y)
    {
        const uint8* lineEnd = srcData + XC_FROM_BIG_ENDIAN(header[y]);

        // too short
        if (aSrc + aSrcLength < lineEnd)
        {
            PSDUTIL_DUMP("decode packbits error 4");
            return false;
        }

        int x = 0;
        while (srcData < lineEnd)
        {
            // packet header
            const char packHead = (char)(*srcData);

            if (packHead < 0)
            {
                // noop operation
                if (packHead == -128) continue;

                // continuos operation
                const int count = -packHead + 1;
                x += count;
                if (x > aWidth)
                {
                    PSDUTIL_DUMP("decode packbits error 5");
                    return false;
                }

                const uint8 value = *(++srcData);
                for (int i = 0; i < count; ++i, aDst += aDstStride)
                {
                    *aDst = value;
                }
            }
            else
            {
                // no continuos operation
                const int count = packHead + 1;
                x += count;
                if (x > aWidth)
                {
                    PSDUTIL_DUMP("decode packbits error 6: %d", x);
                    return false;
                }

                for (int i = 0; i < count; ++i, aDst += aDstStride)
                {
                    if (srcData >= lineEnd)
                    {
                        PSDUTIL_DUMP("decode packbits error 7");
                        return false;
                    }
                    *aDst = *(++srcData);
                }
            }

            if (srcData < lineEnd) ++srcData;
        }

        if (x != aWidth)
        {
            PSDUTIL_DUMP("decode packbits error 8: %d", x);
            return false;
        }
    }

    return true;
}

XCMemBlock PSDUtil::makeInterleavedImage(
        const PSDFormat::Header& aHeader,
        const PSDFormat::Layer& aLayer,
        ColorFormat aFormat)
{
    if (aLayer.entryType != PSDFormat::LayerEntryType_Layer)
    {
        return XCMemBlock();
    }

    return makeInterleavedImage(aHeader, aLayer.channels, aFormat, aLayer.rect.width(), aLayer.rect.height());
}

XCMemBlock PSDUtil::makeInterleavedImage(
        const PSDFormat::Header& aHeader,
        const PSDFormat::ImageData& aImageData,
        ColorFormat aFormat)
{
    return makeInterleavedImage(aHeader, aImageData.channels, aFormat, (int)aHeader.width, (int)aHeader.height);
}

XCMemBlock PSDUtil::makeInterleavedImage(
        const PSDFormat::Header& aHeader,
        const PSDFormat::ChannelList& aChannels,
        ColorFormat aFormat, int aWidth, int aHeight)
{
    PSDUTIL_DUMP("begin make interleaved image");

    if (aHeader.mode != PSDFormat::ColorMode_RGB)
    {
        return XCMemBlock(NULL, 1);
    }
    if (aHeader.depth != 8)
    {
        return XCMemBlock(NULL, 2);
    }

    std::vector<const PSDFormat::Channel*> seq;
    const PSDFormat::Channel* mergeAlpha = nullptr;

    XCMemBlock image;
    const int w = aWidth;
    const int h = aHeight;

    PSDUTIL_DUMP("analyze color format");

    switch (aFormat)
    {
    case ColorFormat_RGB8:
        seq.assign(3, NULL);
        for (const PSDFormat::ChannelPtr& chan : aChannels)
        {
            if (0 <= chan->id && chan->id < 3) { seq[chan->id] = chan.get(); }
            else if (chan->id == -1) { mergeAlpha = chan.get(); }
        }
        if (std::find(seq.begin(), seq.end(), (const PSDFormat::Channel*)NULL) == seq.end())
        {
            image.size = w * h * 3;
            image.data = new uint8[image.size];
        }
        break;
    case ColorFormat_RGBA8:
        seq.assign(4, NULL);
        for (const PSDFormat::ChannelPtr& chan : aChannels)
        {
            if (0 <= chan->id && chan->id < 3) { seq[chan->id] = chan.get(); }
            else if (chan->id == -1) { seq[3] = chan.get(); }
        }
        if (seq.at(0) != NULL && seq.at(1) != NULL && seq.at(2) != NULL)
        {
            image.size = w * h * 4;
            image.data = new uint8[image.size];
        }
        break;
    }

    PSDUTIL_DUMP("check color format");
    if (!image.data)
    {
        return XCMemBlock(NULL, 3);
    }

    PSDUTIL_DUMP("to interleaved");

    const int stride = (int)seq.size();
    int i = 0;
    for (const PSDFormat::Channel* chan : seq)
    {
        if (chan)
        {
            if (chan->compressionId == 0)
            {
                for (int y = 0; y < h; ++y)
                {
                    for (int x = 0; x < w; ++x)
                    {
                        (image.data)[(x + y * w) * stride + i] = (chan->data.get())[x + y * w];
                    }
                }
            }
            else if (chan->compressionId == 1)
            {
                if (!decodePlanePackBits(image.data + i, image.size, chan->data.get(), (size_t)chan->dataLength, w, h, stride))
                {
                    PSDUTIL_DUMP("decode error: id %d", chan->id);
                    return XCMemBlock(NULL, 4);
                }
            }
            else
            {
                return XCMemBlock(NULL, 5);
            }
        }
        else
        {
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    (image.data)[(x + y * w) * stride + i] = 0xff;
                }
            }
        }
        ++i;
    }

    if (mergeAlpha)
    {
        std::vector<uint8> work;
        const uint8* alpha = mergeAlpha->data.get();
        uint8* color = image.data;

        if (mergeAlpha->compressionId == 0)
        {
            alpha = mergeAlpha->data.get();
        }
        else if (mergeAlpha->compressionId == 1)
        {
            work.resize(w * h);
            alpha = work.data();
            if (!decodePlanePackBits(work.data(), work.size(), mergeAlpha->data.get(), (size_t)mergeAlpha->dataLength, w, h, 1))
            {
                return XCMemBlock(NULL, 6);
            }
        }
        else
        {
            return XCMemBlock(NULL, 7);
        }

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const int pixIdx = x + y * w;
                const uint8 a = alpha[pixIdx];

                for (int i = 0; i < stride; ++i)
                {
                    const int byteIdx = stride * pixIdx + i;
                    const uint32 merged = (color[byteIdx] * a + 255 * (255 - a)) / 255;
                    color[byteIdx] = (uint8)(merged & 0x000000ff);
                }
            }
        }
    }

    PSDUTIL_DUMP("finish interleaved. %u, %d\n", (uint32)(image.data), image.size);
    return image;
}

} // namespace img
