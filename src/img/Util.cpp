#include <string>
#include "util/TextUtil.h"
#include "img/Util.h"
#include "img/ColorRGBA.h"
#include "img/PSDUtil.h"
#include "img/BlendMode.h"

namespace img
{

bool Util::isShiftJisCode(const img::PSDFormat& aFormat)
{
    std::string nameset;
    for (const img::PSDFormat::LayerPtr& layer : aFormat.layerAndMaskInfo().layers)
    {
        nameset += layer->name + " ";
    }
    const float score = util::TextUtil::getShiftJisScore(nameset.c_str(), nameset.size());
    return score >= 0.5f;
}

Util::TextFilter::TextFilter(const PSDFormat& aFormat)
    : mShiftJisDecoder()
    , mIsShiftJis()
{
    mIsShiftJis = isShiftJisCode(aFormat);
}

QString Util::TextFilter::get(const std::string& aText) const
{
    return QString(mIsShiftJis ? mShiftJisDecoder.decode(aText.c_str()) : aText.c_str());
}

void Util::copyImage(
        uint8* aDst, const QSize& aDstSize, const QPoint& aTopLeft,
        const uint8* aSrc, const QSize& aSrcSize)
{
    const int t = aTopLeft.y();
    const int l = aTopLeft.x();
    const int sw = aSrcSize.width();
    const int sh = aSrcSize.height();
    const int dw = aDstSize.width();

    XC_ASSERT(l >= 0);
    XC_ASSERT(t >= 0);
    XC_ASSERT(l + sw <= dw);
    XC_ASSERT(t + sh <= aDstSize.height());

    const size_t copySize = sw * 4;

    // copy original to dest
    for (int y = 0; y < sh; ++y)
    {
        const uint8* copyHead = &aSrc[y * copySize];

        uint8* dstHead = &aDst[((y + t) * dw + l) * 4];
        memcpy(dstHead, copyHead, copySize);
    }
}

void Util::expandAlpha1Pixel(uint8* aImage, const QSize& aSize)
{
    auto w = aSize.width();
    auto h = aSize.height();

    // horizontal
    for (int y = 0; y < h; ++y)
    {
        auto row = &((ColorRGBA*)aImage)[w * y];
        auto prev = &row[0];
        auto prevA = prev->a();

        for (int x = 1; x < w; ++x)
        {
            auto curr = &row[x];
            auto currA = curr->a();

            if (prevA == 0)
            {
                if (currA > 0) prev->a() = currA;
            }
            else
            {
                if (currA == 0) curr->a() = prevA;
            }
            prev = curr;
            prevA = currA;
        }
    }
    // vertical
    for (int x = 0; x < w; ++x)
    {
        auto column = &((ColorRGBA*)aImage)[x];
        auto prev = &column[0];
        auto prevA = prev->a();

        for (int y = 1; y < h; ++y)
        {
            auto curr = &column[w * y];
            auto currA = curr->a();

            if (prevA == 0)
            {
                if (currA > 0) prev->a() = currA;
            }
            else
            {
                if (currA == 0) curr->a() = prevA;
            }
            prev = curr;
            prevA = currA;
        }
    }
}

XCMemBlock Util::recreateForBiLinearSampling(XCMemBlock& aGrabbedImage, const QSize& aSize)
{
    XC_PTR_ASSERT(aGrabbedImage.data);
    XC_ASSERT(aSize.width() > 0);
    XC_ASSERT(aSize.height() > 0);

    const int w = aSize.width() + 2;
    const int h = aSize.height() + 2;
    const size_t dstSize = w * h * 4;
    uint8* dstData = new uint8[dstSize];

    // copy original to dest
    copyImage(dstData, QSize(w, h), QPoint(1, 1), aGrabbedImage.data, aSize);

    // kill original
    delete [] aGrabbedImage.data;

    // clear edge
    setEdgeColor(dstData, QSize(w, h), QColor(0, 0, 0, 0));

    // set zero alpha color
    {
        ColorRGBA* colorHead = (ColorRGBA*)dstData;

        // horizontal
        for (int y = 0; y < h; ++y)
        {
            ColorRGBA* line = colorHead + y * w;
            ColorRGBA* prev = line;
            ColorRGBA* prev2 = prev;

            for (int x = 1; x < w; ++x)
            {
                ColorRGBA* color = (line + x);
                if (color->a() == 0)
                {
                    if (prev->a() > 0)
                    {
                        color->setRGB(*prev);
                    }
                    else if (prev2->a() > 0)
                    {
                        color->setRGB(*prev2);
                    }
                }
                else
                {
                    if (prev->a() == 0)
                    {
                        prev->setRGB(*color);

                        if (prev2->a() == 0)
                        {
                            prev2->setRGB(*color);
                        }
                    }
                }
                prev2 = prev;
                prev = color;
            }
        }

        // vertical
        for (int x = 0; x < w; ++x)
        {
            ColorRGBA* line = colorHead + x;
            ColorRGBA* prev = line;

            for (int y = 1; y < h; ++y)
            {
                ColorRGBA* color = line + y * w;
                if (color->a() == 0)
                {
                    if (prev->a() > 0) color->setRGB(*prev);
                }
                else
                {
                    if (prev->a() == 0) prev->setRGB(*color);
                }
                prev = color;
            }
        }
    }

    return XCMemBlock(dstData, dstSize);
}

void Util::setEdgeColor(uint8* aImage, const QSize& aSize, const QColor& aColor)
{
    ColorRGBA* color = (ColorRGBA*)aImage;
    const int w = aSize.width();
    const int h = aSize.height();

    // top, bottom
    for (int x = 0; x < w; ++x)
    {
        color[x].set(aColor);
        color[x + w * (h - 1)].set(aColor);
    }
    // left, right
    for (int y = 1; y < h - 1; ++y)
    {
        color[y * w].set(aColor);
        color[(y + 1) * w - 1].set(aColor);
    }
}

std::pair<XCMemBlock, QRect> Util::createTextureImage(
        const PSDFormat::Header& aHeader,
        const PSDFormat::Layer& aLayer)
{
    // get image
    auto image = PSDUtil::makeInterleavedImage(
                aHeader, aLayer, PSDUtil::ColorFormat_RGBA8);
    XC_PTR_ASSERT(image.data);

    QRect rect(aLayer.rect.left(), aLayer.rect.top(),
               aLayer.rect.width(), aLayer.rect.height());

    // modulate color bit
    image = recreateForBiLinearSampling(image, rect.size());
    rect.moveTopLeft(rect.topLeft() + QPoint(-1, -1));
    rect.setSize(rect.size() + QSize(2, 2));

    return std::pair<XCMemBlock, QRect>(image, rect);
}

std::pair<XCMemBlock, QRect> Util::createTextureImage(const QImage& aImage)
{
    const size_t length = (size_t)aImage.byteCount();

    XCMemBlock workImage;
    workImage.size = length;
    workImage.data = new uint8[length];
    memcpy(workImage.data, aImage.bits(), length);

    // modulate color bit
    auto image = recreateForBiLinearSampling(workImage, aImage.size());
    const QRect rect(QPoint(-1, -1), aImage.size() + QSize(2, 2));

    return std::pair<XCMemBlock, QRect>(image, rect);
}

ResourceNode* Util::createResourceNodes(PSDFormat& aFormat, bool aLoadImage)
{
    // build tree by a psd format
    PSDFormat::LayerList& layers = aFormat.layerAndMaskInfo().layers;
    Util::TextFilter textFilter(aFormat);

    // resource tree stack
    std::vector<ResourceNode*> resStack;
    resStack.push_back(new ResourceNode("topnode"));

    // each layer
    for (auto itr = layers.rbegin(); itr != layers.rend(); ++itr)
    {
        ResourceNode* resCurrent = resStack.back();
        XC_PTR_ASSERT(resCurrent);

        PSDFormat::Layer& layer = *((*itr).get());
        const QString name = textFilter.get(layer.name);
        QRect rect(layer.rect.left(), layer.rect.top(),
                   layer.rect.width(), layer.rect.height());

        if (layer.entryType == PSDFormat::LayerEntryType_Layer)
        {
            // create resource
            auto resNode = new ResourceNode(name);
            resNode->data().setPos(rect.topLeft());
            resNode->data().setUserData(&layer);
            resNode->data().setIsLayer(true);
            resNode->data().setBlendMode(getBlendModeFromPSD(layer.blendMode));

            if (aLoadImage)
            {
                auto image = createTextureImage(aFormat.header(), layer);
                resNode->data().setPos(image.second.topLeft());
                resNode->data().grabImage(image.first, image.second.size(), Format_RGBA8);
            }
            else
            {
                auto header = aFormat.header();
                auto layerPtr = &layer;

                resNode->data().setImageLoader([=](ResourceData& aData)->bool
                {
                    auto image = createTextureImage(header, *layerPtr);
                    aData.setPos(image.second.topLeft());
                    aData.grabImage(image.first, image.second.size(), Format_RGBA8);
                    return true;
                });
            }

            // push tree
            resCurrent->children().pushBack(resNode);
        }
        else if (layer.entryType == PSDFormat::LayerEntryType_Bounding)
        {
            // pop tree
            resStack.pop_back();
        }
        else
        {
            // create resource
            auto resNode = new ResourceNode(name);
            resNode->data().setPos(rect.topLeft());
            resNode->data().setUserData(&layer);
            resNode->data().setIsLayer(false);
            resCurrent->children().pushBack(resNode);

            // push tree
            resStack.push_back(resNode);
        }
    }
    return resStack.front();
}

ResourceNode* Util::createResourceNode(const QImage& aImage, const QString& aName, bool aLoadImage)
{
    // create resource
    auto resNode = new ResourceNode(aName);
    resNode->data().setPos(QPoint(0, 0));
    resNode->data().setIsLayer(true);
    resNode->data().setBlendMode(img::BlendMode_Normal);

    if (aLoadImage)
    {
        auto image = aImage.convertToFormat(QImage::Format_RGBA8888);
        auto texImage = createTextureImage(image);
        resNode->data().setPos(texImage.second.topLeft());
        resNode->data().grabImage(texImage.first, texImage.second.size(), Format_RGBA8);
    }
    else
    {
        resNode->data().setImageLoader([=](ResourceData& aData)->bool
        {
            auto image = aImage.convertToFormat(QImage::Format_RGBA8888);
            auto texImage = createTextureImage(image);
            aData.setPos(texImage.second.topLeft());
            aData.grabImage(texImage.first, texImage.second.size(), Format_RGBA8);
            return true;
        });
    }
    return resNode;
}

} // namespace img
