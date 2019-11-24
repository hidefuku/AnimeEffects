#include "XC.h"
#include "util/MathUtil.h"
#include "gui/CanvasMover.h"

namespace
{
static const int kWheelValue = 120;
static const int kMinScaleIndex = -30 * kWheelValue;
static const int kMaxScaleIndex =  30 * kWheelValue;
static const float kMinScale = 0.00001f;
}

namespace gui
{

CanvasMover::CanvasMover()
    : mCamera()
    , mMoving()
    , mRotating()
    , mResetRotationOrigin(true)
    , mOriginDraggingAngle()
    , mOriginCanvasCenter()
    , mOriginCanvasAngle()
    , mScaleIndex(0)
{
}

void CanvasMover::setCamera(core::CameraInfo* aCamera)
{
    mCamera = aCamera;
    mResetRotationOrigin = true;

    // calculate scale index
    if (mCamera)
    {
        auto scale = mCamera->scale();
        auto workScale = 1.0f;

        mScaleIndex = 0;

        if (scale > 1.0f)
        {
            while (scale > workScale && mScaleIndex < kMaxScaleIndex)
            {
                mScaleIndex += kWheelValue;
                workScale = (float)std::pow(1.1, (double)mScaleIndex / kWheelValue);
            }
        }
        else if (scale < 1.0f)
        {
            while (scale < workScale && mScaleIndex > kMinScaleIndex)
            {
                mScaleIndex -= kWheelValue;
                workScale = (float)std::pow(0.9, -(double)mScaleIndex / kWheelValue);
            }
        }
        mScaleIndex = xc_clamp(mScaleIndex, kMinScaleIndex, kMaxScaleIndex);
    }
}

void CanvasMover::onScreenResized()
{
    mResetRotationOrigin = true;
}

void CanvasMover::setDragAndMove(bool aIsActive)
{
    if (!mMoving && aIsActive)
    {
        mResetRotationOrigin = true;
    }
    mMoving = aIsActive;
}

void CanvasMover::setDragAndRotate(bool aIsActive)
{
    if (!mRotating && aIsActive)
    {
        mResetRotationOrigin = true;
    }
    mRotating = aIsActive;
}

bool CanvasMover::updateByMove(const QVector2D& aCursorPos, const QVector2D& aMoved,
                               bool aPressedL, bool aPressedM, bool aPressedR)
{
    if (mCamera)
    {
        // translate canvas
        if (mMoving && (aPressedL || aPressedM))
        {
            mCamera->setCenter(mCamera->center() + aMoved);
            mResetRotationOrigin = true;
            return true;
        }
        else if (mMoving || mRotating)
        { // rotate canvas

            const bool isPressed = mMoving ? aPressedR : (aPressedL || aPressedM);

            if (isPressed)
            {
                auto scrCenter = mCamera->screenCenter();
                auto cursorPos = aCursorPos;
                auto prevCursorPos = cursorPos - aMoved;

                if (mResetRotationOrigin)
                {
                    mResetRotationOrigin = false;
                    mOriginDraggingAngle = util::MathUtil::getAngleRad(prevCursorPos - scrCenter);
                    mOriginCanvasCenter = mCamera->center();
                    mOriginCanvasAngle = mCamera->rotate();
                }

                auto draggingAngle = util::MathUtil::getAngleRad(cursorPos - scrCenter);
                auto rotate = draggingAngle - mOriginDraggingAngle;
                auto offset = util::MathUtil::getRotateVectorRad(mOriginCanvasCenter - scrCenter, rotate);

                mCamera->setRotate(util::MathUtil::normalizeAngleRad(mOriginCanvasAngle + rotate));
                mCamera->setCenter(scrCenter + offset);
                return true;
            }
            else
            {
                mResetRotationOrigin = true;
            }
        }
    }
    return false;
}

bool CanvasMover::updateByWheel(const QVector2D& aCursorPos, int aDelta, bool aInvertScaling)
{
    if (mCamera)
    {
        auto center = mCamera->center();
        auto preScale = std::max(mCamera->scale(), kMinScale);
        auto scale = preScale;
        auto delta = aInvertScaling ? -aDelta : aDelta;

        mScaleIndex = xc_clamp(mScaleIndex - delta, kMinScaleIndex, kMaxScaleIndex);

        if (mScaleIndex > 0)
        {
            scale = (float)std::pow(1.1, (double)mScaleIndex / kWheelValue);
        }
        else if (mScaleIndex < 0)
        {
            scale = (float)std::pow(0.9, -(double)mScaleIndex / kWheelValue);
        }
        else
        {
            scale = 1.0f;
        }

        mCamera->setScale(scale);
        mCamera->setCenter((scale / preScale) * (center - aCursorPos) + aCursorPos);
        mResetRotationOrigin = true;

        return true;
    }
    return false;
}

void CanvasMover::rotate(float aRotateRad)
{
    if (mCamera)
    {
        auto scrCenter = mCamera->screenCenter();
        auto offset = util::MathUtil::getRotateVectorRad(mCamera->center() - scrCenter, aRotateRad);

        mCamera->setRotate(util::MathUtil::normalizeAngleRad(mCamera->rotate() + aRotateRad));
        mCamera->setCenter(scrCenter + offset);

        mResetRotationOrigin = true;
    }
}

void CanvasMover::resetRotation()
{
    if (mCamera)
    {
        mCamera->setRotate(0);
        mResetRotationOrigin = true;
    }
}

} // namespace gui
