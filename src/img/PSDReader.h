#ifndef IMG_PSDREADER_H
#define IMG_PSDREADER_H

#include <memory>
#include "XC.h"
#include "util/StreamReader.h"
#include "img/PSDFormat.h"

namespace img
{

class PSDReader : public util::BEStreamReader
{
public:
    enum ResultCode
    {
        ResultCode_Success,
        ResultCode_InvalidFileHandle,
        ResultCode_InvalidSignature,
        ResultCode_InvalidValue,
        ResultCode_UnsupportedVersion,
        ResultCode_UnsupportedValue,
        ResultCode_UnexpectedEndOfFile,
        ResultCode_UnexpectedEndOfSection,
        ResultCode_UnknownValue,
        ResultCode_TERM
    };

    PSDReader(std::istream& aIo);

    // you can move the format.
    std::unique_ptr<PSDFormat>& format() { return mFormat; }
    const std::unique_ptr<PSDFormat>& format() const { return mFormat; }

    ResultCode resultCode() const { return mResultCode; }
    const std::string resultMessage() const;
    const std::string resultCodeString() const;

private:
    bool loadHeader();
    bool loadColorModeData();
    bool loadImageResources();
    bool loadLayerAndMaskInfo();
    bool loadAdditionalLayerInfo(std::list<PSDFormat::AdditionalLayerInfoPtr>& aList, PSDFormat::Layer* aLayer, std::ios::pos_type aEndPos);
    bool loadImageData();
    bool checkFailure();
    void skipPads(uint32 aDataSize, uint32 aAlign);

    std::unique_ptr<PSDFormat> mFormat;
    ResultCode mResultCode;
    std::string mSection;
    std::string mValue;
};

} // namespace img

#endif // IMG_PSDREADER_H
