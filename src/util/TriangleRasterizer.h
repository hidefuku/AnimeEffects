#ifndef UTIL_TRIANGLERASTERIZER_H
#define UTIL_TRIANGLERASTERIZER_H

#include <QVector2D>

namespace util
{

class TriangleRasterizer
{
    QVector2D mVtx1;
    QVector2D mVtx2;
    QVector2D mVtx3;
    QVector2D mVtx4;
    int mState;
    float mHeight;
    float mCurrent;
    float mSlope1;
    float mSlope2;
public:
    struct ScanLine
    {
        int xbgn;
        int xend;
        int y;
    };

    TriangleRasterizer(const QVector2D& aVtx1,
                       const QVector2D& aVtx2,
                       const QVector2D& aVtx3);

    inline bool hasNext() const
    {
        return mState >= 0;
    }

    inline ScanLine nextLine()
    {
        ScanLine result;

        if (mState == 0)
        {
            const float l = mCurrent * mSlope1 / mHeight;
            const float r = mCurrent * mSlope2 / mHeight;
            result.xbgn = (int)(mVtx1.x() + l);
            result.xend = (int)(mVtx1.x() + r) + 1;
            result.y = (int)(mVtx1.y() + mCurrent);

            mCurrent += 1.0f;

            if (mCurrent > mHeight)
            {
                if (mVtx2.y() == mVtx3.y())
                {
                    mState = -1;
                    return result;
                }

                mState = 1;
                mHeight = mVtx3.y() - mVtx2.y();
                mSlope1 = (mVtx2.x() - mVtx3.x());
                mSlope2 = (mVtx4.x() - mVtx3.x());
                mCurrent = 0;
            }
        }
        else if (mState == 1)
        {
            const float invCurrent = mHeight - mCurrent;
            const float l = invCurrent * mSlope1 / mHeight;
            const float r = invCurrent * mSlope2 / mHeight;

            result.xbgn = (int)(mVtx3.x() + l);
            result.xend = (int)(mVtx3.x() + r) + 1;
            result.y = (int)(mVtx2.y() + mCurrent);

            mCurrent += 1.0f;

            if (mCurrent > mHeight)
            {
                mState = -1;
            }
        }

        return result;
    }

};

} // namespace util

#endif // UTIL_TRIANGLERASTERIZER_H
