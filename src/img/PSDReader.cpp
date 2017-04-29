#include <memory>
#include "XC.h"
#include "img/PSDReader.h"


//#define PSDREADER_DUMP(...) XC_DEBUG_REPORT(__VA_ARGS__)
#define PSDREADER_DUMP(...)

//#define PSDREADER_VERBOSE(...) XC_DEBUG_REPORT(__VA_ARGS__)
#define PSDREADER_VERBOSE(...)

namespace img
{

PSDReader::PSDReader(std::istream& aIo)
    : StreamReader(aIo)
    , mFormat()
    , mResultCode(ResultCode_TERM)
    , mSection()
    , mValue()
{
    if (isFailed())
    {
        mSection = "input stream";
        mResultCode = ResultCode_InvalidFileHandle;
        return;
    }

    mFormat.reset(new PSDFormat());

    if (!loadHeader())
    {
        return;
    }
    PSDREADER_DUMP("loaded header. tell 1: %d", (int)tellg());

    if (!loadColorModeData())
    {
        return;
    }
    PSDREADER_DUMP("loaded color mode data. tell 2: %d", (int)tellg());

    if (!loadImageResources())
    {
        return;
    }
    PSDREADER_DUMP("loaded image resources. tell 3: %d", (int)tellg());

    if (!loadLayerAndMaskInfo())
    {
        return;
    }
    PSDREADER_DUMP("loaded layer and mask info. tell 4: %d", (int)tellg());

    if (!loadImageData())
    {
        return;
    }
    PSDREADER_DUMP("loaded image data. tell 5: %d", (int)tellg());

    mSection = "end of file";
    mResultCode = ResultCode_Success;
}


//------------------------------------------------------------//
// psd header(26 bytes)
//------------------------------------------------------------//
bool PSDReader::loadHeader()
{
    mSection = "header";

    PSDFormat::Header& form = mFormat->header();

    // file signature
    std::string signature = readString(4);
    if (checkFailure()) return false;
    PSDREADER_DUMP("signature: %s", signature.c_str());
    if (signature != "8BPS")
    {
        mSection = "header/signature";
        mValue = signature;
        mResultCode = ResultCode_InvalidSignature;
        return false;
    }

    // version
    form.version = readUInt16();
    if (checkFailure()) return false;
    PSDREADER_DUMP("version: %d", form.version);
    if (form.version != 1)
    {
        mSection = "header/version";
        mValue = std::to_string(form.version);
        mResultCode = ResultCode_UnsupportedVersion;
        return false;
    }

    if (!skipZeroArea(6))
    {
        mSection = "header/reserve bits";
        mResultCode = ResultCode_InvalidValue;
        return false;
    }
    if (checkFailure()) return false;

    // channels
    form.channels = readUInt16();
    if (checkFailure()) return false;
    PSDREADER_DUMP("channels: %d", form.channels);
    if (form.channels < 1 || 56 < form.channels)
    {
        mSection = "header/channels";
        mValue = std::to_string(form.channels);
        mResultCode = ResultCode_InvalidValue;
        return false;
    }

    // height
    form.height = readUInt32();
    if (checkFailure()) return false;
    PSDREADER_DUMP("height: %d", form.height);
    if (form.height < 1 || 30000 < form.height)
    {
        mSection = "header/height";
        mValue = std::to_string(form.height);
        mResultCode = ResultCode_InvalidValue;
        return false;
    }

    // width
    form.width = readUInt32();
    if (checkFailure()) return false;
    PSDREADER_DUMP("width: %d", form.width);
    if (form.width < 1 || 30000 < form.width)
    {
        mSection = "header/width";
        mValue = std::to_string(form.width);
        mResultCode = ResultCode_InvalidValue;
        return false;
    }

    // depth
    form.depth = readUInt16();
    if (checkFailure()) return false;
    PSDREADER_DUMP("depth: %d", form.depth);
    switch (form.depth)
    {
    case 1:
    case 8:
    case 16:
    case 32:
        break;
    default:
        mSection = "header/depth";
        mValue = std::to_string(form.depth);
        mResultCode = ResultCode_InvalidValue;
        return false;
    }

    // mode
    form.mode = readUInt16();
    if (checkFailure()) return false;
    PSDREADER_DUMP("mode: %d", form.mode);
    switch (form.mode)
    {
    case PSDFormat::ColorMode_Bitmap:
    case PSDFormat::ColorMode_Grayscale:
    case PSDFormat::ColorMode_RGB:
    case PSDFormat::ColorMode_CMYK:
    case PSDFormat::ColorMode_Multichannel:
    case PSDFormat::ColorMode_Lab:
        break;
    case PSDFormat::ColorMode_Indexed:
    case PSDFormat::ColorMode_Duotone:
        mSection = "header/mode";
        mValue = std::to_string(form.mode);
        mResultCode = ResultCode_UnsupportedValue;
        return false;
    default:
        mSection = "header/mode";
        mValue = std::to_string(form.mode);
        mResultCode = ResultCode_UnknownValue;
        return false;
    }

    return true;
}

//------------------------------------------------------------//
// color mode data
//------------------------------------------------------------//
bool PSDReader::loadColorModeData()
{
    mSection = "color mode data";

    XC_ASSERT(mFormat->header().mode != PSDFormat::ColorMode_Indexed); // unsupport: index mode
    XC_ASSERT(mFormat->header().mode != PSDFormat::ColorMode_Duotone); // unsupport: duotone mode

    uint32 length = readUInt32();
    if (checkFailure()) return false;
    if (length != 0)
    {
        mSection = "color mode data/length";
        mValue = std::to_string(length);
        mResultCode = ResultCode_InvalidValue;
        return false;
    }

    return true;
}

//------------------------------------------------------------//
// image resources
//------------------------------------------------------------//
bool PSDReader::loadImageResources()
{
    mSection = "image resources";

    unsigned long headerSize = readUInt32();
    if (checkFailure()) return false;
    PSDREADER_DUMP("image resource blocks size: %lu", headerSize);

    while (headerSize)
    {
        PSDFormat::ImageResourceBlock* block = new PSDFormat::ImageResourceBlock();
        mFormat->imageResources().blocks.push_back(PSDFormat::ImageResourceBlockPtr(block));

        std::ios::pos_type blockBegin = tellg();

        // signature
        std::string signature = readString(4);
        if (signature != "8BIM")
        {
            mSection = "image resources/signature";
            mValue = signature;
            mResultCode = ResultCode_InvalidSignature;
            return false;
        }

        // id
        block->id = readUInt16();
        if (checkFailure()) return false;

        // name
        const unsigned char nameLength = readByte();
        if (nameLength > 0)
        {
            block->name = readString(nameLength);
            skipPads(nameLength + 1, 2);
        }
        else
        {
            readByte();
        }
        if (checkFailure()) return false;

        // block data
        block->dataLength = readUInt32();
        if (checkFailure()) return false;

        block->data.reset(new uint8[block->dataLength]);
        readBuf(block->data.get(), block->dataLength);
#if 0
        if (block->id == 0x03ed && block->dataLength == 16)
        {
            sint32 hRes = readSInt32();
            sint16 hResUnit = readSInt16();
            sint16 widthUnit = readSInt16();
            sint32 vRes = readSInt32();
            sint16 vResUnit = readSInt16();
            sint16 heightUnit = readSInt16();
            PSDREADER_DUMP("image resource block resolution info: %d", block->dataLength);
            PSDREADER_DUMP("    hRes       : %d", hRes);
            PSDREADER_DUMP("    hResUnit   : %d", hResUnit);
            PSDREADER_DUMP("    widthUnit  : %d", widthUnit);
            PSDREADER_DUMP("    vRes       : %d", vRes);
            PSDREADER_DUMP("    vResUnit   : %d", vResUnit);
            PSDREADER_DUMP("    heightUnit : %d", heightUnit);
        }
        else if (block->id == 0x0402)
        {
            PSDREADER_DUMP("image resource block group ids: %d", block->dataLength);
            const int count = block->dataLength / 2;

            for (int i = 0; i < count; ++i)
            {
                unsigned short groupId = readUInt16();
                (void)groupId;
                //PSDREADER_DUMP("    groupId : %d", groupId);
            }
            if (block->dataLength % 2 != 0)
            {
                skip(1);
            }
        }
        else if (id == 0x0421)
        {
            uint32 version = readUInt32();
            uint8 hasRealMergedData = readByte();
            PSDREADER_DUMP("version info: version %d, hasRealMergedData %d", version, hasRealMergedData);
            skip(block->dataLength - 5);
        }
        else
        {
            skip(block->dataLength);
        }
#endif

        skipPads(block->dataLength, 2);
        if (checkFailure()) return false;

        std::ios::pos_type blockEnd = tellg();
        const unsigned long blockSize = static_cast<unsigned long>(blockEnd - blockBegin);

        PSDREADER_VERBOSE("image resource block: id(%x), data size %d", block->id, block->dataLength);

        if (headerSize < blockSize)
        {
            mSection = "image resources/image resource block";
            mValue = std::to_string(headerSize);
            mResultCode = ResultCode_UnexpectedEndOfSection;
            return false;
        }
        headerSize -= blockSize;
    }
    return true;
}

//------------------------------------------------------------//
// layer and mask information blocks
//------------------------------------------------------------//
bool PSDReader::loadLayerAndMaskInfo()
{
    mSection = "layer and mask info";

    PSDFormat::LayerAndMaskInfo& form = mFormat->layerAndMaskInfo();

    // total length
    unsigned long layerAndMaskLength = readUInt32();
    if(layerAndMaskLength == 0)
    {
        return true;
    }
    const std::ios::pos_type endPos = tellg() + std::ios::off_type(layerAndMaskLength);

    // layers info length
    unsigned long layersInfoLength = readUInt32();
    const std::ios::pos_type layerInfoEnd = tellg() + std::ios::off_type(layersInfoLength);

    // layers
    form.layerCount = readSInt16();
    if (form.layerCount < 0)
    {
        // first alpha channel contains the transparency data for merged result.
        mFormat->imageData().hasTransparency = true;
        form.layerCount *= -1;
    }
    else
    {
        mFormat->imageData().hasTransparency = false;
    }

    if (checkFailure()) return false;
    PSDREADER_DUMP("layer and mask size: %lu", layerAndMaskLength);
    PSDREADER_DUMP("layers info size: %lu", layersInfoLength);
    PSDREADER_DUMP("layer count: %d", form.layerCount);

    // each layer
    for (int i = 0; i < form.layerCount; ++i)
    {
        PSDFormat::Layer* layer = new PSDFormat::Layer();
        mFormat->layerAndMaskInfo().layers.push_back(PSDFormat::LayerPtr(layer));

        // rect
        for (int i = 0; i < 4; ++i)
        {
            layer->rect.edge[i] = readUInt32();
        }
        if (checkFailure()) return false;

        // channel count
        layer->channelCount = readUInt16();
        if (checkFailure()) return false;

        // each channel
        for (int k = 0; k < layer->channelCount; ++k)
        {
            PSDFormat::Channel* channel = new PSDFormat::Channel();
            layer->channels.push_back(PSDFormat::ChannelPtr(channel));

            channel->id = readSInt16();
            channel->dataLength = readUInt32();

            // without compression id
            if (channel->dataLength >= 2)
            {
                channel->dataLength -= 2;
            }
            if (checkFailure()) return false;
        }

        // blend mode
        std::string blendModeSig = readString(4);
        if (blendModeSig != "8BIM")
        {
            mSection = "layer and mask info/layer info/layer records/blend mode signature";
            mValue = blendModeSig;
            mResultCode = ResultCode_InvalidSignature;
            return false;
        }
        if (checkFailure()) return false;

        layer->blendMode   = readString(4);
        layer->opacity     = readByte();
        layer->clipping    = readByte();
        layer->flagsOffset = tellg();
        layer->flags       = readByte();
        skip(1); // padding

        // length of the extra data field
        unsigned long extraLength = readUInt32();
        const std::ios::pos_type layerEnd = tellg() + std::ios::off_type(extraLength);
        if (checkFailure()) return false;

        // layer mask section
        unsigned long maskLength = readUInt32();
        if (maskLength >= 20)
        {
            PSDFormat::LayerMask* mask = new PSDFormat::LayerMask();
            layer->mask.reset(mask);

            const std::ios::pos_type maskEnd = tellg() + std::ios::off_type(maskLength);

            for (int i = 0; i < 4; ++i)
            {
                mask->rect.edge[i] = readUInt32();
            }
            mask->defaultColor  = readByte();
            mask->flags  = readByte();
            if (checkFailure()) return false;

            // the mask has real params.
            if (maskLength >= 36)
            {
                mask->hasReal = true;
                mask->realFlags = readByte();
                mask->realUserMaskBG = readByte();
                for (int i = 0; i < 4; ++i)
                {
                    mask->realUserRect.edge[i] = readUInt32();
                }
            }
            else
            {
                skip(2); // padding
            }
            if (checkFailure()) return false;

            // fail safe code
            if (tellg() != maskEnd)
            {
                skipTo(maskEnd);
            }
        }

        // blending range
        uint32 blendRangeLength = readUInt32();
        if (checkFailure()) return false;
        if (blendRangeLength >= 8)
        {
            PSDFormat::BlendingRange* compoRange = new PSDFormat::BlendingRange();
            compoRange->isValid = true;
            for (int i = 0; i < 4; ++i) compoRange->src[i] = readByte();
            for (int i = 0; i < 4; ++i) compoRange->dst[i] = readByte();
            if (checkFailure()) return false;
            layer->compositeBlendingRange.reset(compoRange);
            blendRangeLength -= 8;

            for (PSDFormat::ChannelPtr& channel : layer->channels)
            {
                if (blendRangeLength < 8)
                {
                    break;
                }
                for (int i = 0; i < 4; ++i) channel->blendingRange.src[i] = readByte();
                for (int i = 0; i < 4; ++i) channel->blendingRange.dst[i] = readByte();
                if (checkFailure()) return false;
                blendRangeLength -= 8;
            }
        }
        skip(blendRangeLength); // fail safe code

        // layer name
        unsigned char nameLength = readByte();
        layer->name = readString(nameLength);
        skipPads(nameLength + 1, 4);
        if (checkFailure()) return false;

#if 0
        // entry temp setting
        if (layer->name == "</Layer group>")
        {
            layer->entryType = PSDFormat::LayerEntryType_Bounding;
        }
#endif
        // additional layer info
        if (!loadAdditionalLayerInfo(layer->additionalInfos, layer, layerEnd))
        {
            mSection = "layer and mask info/layer info/layer records/" + mSection;
            return false;
        }
    }
    PSDREADER_DUMP("pre cid : tell %d", (int)tellg());

    // channel image data
    for (PSDFormat::LayerPtr& layer : form.layers)
    {
        PSDREADER_VERBOSE("channel image data: %s", layer->name.c_str());

        for (PSDFormat::ChannelPtr& channel : layer->channels)
        {
            // compression id
            channel->compressionId = readUInt16();
            PSDREADER_VERBOSE("channel image comp: %d", channel->compressionId);
            if (channel->compressionId != 0 && channel->compressionId != 1)
            {
                mSection = "layer and mask info/layer info/channel image data/compression id";
                mValue = std::to_string(channel->compressionId);
                mResultCode = ResultCode_UnsupportedValue;
                return false;
            }

            // read image
            channel->data.reset(new uint8[channel->dataLength]);
            readBuf(channel->data.get(), channel->dataLength);
            if (checkFailure()) return false;
            PSDREADER_VERBOSE("channel image size: %d", channel->dataLength);
        }
    }
    PSDREADER_DUMP("post cid : remain %d", (int)(layerInfoEnd - tellg()));

    skipTo(layerInfoEnd); // fail safe code

    // global layer mask info
    if (endPos - tellg() > 0)
    {
        PSDREADER_DUMP("global layer mask info : tell %d, size %d", (int)tellg(), (int)(endPos - tellg()));

        // global layer mask info
        uint32 length = readUInt32();
        if (length)
        {
            static const int kUseSize = 13;
            form.globalLayerMaskInfo.reset(new PSDFormat::GlobalLayerMaskInfo);

            form.globalLayerMaskInfo->overlayColorSpace = readUInt16();
            form.globalLayerMaskInfo->colorComponent[0] = readUInt32();
            form.globalLayerMaskInfo->colorComponent[1] = readUInt32();
            form.globalLayerMaskInfo->opacity = readUInt16();
            form.globalLayerMaskInfo->kind = readByte();
            form.globalLayerMaskInfo->fillerCount = length - kUseSize;
            if (length > kUseSize) skip(length - kUseSize);

            if (checkFailure()) return false;

            PSDREADER_DUMP("glmi : %d", length);
            PSDREADER_DUMP("glmi : %d", form.globalLayerMaskInfo->overlayColorSpace);
            PSDREADER_DUMP("glmi : %d", form.globalLayerMaskInfo->colorComponent[0]);
            PSDREADER_DUMP("glmi : %d", form.globalLayerMaskInfo->colorComponent[1]);
            PSDREADER_DUMP("glmi : %d", form.globalLayerMaskInfo->opacity);
            PSDREADER_DUMP("glmi : %d", form.globalLayerMaskInfo->kind);
            PSDREADER_DUMP("glmi : %d", form.globalLayerMaskInfo->fillerCount);
        }
    }

    // additional layer info
    if (!loadAdditionalLayerInfo(form.additionalLayerInfos, NULL, endPos))
    {
        return false;
    }

    skipTo(endPos); // fail safe code

    return true;
}

//------------------------------------------------------------//
// additional layer info
//------------------------------------------------------------//
bool PSDReader::loadAdditionalLayerInfo(
        std::list<PSDFormat::AdditionalLayerInfoPtr>& aList,
        PSDFormat::Layer* aLayer,
        std::ios::pos_type aEndPos)
{
    int length = aEndPos - tellg();
    std::string dumpKeys; // for dump
    while (length >= 12)
    {
        PSDFormat::AdditionalLayerInfo* info = new PSDFormat::AdditionalLayerInfo();
        aList.push_back(PSDFormat::AdditionalLayerInfoPtr(info));

        // signature
        std::string signature = readString(4);
        if (checkFailure()) return false;
        if (signature != "8BIM" && signature != "8B64")
        {
            mSection += "/additional layer info/signature";
            mValue = signature;
            mResultCode = ResultCode_InvalidSignature;
            return false;
        }

        // key
        info->key = readString(4);
        if (checkFailure()) return false;
        dumpKeys += info->key + " ";

        // length
        info->dataLength  = readUInt32();
        if (checkFailure()) return false;

        //Section divider setting
        if (aLayer && info->key == "lsct" && info->dataLength >= 4)
        {
            std::ios::pos_type dataBeginPos = tellg();
            // entry
            aLayer->entryType = (PSDFormat::LayerEntryType)readUInt32();
            if (checkFailure()) return false;

            // check value validity
            if (aLayer->entryType >= PSDFormat::LayerEntryType_TERM)
            {
                mSection += "/additional layer info/section divider setting/entry";
                mValue = std::to_string(aLayer->entryType);
                mResultCode = ResultCode_InvalidValue;
                return false;
            }
            if (checkFailure()) return false;

            if (info->dataLength >= 12)
            {
                // signature
                std::string signature = readString(4);
                if (checkFailure()) return false;

                // check value validity
                if (signature != "8BIM")
                {
                    mSection += "/additional layer info/signature/section divider setting/signature";
                    mValue = signature;
                    mResultCode = ResultCode_InvalidSignature;
                    return false;
                }
                aLayer->entryKey = readString(4);
                if (checkFailure()) return false;
            }

            skipTo(dataBeginPos);
        }
        else if (aLayer && info->key == "lsdk" && info->dataLength >= 4)
        {   //Nested Section divider setting (Undocumented option)

            std::ios::pos_type dataBeginPos = tellg();
            // entry
            aLayer->entryType = (PSDFormat::LayerEntryType)readUInt32();
            if (checkFailure()) return false;

            // check value validity
            if (aLayer->entryType >= PSDFormat::LayerEntryType_TERM)
            {
                mSection += "/additional layer info/nested section divider setting/entry";
                mValue = std::to_string(aLayer->entryType);
                mResultCode = ResultCode_InvalidValue;
                return false;
            }
            if (checkFailure()) return false;

            skipTo(dataBeginPos);
        }

        // read as a raw data
        info->data.reset(new uint8[info->dataLength]);
        readBuf(info->data.get(), info->dataLength);
        if (checkFailure()) return false;

        // to next
        length = aEndPos - tellg();
    }
    skip(length);// fail safe code
    if (checkFailure()) return false;

    PSDREADER_VERBOSE("additional layer info key: %s", dumpKeys.c_str());
    return true;
}

//------------------------------------------------------------//
// merged image data
//------------------------------------------------------------//
bool PSDReader::loadImageData()
{
    mSection = "image data";

    PSDFormat::Header& header = mFormat->header();
    PSDFormat::ImageData& form = mFormat->imageData();
    const uint32 rowBytes = (header.width * header.height + 7) / 8;
    const int modeChannelCount = PSDFormat::getChannelCount((PSDFormat::ColorMode)header.mode);

    form.compressionId = readUInt16();
    PSDREADER_DUMP("compression id: %d", form.compressionId);
    if (checkFailure()) return false;

    if (form.compressionId != 0 && form.compressionId != 1)
    {
        mSection = "image data/compression id";
        mValue = std::to_string(form.compressionId);
        mResultCode = ResultCode_UnsupportedValue;
        return false;
    }

    // load compression header
    std::unique_ptr<uint16[]> lineLengths;
    std::unique_ptr<uint32[]> channelLengths;
    if (form.compressionId == 1)
    {
        const int count = header.channels * header.height;
        lineLengths.reset(new uint16[count]);
        channelLengths.reset(new uint32[header.channels]);
        for (int i = 0; i < header.channels; ++i)
        {
            channelLengths[i] = 0;
        }

        for (int i = 0; i < count; ++i)
        {
            uint16 length = readUInt16();
            lineLengths[i] = length;
            if(length < 2 || length > 2 * rowBytes)
            {
                mSection = "image data/scanline length";
                mValue = std::to_string(length);
                mResultCode = ResultCode_InvalidValue;
                return false;
            }
            channelLengths[i / header.height] += length;
        }
    }

    // each channel
    for (int i = 0; i < header.channels; ++i)
    {
        PSDFormat::Channel* channel = new PSDFormat::Channel();
        channel->id = i;
        if (modeChannelCount > 0 && i >= modeChannelCount)
        {
            channel->id = modeChannelCount - (i + 1); // transparency
        }
        channel->compressionId = form.compressionId;

        if (form.compressionId == 0)
        {
            // raw image
            channel->dataLength = rowBytes * header.height;
            channel->data.reset(new uint8[channel->dataLength]);
            readBuf(channel->data.get(), channel->dataLength);
        }
        else if (form.compressionId == 1)
        {
            // rle compression image
            const uint32 compHeaderLength = header.height * sizeof(uint16);
            channel->dataLength = compHeaderLength + channelLengths[i];
            channel->data.reset(new uint8[channel->dataLength]);

            // copy compression header
            for (int k = 0; k < (int)header.height; ++k)
            {
                uint16 length = lineLengths[header.height * i + k];
                channel->data[2 * k    ] = (uint8)((length & 0xff00) >> 8);
                channel->data[2 * k + 1] = (uint8)(length & 0x00ff);
            }

            // read compression data
            readBuf(channel->data.get() + compHeaderLength, channelLengths[i]);
        }
        if (checkFailure()) return false;

        form.channels.push_back(PSDFormat::ChannelPtr(channel));
    }

    return true;
}

const std::string PSDReader::resultMessage() const
{
    return resultCodeString() + ". (" + mSection + ") '" + mValue + "'";
}

const std::string PSDReader::resultCodeString() const
{
    switch (mResultCode)
    {
    case ResultCode_Success: return "success";

    case ResultCode_InvalidFileHandle: return "invalid file handle";
    case ResultCode_InvalidSignature: return "invalid signature";
    case ResultCode_InvalidValue: return "invalid value";

    case ResultCode_UnsupportedVersion: return "unsupported version";
    case ResultCode_UnsupportedValue: return "unsupported value";

    case ResultCode_UnexpectedEndOfFile: return "unexpected end of file";
    case ResultCode_UnexpectedEndOfSection: return "unexpected end of section";

    case ResultCode_UnknownValue: return "unknown value";

    case ResultCode_TERM: return "no result";
    default:
        XC_ASSERT(0);
        return "";
    }
}

bool PSDReader::checkFailure()
{
    if(isFailed())
    {
        mResultCode = ResultCode_UnexpectedEndOfFile;
        return true;
    }
    return false;
}

void PSDReader::skipPads(uint32 aDataSize, uint32 aAlign)
{
    if (aDataSize % aAlign != 0)
    {
        skip(aAlign - aDataSize % aAlign);
    }
}

} // namespace img
