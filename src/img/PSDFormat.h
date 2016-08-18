#ifndef IMG_PSDFORMAT_H
#define IMG_PSDFORMAT_H

#include <memory>
#include "XC.h"

namespace img
{

class PSDFormat
{
public:
    enum ColorMode
    {
        ColorMode_Bitmap = 0,
        ColorMode_Grayscale = 1,
        ColorMode_Indexed = 2,
        ColorMode_RGB = 3,
        ColorMode_CMYK = 4,
        ColorMode_Multichannel = 7,
        ColorMode_Duotone = 8,
        ColorMode_Lab = 9,
    };

    enum LayerEntryType
    {
        LayerEntryType_Layer,
        LayerEntryType_OpenFolder,
        LayerEntryType_CloseFolder,
        LayerEntryType_Bounding,
        LayerEntryType_TERM,
    };

    static int getChannelCount(ColorMode aMode)
    {
        switch (aMode)
        {
        case ColorMode_Bitmap:
        case ColorMode_Grayscale:
        case ColorMode_Indexed:
        case ColorMode_Duotone:
            return 1;
        case ColorMode_RGB:
        case ColorMode_Lab:
            return 3;
        case ColorMode_CMYK:
            return 4;
        default:
            //ColorMode_Multichannel
            return -1;
        }
    }

    struct Rect
    {
        Rect() { for (int i = 0; i < 4; ++i) edge[i] = 0; }
        int edge[4]; // t, l, b, r
        int width() const { return edge[3] - edge[1]; }
        int height() const { return edge[2] - edge[0]; }
        int left() const { return edge[1]; }
        int top() const { return edge[0]; }
        int right() const { return edge[3]; }
        int bottom() const { return edge[2]; }
    };

    struct BlendingRange
    {
        BlendingRange() : isValid()
        { src[0] = src[1] = dst[0] = dst[1] = 0; src[2] = src[3] = dst[2] = dst[3] = 255; }
        bool isValid;
        uint8 src[4];
        uint8 dst[4];
    };

    class Header
    {
    public:
        uint16 version;
        uint16 channels;
        uint32 height;
        uint32 width;
        uint16 depth;
        uint16 mode;
    };

    class ColorModeData
    {
    public:
    };

    class ImageResourceBlock
    {
    public:
        uint16 id;
        std::string name;
        uint32 dataLength;
        std::unique_ptr<uint8[]> data;
    };
    typedef std::unique_ptr<ImageResourceBlock> ImageResourceBlockPtr;
    typedef std::list<ImageResourceBlockPtr> ImageResourceBlockList;

    class ImageResources
    {
    public:
        ImageResourceBlockList blocks;
    };

    class Channel
    {
    public:
        sint16 id;
        uint16 compressionId;
        uint32 dataLength;
        std::unique_ptr<uint8[]> data;
        BlendingRange blendingRange;
    };
    typedef std::unique_ptr<Channel> ChannelPtr;
    typedef std::list<ChannelPtr> ChannelList;

    class LayerMask
    {
    public:
        LayerMask() :
            rect(), defaultColor(), flags(),
            hasReal(false), realFlags(), realUserMaskBG(), realUserRect() {}

        Rect rect;
        uint8 defaultColor;
        uint8 flags;
        bool hasReal;
        uint8 realFlags;
        uint8 realUserMaskBG;
        Rect realUserRect;
    };

    class AdditionalLayerInfo
    {
    public:
        std::string key;
        uint32 dataLength;
        std::unique_ptr<uint8[]> data;
    };
    typedef std::unique_ptr<AdditionalLayerInfo> AdditionalLayerInfoPtr;
    typedef std::list<AdditionalLayerInfoPtr> AdditionalLayerInfoList;

    class Layer
    {
    public:
        Layer()
            : entryType(LayerEntryType_Layer), entryKey(),
              channelCount(), opacity(), clipping(), flagsOffset(), flags() {}

        bool isVisible() const { return (flags & 0x02) == 0; }
        LayerEntryType entryType; // doesn't ref in writer
        std::string entryKey; // doesn't ref in writer
        Rect rect;
        uint16 channelCount; // doesn't ref in writer
        ChannelList channels;
        std::string blendMode;
        uint8 opacity;
        uint8 clipping;
        std::ios::pos_type flagsOffset; // doesn't ref in writer
        uint8 flags;
        std::unique_ptr<LayerMask> mask;
        std::unique_ptr<BlendingRange> compositeBlendingRange;
        std::string name;
        AdditionalLayerInfoList additionalInfos; // it's contained by each layer
    };
    typedef std::unique_ptr<Layer> LayerPtr;
    typedef std::list<LayerPtr> LayerList;

    class GlobalLayerMaskInfo
    {
    public:
        uint16 overlayColorSpace;
        uint32 colorComponent[2];
        uint16 opacity;
        uint8 kind;
        int fillerCount;
    };

    class LayerAndMaskInfo
    {
    public:
        LayerAndMaskInfo() : layerCount() {}

        sint16 layerCount; // doesn't ref in writer
        LayerList layers;
        std::unique_ptr<GlobalLayerMaskInfo> globalLayerMaskInfo;
        AdditionalLayerInfoList additionalLayerInfos; // global infos
    };

    class ImageData
    {
    public:
        uint16 compressionId;
        uint8 hasTransparency;
        std::list<ChannelPtr> channels;
    };

    PSDFormat()
        : mHeader()
        , mColorModeData()
        , mImageResources()
        , mLayerAndMaskInfo()
        , mImageData()
    {
    }

    Header& header() { return mHeader; }
    const Header& header() const { return mHeader; }

    ColorModeData& colorModeData() { return mColorModeData; }
    const ColorModeData& colorModeData() const { return mColorModeData; }

    ImageResources& imageResources() { return mImageResources; }
    const ImageResources& imageResources() const { return mImageResources; }

    LayerAndMaskInfo& layerAndMaskInfo() { return mLayerAndMaskInfo; }
    const LayerAndMaskInfo& layerAndMaskInfo() const { return mLayerAndMaskInfo; }

    ImageData& imageData() { return mImageData; }
    const ImageData& imageData() const { return mImageData; }

private:
    Header mHeader;
    ColorModeData mColorModeData;
    ImageResources mImageResources;
    LayerAndMaskInfo mLayerAndMaskInfo;
    ImageData mImageData;
};

} // namespace img

#endif // IMG_PSDFORMAT_H
