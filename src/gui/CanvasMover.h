#ifndef GUI_CANVASMOVER_H
#define GUI_CANVASMOVER_H

#include "core/CameraInfo.h"

namespace gui
{

class CanvasMover
{
public:
    CanvasMover();

    void setCamera(core::CameraInfo* aCamera);
    void onScreenResized();

    void setDragAndMove(bool aIsActive);
    void setDragAndRotate(bool aIsActive);

    bool updateByMove(const QVector2D& aCursorPos, const QVector2D& aMoved,
                      bool aPressedL, bool aPressedR);
    bool updateByWheel(const QVector2D& aCursorPos, int aDelta);

    void rotate(float aRotateRad);
    void resetRotation();

private:
    core::CameraInfo* mCamera;
    bool mMoving;
    bool mRotating;
    bool mResetRotationOrigin;
    float mOriginDraggingAngle;
    QVector2D mOriginCanvasCenter;
    float mOriginCanvasAngle;
    int mScaleIndex;
};

} // namespace gui

#endif // GUI_CANVASMOVER_H
