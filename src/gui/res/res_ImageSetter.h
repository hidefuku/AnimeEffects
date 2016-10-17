#ifndef GUI_RES_IMAGESETTER
#define GUI_RES_IMAGESETTER

#include "cmnd/Stack.h"
#include "img/ResourceNode.h"

namespace gui {
namespace res {

class ImageSetter : public cmnd::Stable
{
    img::ResourceNode& mNode;
    XCMemBlock mBlock;
    QRect mRect;
    img::ResourceHandle mAnotherHandle;
    bool mDoneOnce;

public:
    ImageSetter(img::ResourceNode& aNode, const XCMemBlock& aBlock, const QRect& aRect)
        : mNode(aNode)
        , mBlock(aBlock)
        , mRect(aRect)
        , mAnotherHandle()
        , mDoneOnce(false)
    {
        XC_PTR_ASSERT(aBlock.data);
    }

    ~ImageSetter()
    {
        if (!mDoneOnce)
        {
            delete [] mBlock.data;
        }
    }

    void exec()
    {
        mAnotherHandle = mNode.handle();

        mNode.resetData();
        mNode.data() = *mAnotherHandle;
        mNode.data().setPos(mRect.topLeft());
        mNode.data().grabImage(mBlock, mRect.size(), img::Format_RGBA8);
        mDoneOnce = true;
    }

    void redo()
    {
        mNode.swapData(mAnotherHandle);
    }

    void undo()
    {
        mNode.swapData(mAnotherHandle);
    }
};

} // namespace res
} // namespace gui

#endif // GUI_RES_IMAGESETTER

