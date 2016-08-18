#include "XC.h"
#include "img/PSDWriter.h"
#include "img/PSDUtil.h"

#define PSDWRITER_CHECK_VALIDVALUE(cond, value, section) \
    do { if (!(cond)) { mSection = section; mValue = std::to_string(value); mResultCode = ResultCode_InvalidValue; return false; } } while(0)

#define PSDWRITER_DUMP(...) XC_DEBUG_REPORT(__VA_ARGS__)
//#define PSDWRITER_DUMP(...)

namespace img
{

PSDWriter::PSDWriter(std::ostream& aOut, const PSDFormat& aFormat)
    : mOut(aOut)
    , mFormat(aFormat)
    , mResultCode(ResultCode_TERM)
    , mSection()
    , mValue()
{
    if (mOut.fail())
    {
        mSection = "output stream";
        mResultCode = ResultCode_InvalidFileHandle;
        return;
    }

    if (!writeHeader())
    {
        return;
    }
    PSDWRITER_DUMP("wrote header. tell: %d", (int)mOut.tellp());

    if (!writeColorModeData())
    {
        return;
    }
    PSDWRITER_DUMP("wrote color mode data. tell: %d", (int)mOut.tellp());

    if (!writeImageResources())
    {
        return;
    }
    PSDWRITER_DUMP("wrote image resources. tell: %d", (int)mOut.tellp());

    if (!writeLayerAndMaskInfo())
    {
        return;
    }
    PSDWRITER_DUMP("wrote layer and mask info. tell: %d", (int)mOut.tellp());

    if (!writeImageData())
    {
        return;
    }
    PSDWRITER_DUMP("wrote image data. tell: %d", (int)mOut.tellp());

    mSection = "end of file";
    mResultCode = ResultCode_Success;
}

bool PSDWriter::writeHeader()
{
    mSection = "header";

    // signature
    write((uint8*)"8BPS", 4);
    // version
    write(mFormat.header().version);
    // reserved
    writeZero(6);
    // channels
    write(mFormat.header().channels);
    // height
    write(mFormat.header().height);
    // width
    write(mFormat.header().width);
    // depth
    write(mFormat.header().depth);
    // mode
    write(mFormat.header().mode);

    return !checkFailure();
}

bool PSDWriter::writeColorModeData()
{
    mSection = "color mode data";

    // length
    write((uint32)0);

    return !checkFailure();
}

bool PSDWriter::writeImageResources()
{
    mSection = "image resources";

    // keep position of the 'length'
    std::ios::pos_type lengthPos = mOut.tellp();
    write((uint32)0);

    // image resource blocks
    for (const PSDFormat::ImageResourceBlockPtr& block : mFormat.imageResources().blocks)
    {
        // signature
        write((uint8*)"8BIM", 4);
        // id
        write(block->id);
        // name
        if (!block->name.empty())
        {
            const int nameLength = static_cast<int>(block->name.size());
            PSDWRITER_CHECK_VALIDVALUE(nameLength <= 255, block->name.size(), "image resource block/name");
            write((uint8)nameLength);
            write((uint8*)block->name.c_str(), nameLength);
            // padding
            if ((nameLength + 1) % 2) writeZero(1);
        }
        else
        {
            writeZero(2);
        }

        // data length
        write(block->dataLength);
        // data
        write(block->data.get(), block->dataLength);
        // padding
        if (block->dataLength % 2) writeZero(1);
    }

    // write the 'length'
    writeTo(lengthPos, (uint32)(mOut.tellp() - lengthPos - 4));
    PSDWRITER_DUMP("ir len : %u", (uint32)(mOut.tellp() - lengthPos - 4));
    return !checkFailure();
}

bool PSDWriter::writeLayerAndMaskInfo()
{
    mSection = "layer and mask info";

    const PSDFormat::LayerAndMaskInfo& form = mFormat.layerAndMaskInfo();

    // keep position of the 'length'
    std::ios::pos_type lengthPos = mOut.tellp();
    write((uint32)0);

    // check empty
    if (form.layers.empty() && !(bool)form.globalLayerMaskInfo && form.additionalLayerInfos.empty())
    {
        return !checkFailure();
    }

    // layer info
    if (!writeLayerInfo())
    {
        return false;
    }

    PSDWRITER_DUMP("global layer mask info : tell %d", (int)mOut.tellp());

    // global layer mask info
    if (form.globalLayerMaskInfo)
    {
        write((uint32)(13 + form.globalLayerMaskInfo->fillerCount));
        write(form.globalLayerMaskInfo->overlayColorSpace);
        write(form.globalLayerMaskInfo->colorComponent[0]);
        write(form.globalLayerMaskInfo->colorComponent[1]);
        write(form.globalLayerMaskInfo->opacity);
        write(form.globalLayerMaskInfo->kind);
        writeZero(form.globalLayerMaskInfo->fillerCount);

        if (checkFailure()) return false;
    }
    else
    {
        write((uint32)0);
    }

    // additional layer info
    for (const PSDFormat::AdditionalLayerInfoPtr& addiInfo : form.additionalLayerInfos)
    {
        const bool isOddLength = (addiInfo->dataLength % 2 != 0);
        write((const uint8*)"8BIM", 4);
        write((const uint8*)addiInfo->key.c_str(), 4);
        write(addiInfo->dataLength + (uint32)(isOddLength ? 1u : 0u));
        write(addiInfo->data.get(), addiInfo->dataLength);
        if (isOddLength) writeZero(1);

        if (checkFailure()) return false;
    }

    // write the 'length'
    writeTo(lengthPos, (uint32)(mOut.tellp() - lengthPos - 4));

    return !checkFailure();
}

bool PSDWriter::writeLayerInfo()
{
    const PSDFormat::LayerAndMaskInfo& form = mFormat.layerAndMaskInfo();

    // keep position of the 'layer info length'
    std::ios::pos_type layerInfoLengthPos = mOut.tellp();
    write((uint32)0);

    // layer count
    sint16 layerCount = (sint16)form.layers.size();
    if (mFormat.imageData().hasTransparency)
    {
        layerCount *= -1;
    }
    write(layerCount);

    // each layer
    for (const PSDFormat::LayerPtr& layer : form.layers)
    {
        // rectangle
        for (int i = 0; i < 4; ++i) write(layer->rect.edge[i]);
        // number of channels
        uint16 channelCount = (uint16)layer->channels.size();
        write(channelCount);
        // information of each channel
        for (const PSDFormat::ChannelPtr& channel : layer->channels)
        {
            write(channel->id);
            uint32 dataLength = channel->dataLength + 2u; // with compression id
            write(dataLength);
        }
        if (checkFailure()) return false;

        // blend mode signature
        write((const uint8*)"8BIM", 4);

        // blend mode key
        PSDWRITER_CHECK_VALIDVALUE(layer->blendMode.size() == 4, layer->blendMode.size(), "layer and mask info/layer info/layer records/blend mode key");
        write((uint8*)layer->blendMode.c_str(), 4);

        // opacity
        write(layer->opacity);
        // clipping
        PSDWRITER_CHECK_VALIDVALUE(layer->clipping == 0 || layer->clipping == 1, layer->clipping, "layer and mask info/layer info/layer records/clipping");
        write(layer->clipping);
        // flags
        write(layer->flags);
        // filler
        writeZero(1);

        if (checkFailure()) return false;

        // keep position of the 'length of the extra data field'
        std::ios::pos_type extraDataLengthPos = mOut.tellp();
        write((uint32)0);

        // layer mask
        if (layer->mask)
        {
            write((uint32)(layer->mask->hasReal ? 36 : 20));
            // rectangle
            for (int i = 0; i < 4; ++i) write(layer->mask->rect.edge[i]);
            // default color
            PSDWRITER_CHECK_VALIDVALUE(layer->mask->defaultColor == 0 || layer->mask->defaultColor == 255, layer->mask->defaultColor, "layer and mask info/layer info/layer records/layer mask/default color");
            write(layer->mask->defaultColor);
            // flags
            write(layer->mask->flags);

            // real params
            if (layer->mask->hasReal)
            {
                write(layer->mask->realFlags);
                write(layer->mask->realUserMaskBG);
                for (int i = 0; i < 4; ++i) write(layer->mask->realUserRect.edge[i]);
            }
            else
            {
                writeZero(2);
            }
        }
        else
        {
            write((uint32)0);
        }
        if (checkFailure()) return false;

        // blending range
        if (layer->compositeBlendingRange)
        {
            // keep length pos
            std::ios::pos_type blendRangeLengthPos = mOut.tellp();
            write((uint32)0);

            // composite range
            if (layer->compositeBlendingRange->isValid)
            {
                write(layer->compositeBlendingRange->src, 4);
                write(layer->compositeBlendingRange->dst, 4);
            }
            else
            {
                write(PSDFormat::BlendingRange().src, 4);
                write(PSDFormat::BlendingRange().dst, 4);
            }
            // range of each channel
            for (const PSDFormat::ChannelPtr& channel : layer->channels)
            {
                if (channel->blendingRange.isValid)
                {
                    write(channel->blendingRange.src, 4);
                    write(channel->blendingRange.dst, 4);
                }
                else
                {
                    write(PSDFormat::BlendingRange().src, 4);
                    write(PSDFormat::BlendingRange().dst, 4);
                }
            }

            // write length
            writeTo(blendRangeLengthPos, (uint32)(mOut.tellp() - blendRangeLengthPos - 4));
        }
        else
        {
            write((uint32)0);
        }
        if (checkFailure()) return false;

        // layer name
        {
            int nameLength = (int)layer->name.size();
            PSDWRITER_CHECK_VALIDVALUE(nameLength <= 255, nameLength, "layer and mask info/layer info/layer record/layer name");
            write((uint8)nameLength);
            write((uint8*)layer->name.c_str(), nameLength);
            // padding
            if ((nameLength + 1) % 4) writeZero(4 - ((nameLength + 1) % 4));
        }
        if (checkFailure()) return false;

        // additional layer info
        for (PSDFormat::AdditionalLayerInfoPtr& addiInfo : layer->additionalInfos)
        {
            const bool isOddLength = (addiInfo->dataLength % 2 != 0);
            write((const uint8*)"8BIM", 4);
            write((const uint8*)addiInfo->key.c_str(), 4);
            write(addiInfo->dataLength + (uint32)(isOddLength ? 1u : 0u));
            write(addiInfo->data.get(), addiInfo->dataLength);
            if (isOddLength) writeZero(1);

            if (checkFailure()) return false;
        }
        // write the 'length of the extra data field'
        writeTo(extraDataLengthPos, (uint32)(mOut.tellp() - extraDataLengthPos - 4));
    }
    PSDWRITER_DUMP("pre cid : tell %d", (int)mOut.tellp());

    // channel image data
    for (const PSDFormat::LayerPtr& layer : form.layers)
    {
        for (const PSDFormat::ChannelPtr& channel : layer->channels)
        {
            write(channel->compressionId);
            write(channel->data.get(), channel->dataLength);
            if (checkFailure()) return false;
        }
    }
    PSDWRITER_DUMP("pre cid : tell %d", (int)mOut.tellp());

    // write the 'layer info length' with a padding
    uint32 layerInfoLength = (uint32)(mOut.tellp() - layerInfoLengthPos - 4);
    PSDWRITER_DUMP("test tell %d", layerInfoLength);

    if (layerInfoLength % 2)
    {
        writeZero(1);
        layerInfoLength += 1;
    }
    writeTo(layerInfoLengthPos, layerInfoLength);

    return !checkFailure();
}

bool PSDWriter::writeImageData()
{
    mSection = "image data";

    const uint32 rleHeaderLength = mFormat.header().height * sizeof(uint16);

    // compression id
    write(mFormat.imageData().compressionId);

    // compression header
    if (mFormat.imageData().compressionId == 1)
    {

        for (const PSDFormat::ChannelPtr& channel : mFormat.imageData().channels)
        {
            write(channel->data.get(), rleHeaderLength);
        }
    }

    // compression data
    for (const PSDFormat::ChannelPtr& channel : mFormat.imageData().channels)
    {
        if (mFormat.imageData().compressionId == 1)
        {
            write(channel->data.get() + rleHeaderLength, channel->dataLength - rleHeaderLength);
        }
        else
        {
            write(channel->data.get(), channel->dataLength);
        }
    }

    return !checkFailure();
}

bool PSDWriter::checkFailure()
{
    if (mOut.fail())
    {
        mResultCode = ResultCode_UnexpectedEndOfFile;
        return true;
    }
    return false;
}

const std::string PSDWriter::resultMessage() const
{
    return resultCodeString() + ". '" + mValue + "' (" + mSection + ")";
}

const std::string PSDWriter::resultCodeString() const
{
    switch (mResultCode)
    {
    case ResultCode_Success: return "success";

    case ResultCode_InvalidFileHandle: return "invalid file handle";
    case ResultCode_InvalidValue: return "invalid value";

    case ResultCode_UnexpectedEndOfFile: return "unexpected end of file";

    case ResultCode_TERM: return "no result";
    default:
        XC_ASSERT(0);
        return "";
    }
}

} // namespace img
