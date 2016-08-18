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
    XCMemBlock mBlockPrev;
    QRect mRect;
    QRect mRectPrev;
    img::Format mFormatPrev;
    bool mDone;

public:
    ImageSetter(img::ResourceNode& aNode, const XCMemBlock& aBlock, const QRect& aRect)
        : mNode(aNode)
        , mBlock(aBlock)
        , mBlockPrev()
        , mRect(aRect)
        , mRectPrev()
        , mFormatPrev()
        , mDone(false)
    {
        XC_PTR_ASSERT(aBlock.data);
    }

    ~ImageSetter()
    {
        if (mDone)
        {
            if (mBlockPrev.data)
            {
                delete [] mBlockPrev.data;
            }
        }
        else
        {
            delete [] mBlock.data;
        }
    }

    void redo()
    {
        if (mNode.hasImage())
        {
            mFormatPrev = mNode.image().format();
            mRectPrev = QRect(mNode.pos(), mNode.image().pixelSize());
            mBlockPrev = mNode.releaseImage();
            XC_ASSERT(mBlockPrev.data != mBlock.data);
        }
        mNode.setPos(mRect.topLeft());
        mNode.grabImage(mBlock, mRect.size(), img::Format_RGBA8);
        mDone = true;
    }

    void undo()
    {
        mNode.releaseImage();
        if (mBlockPrev.data)
        {
            mNode.setPos(mRectPrev.topLeft());
            mNode.grabImage(mBlockPrev, mRectPrev.size(), mFormatPrev);
        }
        mDone = false;
    }
};

} // namespace res
} // namespace gui

#endif // GUI_RES_IMAGESETTER

