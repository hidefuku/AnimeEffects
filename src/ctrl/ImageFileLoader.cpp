#include <QRegExp>
#include <fstream>
#include "XC.h"
#include "util/TextUtil.h"
#include "img/PSDReader.h"
#include "img/PSDUtil.h"
#include "img/ResourceNode.h"
#include "img/Util.h"
#include "img/BlendMode.h"
#include "core/LayerNode.h"
#include "core/LayerSetNode.h"
#include "core/HeightMap.h"
#include "core/ObjectNodeUtil.h"
#include "ctrl/ImageFileLoader.h"

using namespace core;

namespace ctrl
{

ImageFileLoader::ImageFileLoader()
    : mLog()
    , mFile()
    , mFileInfo()
    , mGLDeviceInfo()
{
}

bool ImageFileLoader::load(
        const QFileInfo& aPath, core::Project& aProject,
        const gl::DeviceInfo& aGLDeviceInfo,
        util::IProgressReporter& aReporter)
{
    XC_DEBUG_REPORT("------------------------------------------");

    mGLDeviceInfo = aGLDeviceInfo;

    mFileInfo = aPath;
    const QString filePath = aPath.filePath();
    mFile.reset(new std::ifstream(filePath.toLocal8Bit(), std::ios::binary));
    XC_DEBUG_REPORT() << "image path =" << filePath;

    if (mFile->fail())
    {
        mLog = "Can not find a file.";
        return false;
    }

    if (aPath.suffix() == "psd")
    {
        return loadPsd(aProject, aReporter);
    }
    else
    {
        mLog = "unknown format";
        return false;
    }
}

img::ResourceNode* createLayerResource(
        const img::PSDFormat::Header& aHeader,
        const img::PSDFormat::Layer& aLayer,
        const QString& aName, QRect& aInOutRect)
{
    using img::PSDFormat;
    using img::PSDUtil;

    const PSDUtil::ColorFormat colorFormat = PSDUtil::ColorFormat_RGBA8;

    // get image
    auto image = PSDUtil::makeInterleavedImage(aHeader, aLayer, colorFormat);

    // modulate color bit
    image = img::Util::recreateForBiLinearSampling(image, aInOutRect.size());
    aInOutRect.moveTopLeft(aInOutRect.topLeft() + QPoint(-1, -1));
    aInOutRect.setSize(aInOutRect.size() + QSize(2, 2));

    // create resource
    auto resNode = new img::ResourceNode(aName);
    resNode->grabImage(image, aInOutRect.size(), img::Format_RGBA8);
    resNode->setPos(aInOutRect.topLeft());
    resNode->setIsLayer(true);
    resNode->setBlendMode(img::getBlendModeFromPSD(aLayer.blendMode));
    return resNode;
}

img::ResourceNode* createLayerSetResource(const QString& aName, const QPoint& aPos)
{
    auto resNode = new img::ResourceNode(aName);
    resNode->setPos(aPos);
    resNode->setIsLayer(false);
    return resNode;
}

bool ImageFileLoader::loadPsd(
        core::Project& aProject,
        util::IProgressReporter& aReporter)
{
    using img::PSDFormat;
    using img::PSDReader;
    using img::PSDUtil;

    aReporter.setSection("Loading the PSD File...");
    aReporter.setMaximum(1);
    aReporter.setProgress(0);

    // read psd
    PSDReader reader(*mFile);

    if (reader.resultCode() != PSDReader::ResultCode_Success)
    {
        mLog = "error(" + QString::number(reader.resultCode()) + ") " +
                QString::fromStdString(reader.resultMessage());
        return false;
    }
    aReporter.setProgress(1);
    mFile->close(); // do not use any more

    aReporter.setSection("Building a Object Tree...");
    aReporter.setMaximum(reader.format()->layerAndMaskInfo().layerCount);
    aReporter.setProgress(0);
    int progress = 0;

    // build tree by a psd format
    std::unique_ptr<PSDFormat>& format = reader.format();
    PSDFormat::LayerList& layers = format->layerAndMaskInfo().layers;

    const uint32 imageWidth = format->header().width;
    const uint32 imageHeight = format->header().height;
    img::Util::TextFilter textFilter(*format);

    // check the image has valid size as a texture.
    if (!checkTextureSizeError(imageWidth, imageHeight))
    {
        return false;
    }

    XC_DEBUG_REPORT("image size = (%u, %u)", imageWidth, imageHeight);

    aProject.attribute().setImageSize(QSize(imageWidth, imageHeight));

    // create tree top node
    LayerSetNode* topNode = new LayerSetNode(mFileInfo.baseName());
    aProject.objectTree().grabTopNode(topNode);

    // tree stack
    std::vector<LayerSetNode*> treeStack;
    treeStack.push_back(topNode);
    QRect boundBox;
    float globalDepth = 0.0f;

    // resource tree stack
    std::vector<img::ResourceNode*> resStack;
    resStack.push_back(createLayerSetResource("topnode", QPoint(0, 0)));
    aProject.resourceHolder().pushImageTree(*resStack.back(), mFileInfo.absoluteFilePath());

    // for each layer
    PSDFormat::LayerList::reverse_iterator layerItr;
    for (layerItr = layers.rbegin(); layerItr != layers.rend(); ++layerItr)
    {
        LayerSetNode* current = treeStack.back();
        XC_PTR_ASSERT(current);
        img::ResourceNode* resCurrent = resStack.back();
        XC_PTR_ASSERT(resCurrent);

        PSDFormat::Layer& layer = *((*layerItr).get());
        const QString name = textFilter.get(layer.name);
        const PSDFormat::Rect frmRect = layer.rect;
        QRect rect(frmRect.left(), frmRect.top(), frmRect.width(), frmRect.height());
        const float parentDepth = core::ObjectNodeUtil::getGlobalDepth(*current);

        XC_REPORT() << "name =" << name << "size =" << rect.width() << "," << rect.height();

        // check the image has valid size as a texture.
        if (!checkTextureSizeError(rect.width(), rect.height()))
        {
            return false;
        }


        if (layer.entryType == PSDFormat::LayerEntryType_Layer)
        {
            // create layer resource (Note that the rect be modified.)
            auto resNode = createLayerResource(format->header(), layer, name, rect);
            resCurrent->children().pushBack(resNode);

            // create layer node
            LayerNode* layerNode = new LayerNode(name, aProject.objectTree().shaderHolder());
            layerNode->setDepth(globalDepth - parentDepth);
            layerNode->setVisibility(layer.isVisible());
            layerNode->setImage(resNode->handle());
            layerNode->setInitialCenter(util::MathUtil::getCenter(rect));
            layerNode->setClipped(layer.clipping != 0);

            current->children().pushBack(layerNode);

            // update depth
            globalDepth -= 1.0f;

            // update bounding box
            boundBox = boundBox.isValid() ? boundBox.united(rect) : rect;
        }
        else if (layer.entryType == PSDFormat::LayerEntryType_Bounding)
        {
            // set bounding box
            if (!boundBox.isValid())
            {
                boundBox = QRect(QPoint(0, 0), QSize(1, 1));
            }
            current->setBoundingRect(boundBox);
            current->setInitialCenter(util::MathUtil::getCenter(boundBox));

            // pop tree
            treeStack.pop_back();
            resStack.pop_back();
        }
        else
        {
            // create layerset resource
            auto resNode = createLayerSetResource(name, rect.topLeft());
            resCurrent->children().pushBack(resNode);
            resStack.push_back(resNode);

            // create layerset node
            LayerSetNode* layerSetNode = new LayerSetNode(name);
            layerSetNode->setDepth(globalDepth - parentDepth);
            layerSetNode->setVisibility(layer.isVisible());
            layerSetNode->setClipped(layer.clipping != 0);

            // push tree
            current->children().pushBack(layerSetNode);
            treeStack.push_back(layerSetNode);

            // update depth
            globalDepth -= 1.0f;

            // reset bounding box
            boundBox = QRect();
        }

        ++progress;
        aReporter.setProgress(progress);
    }

    XC_DEBUG_REPORT("------------------------------------------");

    mLog = "success";
    return true;
}

bool ImageFileLoader::checkTextureSizeError(uint32 aWidth, uint32 aHeight)
{
    const uint32 maxSize = (uint32)mGLDeviceInfo.maxTextureSize;

    if (maxSize < aWidth || maxSize < aHeight)
    {
        mLog = QString("The image size over the max texture size of your current device. ") +
        "image size(" + QString::number(aWidth) + ", " + QString::number(aHeight) + "), " +
        "max size(" + QString::number(maxSize) + ", " + QString::number(maxSize) + ")";
        return false;
    }
    return true;
}

} // namespace ctrl
