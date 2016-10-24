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
    resNode->data().grabImage(image, aInOutRect.size(), img::Format_RGBA8);
    resNode->data().setPos(aInOutRect.topLeft());
    resNode->data().setIsLayer(true);
    resNode->data().setBlendMode(img::getBlendModeFromPSD(aLayer.blendMode));
    return resNode;
}

img::ResourceNode* createLayerSetResource(const QString& aName, const QPoint& aPos)
{
    auto resNode = new img::ResourceNode(aName);
    resNode->data().setPos(aPos);
    resNode->data().setIsLayer(false);
    return resNode;
}

bool ImageFileLoader::loadPsd(
        core::Project& aProject,
        util::IProgressReporter& aReporter)
{
    using img::PSDFormat;
    using img::PSDReader;
    using img::PSDUtil;
    typedef PSDFormat::LayerList::reverse_iterator ReverseIterator;

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
    topNode->setDefaultOpacity(1.0f);
    aProject.objectTree().grabTopNode(topNode);

    // tree stack
    std::vector<LayerSetNode*> treeStack;
    treeStack.push_back(topNode);
    float globalDepth = 0.0f;

    // resource tree stack
    std::vector<img::ResourceNode*> resStack;
    resStack.push_back(createLayerSetResource("topnode", QPoint(0, 0)));
    aProject.resourceHolder().pushImageTree(*resStack.back(), mFileInfo.absoluteFilePath());

    // for each layer
    for (ReverseIterator itr = layers.rbegin(); itr != layers.rend(); ++itr)
    {
        LayerSetNode* current = treeStack.back();
        XC_PTR_ASSERT(current);
        img::ResourceNode* resCurrent = resStack.back();
        XC_PTR_ASSERT(resCurrent);

        PSDFormat::Layer& layer = *((*itr).get());
        const QString name = textFilter.get(layer.name);
        QRect rect(layer.rect.left(), layer.rect.top(),
                   layer.rect.width(), layer.rect.height());
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
            layerNode->setClipped(layer.clipping != 0);
            layerNode->setInitialRect(rect);
            layerNode->setDefaultImage(resNode->handle());
            layerNode->setDefaultOpacity(layer.opacity / 255.0f);

            current->children().pushBack(layerNode);

            // update depth
            globalDepth -= 1.0f;
        }
        else if (layer.entryType == PSDFormat::LayerEntryType_Bounding)
        {
            // create bounding box
            current->setInitialRect(calculateBoundingRectFromChildren(*current));

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
            layerSetNode->setDefaultOpacity(layer.opacity / 255.0f);

            // push tree
            current->children().pushBack(layerSetNode);
            treeStack.push_back(layerSetNode);

            // update depth
            globalDepth -= 1.0f;
        }

        ++progress;
        aReporter.setProgress(progress);
    }
    topNode->setInitialRect(calculateBoundingRectFromChildren(*topNode));

    setDefaultPositions(*topNode);

    XC_DEBUG_REPORT("------------------------------------------");

    mLog = "success";
    return true;
}

QRect ImageFileLoader::calculateBoundingRectFromChildren(const ObjectNode& aNode)
{
    QRect rect;
    for (auto child : aNode.children())
    {
        if (child->initialRect().isValid())
        {
            rect = rect.isValid() ? rect.united(child->initialRect()) : child->initialRect();
        }
    }
    return rect;
}

void ImageFileLoader::setDefaultPositions(ObjectNode& aNode)
{
    ObjectNode::Iterator itr(&aNode);
    while (itr.hasNext())
    {
        auto node = itr.next();
        auto parent = node->parent();

        // parent position
        const QVector2D parentPos = (parent && parent->initialRect().isValid()) ?
                    util::MathUtil::getCenter(parent->initialRect()) : QVector2D();

        // node position
        const QVector2D pos = (node->initialRect().isValid()) ?
                    util::MathUtil::getCenter(node->initialRect()) : parentPos;

        // set
        if (node->type() == ObjectType_Layer)
        {
            ((LayerNode*)node)->setDefaultPos(pos - parentPos);
        }
        else if (node->type() == ObjectType_LayerSet)
        {
            ((LayerSetNode*)node)->setDefaultPos(pos - parentPos);
        }
    }
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
