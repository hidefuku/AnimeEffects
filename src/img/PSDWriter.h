#ifndef IMG_PSDWRITER_H
#define IMG_PSDWRITER_H

#include <iostream>
#include "XC.h"
#include "img/PSDFormat.h"

namespace img
{

class PSDWriter
{
public:
    enum ResultCode
    {
        ResultCode_Success,
        ResultCode_InvalidFileHandle,
        ResultCode_InvalidValue,
        ResultCode_UnexpectedEndOfFile,
        ResultCode_TERM
    };

    PSDWriter(std::ostream& aOut, const PSDFormat& aFormat);

    ResultCode resultCode() const { return mResultCode; }
    const std::string resultMessage() const;
    const std::string resultCodeString() const;

private:
    bool writeHeader();
    bool writeColorModeData();
    bool writeImageResources();
    bool writeLayerAndMaskInfo();
    bool writeLayerInfo();
    bool writeImageData();
    bool checkFailure();

    template<typename tValue> void write(tValue aValue)
    {
        tValue value = XC_TO_BIG_ENDIAN(aValue);
        mOut.write((char*)(&value), sizeof(tValue));
    }
    template<typename tValue> void writeTo(std::ios::pos_type aPos, tValue aValue)
    {
        std::ios::pos_type current = mOut.tellp();
        mOut.seekp(aPos);
        write<tValue>(aValue);
        mOut.seekp(current);
    }
    void write(const uint8* aBuffer, int aLength) { mOut.write((const char*)aBuffer, aLength); }
    void writeZero(int aLength) { for (int i = 0; i < aLength; ++i) mOut.put((char)0); }

    std::ostream& mOut;
    const PSDFormat& mFormat;
    ResultCode mResultCode;
    std::string mSection;
    std::string mValue;
};

} // namespace img

#endif // IMG_PSDWRITER_H
