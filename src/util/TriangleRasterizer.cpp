#include "util/TriangleRasterizer.h"

namespace util
{

TriangleRasterizer::TriangleRasterizer(
        const QVector2D& aVtx1,
        const QVector2D& aVtx2,
        const QVector2D& aVtx3)
    : mVtx1(aVtx1)
    , mVtx2(aVtx2)
    , mVtx3(aVtx3)
    , mVtx4()
    , mState(0)
    , mHeight()
    , mCurrent()
    , mSlope1()
    , mSlope2()
{
    if (mVtx1.y() > mVtx2.y()) std::swap(mVtx1, mVtx2);
    if (mVtx1.y() > mVtx3.y()) std::swap(mVtx1, mVtx3);
    if (mVtx2.y() > mVtx3.y()) std::swap(mVtx2, mVtx3);

    if (mVtx2.y() == mVtx3.y())
    { // bottom flat
        mVtx4 = mVtx3;
        mHeight = mVtx2.y() - mVtx1.y();
    }
    else if (mVtx1.y() == mVtx2.y())
    { // top flat
        mState = 1;
        mVtx4 = mVtx1;
        mHeight = mVtx3.y() - mVtx2.y();
    }
    else
    {
        mVtx4.setY(mVtx2.y());
        const float ratioY = (mVtx2.y() - mVtx1.y()) / (mVtx3.y() - mVtx1.y());
        mVtx4.setX(mVtx1.x() + ratioY * (mVtx3.x() - mVtx1.x()));
        mHeight = mVtx2.y() - mVtx1.y();
    }

    if (mVtx2.x() > mVtx4.x())
    {
        std::swap(mVtx2, mVtx4);
    }

    if (mHeight <= 0.0f)
    {
        mState = -1;
    }
    else if (mState == 0)
    {
        mSlope1 = mVtx2.x() - mVtx1.x();
        mSlope2 = mVtx4.x() - mVtx1.x();
    }
    else
    {
        mSlope1 = mVtx2.x() - mVtx3.x();
        mSlope2 = mVtx4.x() - mVtx3.x();
    }
}

} // namespace util
