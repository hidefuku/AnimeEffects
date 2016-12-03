#include <QRegExp>
#include <fstream>
#include <QDebug>
#include "XC.h"
#include "util/TextUtil.h"
#include "img/PSDReader.h"
#include "img/PSDUtil.h"
#include "img/ResourceNode.h"
#include "img/Util.h"
#include "img/BlendMode.h"
#include "core/LayerNode.h"
#include "core/FolderNode.h"
#include "core/HeightMap.h"
#include "core/ObjectNodeUtil.h"
#include "ctrl/ImageFileLoader.h"

using namespace core;

namespace ctrl
{

//-------------------------------------------------------------------------------------------------
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

img::ResourceNode* createFolderResource(const QString& aName, const QPoint& aPos)
{
    auto resNode = new img::ResourceNode(aName);
    resNode->data().setPos(aPos);
    resNode->data().setIsLayer(false);
    return resNode;
}

FolderNode* createTopNode(const QString& aName, const QRect& aInitialRect)
{
    // create tree top node
    FolderNode* node = new FolderNode(aName);
    node->setInitialRect(aInitialRect);
    node->setDefaultOpacity(1.0f);
    node->setDefaultPosture(QVector2D());
    return node;
}

//-------------------------------------------------------------------------------------------------
ImageFileLoader::ImageFileLoader(const gl::DeviceInfo& aDeviceInfo)
    : mLog()
    , mFileInfo()
    , mGLDeviceInfo(aDeviceInfo)
    , mCanvasSize(512, 512)
    , mForceCanvasSize(false)
{
}

void ImageFileLoader::setCanvasSize(const QSize &aSize, bool aForce)
{
    if (aSize.width() <= 0 || aSize.height() <= 0) return;

    mCanvasSize = aSize;
    mForceCanvasSize = aForce;
}

bool ImageFileLoader::load(
        const QString& aPath,
        core::Project& aProject,
        util::IProgressReporter& aReporter)
{
    XC_DEBUG_REPORT("------------------------------------------");

    mFileInfo = QFileInfo(aPath);
    const QString suffix = mFileInfo.suffix();

    if (aPath.isEmpty() || !mFileInfo.isFile())
    {
        return createEmptyCanvas(aProject, "topnode", mCanvasSize);
    }
    else if (suffix == "psd")
    {
        return loadPsd(aProject, aReporter);
    }
    else
    {
        return loadImage(aProject, aReporter);
    }
}

//-------------------------------------------------------------------------------------------------
bool ImageFileLoader::createEmptyCanvas(core::Project& aProject, const QString& aTopName, const QSize& aCanvasSize)
{
    // check the image has valid size as a texture.
    if (!checkTextureSizeError((uint32)aCanvasSize.width(), (uint32)aCanvasSize.height()))
    {
        mLog = "invalid canvas size";
        return false;
    }
    XC_DEBUG_REPORT("canvas size = (%d, %d)", aCanvasSize.width(), aCanvasSize.height());

    // set canvas size
    aProject.attribute().setImageSize(aCanvasSize);

    // create tree top node
    FolderNode* topNode = createTopNode(aTopName, QRect(QPoint(0, 0), aCanvasSize));
    aProject.objectTree().grabTopNode(topNode);

    mLog = "success";
    return true;
}

//-------------------------------------------------------------------------------------------------
bool ImageFileLoader::loadImage(
        core::Project& aProject,
        util::IProgressReporter& aReporter)
{
    aReporter.setSection("Loading the Image File...");
    aReporter.setMaximum(1);
    aReporter.setProgress(0);

    QImage image(mFileInfo.filePath());
    if (image.isNull())
    {
        mLog = "Failed to load image file";
        return false;
    }

    auto size = mForceCanvasSize ? mCanvasSize : image.size();
    auto name = mFileInfo.baseName();

    if (!createEmptyCanvas(aProject, name, size))
    {
        return false;
    }

    {
        auto topNode = aProject.objectTree().topNode();
        XC_PTR_ASSERT(topNode);

        // resource tree stack
        img::ResourceNode* resTree = createFolderResource("topnode", QPoint(0, 0));
        aProject.resourceHolder().pushImageTree(*resTree, mFileInfo.absoluteFilePath());

        // create layer resource (Note that the rect be modified.)
        auto resNode = img::Util::createResourceNode(image, name, true);
        resTree->children().pushBack(resNode);

        // create layer node
        LayerNode* layerNode = new LayerNode(name, aProject.objectTree().shaderHolder());
        layerNode->setInitialRect(resNode->data().rect());
        layerNode->setDefaultImage(resNode->handle());
        layerNode->setDefaultOpacity(1.0f);
        layerNode->setDefaultPosture(resNode->data().center());
        topNode->children().pushBack(layerNode);
    }

    aReporter.setProgress(1);
    mLog = "success";
    return true;
}

//-------------------------------------------------------------------------------------------------
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

    // open file
    QScopedPointer<std::ifstream> file;
    {
        auto path = mFileInfo.filePath();
        file.reset(new std::ifstream(path.toLocal8Bit(), std::ios::binary));
        XC_DEBUG_REPORT() << "image path =" << path;

        if (file->fail())
        {
            mLog = "Can not find a file.";
            return false;
        }
    }

    // read psd
    PSDReader reader(*file);

    if (reader.resultCode() != PSDReader::ResultCode_Success)
    {
        mLog = "error(" + QString::number(reader.resultCode()) + ") " +
                QString::fromStdString(reader.resultMessage());
        return false;
    }
    aReporter.setProgress(1);
    file->close(); // do not use any more

    // update reporter
    aReporter.setSection("Building a Object Tree...");
    aReporter.setMaximum(reader.format()->layerAndMaskInfo().layerCount);
    aReporter.setProgress(0);
    int progress = 0;

    // build tree by a psd format
    std::unique_ptr<PSDFormat>& format = reader.format();
    PSDFormat::LayerList& layers = format->layerAndMaskInfo().layers;

    img::Util::TextFilter textFilter(*format);

    auto canvasSize =
            mForceCanvasSize ?
                mCanvasSize :
                QSize((int)format->header().width, (int)format->header().height);

    // check the image has valid size as a texture.
    if (!checkTextureSizeError((uint32)canvasSize.width(), (uint32)canvasSize.height()))
    {
        mLog = "invalid canvas size";
        return false;
    }
    XC_DEBUG_REPORT("image size = (%d, %d)", canvasSize.width(), canvasSize.height());

    aProject.attribute().setImageSize(canvasSize);

    // create tree top node
    FolderNode* topNode = createTopNode(mFileInfo.baseName(), QRect(QPoint(), canvasSize));
    aProject.objectTree().grabTopNode(topNode);

    // tree stack
    std::vector<FolderNode*> treeStack;
    treeStack.push_back(topNode);
    float globalDepth = 0.0f;

    // resource tree stack
    std::vector<img::ResourceNode*> resStack;
    resStack.push_back(createFolderResource("topnode", QPoint(0, 0)));
    aProject.resourceHolder().pushImageTree(*resStack.back(), mFileInfo.absoluteFilePath());

    // for each layer
    for (ReverseIterator itr = layers.rbegin(); itr != layers.rend(); ++itr)
    {
        FolderNode* current = treeStack.back();
        XC_PTR_ASSERT(current);
        img::ResourceNode* resCurrent = resStack.back();
        XC_PTR_ASSERT(resCurrent);

        PSDFormat::Layer& layer = *((*itr).get());
        const QString name = textFilter.get(layer.name);
        QRect rect(layer.rect.left(), layer.rect.top(),
                   layer.rect.width(), layer.rect.height());
        const float parentDepth = ObjectNodeUtil::getInitialWorldDepth(*current);

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
            layerNode->setVisibility(layer.isVisible());
            layerNode->setClipped(layer.clipping != 0);
            layerNode->setInitialRect(rect);
            layerNode->setDefaultImage(resNode->handle());
            layerNode->setDefaultDepth(globalDepth - parentDepth);
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
            // create folder resource
            auto resNode = createFolderResource(name, rect.topLeft());
            resCurrent->children().pushBack(resNode);
            resStack.push_back(resNode);

            // create folder node
            FolderNode* folderNode = new FolderNode(name);
            folderNode->setVisibility(layer.isVisible());
            folderNode->setClipped(layer.clipping != 0);
            folderNode->setDefaultDepth(globalDepth - parentDepth);
            folderNode->setDefaultOpacity(layer.opacity / 255.0f);

            // push tree
            current->children().pushBack(folderNode);
            treeStack.push_back(folderNode);

            // update depth
            globalDepth -= 1.0f;
        }

        ++progress;
        aReporter.setProgress(progress);
    }

    // setup default positions
    setDefaultPosturesFromInitialRects(*topNode);

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

void ImageFileLoader::setDefaultPosturesFromInitialRects(ObjectNode& aNode)
{
    ObjectNode::Iterator itr(&aNode);
    while (itr.hasNext())
    {
        auto node = itr.next();
        auto parent = node->parent();
        const bool isTop = !parent;
        const bool parentIsTop = parent ? !parent->parent() : false;

        QVector2D pos;
        QVector2D parentPos;
        if (!isTop)
        {
            // parent position
            parentPos = (parent->initialRect().isValid() && !parentIsTop) ?
                        util::MathUtil::getCenter(parent->initialRect()) : QVector2D();

            // node position
            pos = (node->initialRect().isValid()) ?
                        util::MathUtil::getCenter(node->initialRect()) : parentPos;

        }

        // set
        if (node->type() == ObjectType_Layer)
        {
            ((LayerNode*)node)->setDefaultPosture(pos - parentPos);
        }
        else if (node->type() == ObjectType_Folder)
        {
            ((FolderNode*)node)->setDefaultPosture(pos - parentPos);
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
