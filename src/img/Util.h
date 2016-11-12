#ifndef IMG_UTIL_H
#define IMG_UTIL_H

#include <QSize>
#include <QRect>
#include <QColor>
#include <QImage>
#include "XC.h"
#include "util/TextUtil.h"
#include "img/PSDFormat.h"
#include "img/ResourceNode.h"

namespace img
{

class Util
{
public:
    static bool isShiftJisCode(const img::PSDFormat& aFormat);

    class TextFilter
    {
    public:
        TextFilter(const img::PSDFormat& aFormat);
        QString get(const std::string& aText) const;
    private:
        util::TextUtil::ShiftJisDecoder mShiftJisDecoder;
        bool mIsShiftJis;
    };

    /// @note each width and height increase 2 pixel
    static XCMemBlock recreateForBiLinearSampling(XCMemBlock& aGrabbedImage, const QSize& aSize);

    static void setEdgeColor(
            uint8* aImage, const QSize& aSize, const QColor& aColor);

    static void copyImage(
            uint8* aDst, const QSize& aDstSize, const QPoint& aTopLeft,
            const uint8* aSrc, const QSize& aSrcSize);

    static std::pair<XCMemBlock, QRect> createTextureImage(
            const PSDFormat::Header& aHeader,
            const PSDFormat::Layer& aLayer);
    static std::pair<XCMemBlock, QRect> createTextureImage(const QImage& aImage);

    static ResourceNode* createResourceNodes(PSDFormat& aFormat, bool aLoadImage);
    static ResourceNode* createResourceNode(const QImage& aImage, const QString& aName, bool aLoadImage);
};

} // namespace img

#endif // IMG_UTIL_H
