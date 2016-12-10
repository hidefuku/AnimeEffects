#ifndef CTRL_FFD_TASK_H
#define CTRL_FFD_TASK_H

#include <QMatrix4x4>
#include "util/Circle.h"
#include "gl/Vector2I.h"
#include "gl/Vector2.h"
#include "gl/Vector3.h"
#include "gl/Task.h"
#include "gl/BufferObject.h"
#include "gl/ComputeTexture1D.h"
#include "core/TimeKeyExpans.h"
#include "core/MeshTransformer.h"
#include "core/LayerMesh.h"
#include "ctrl/FFDParam.h"
#include "ctrl/ffd/ffd_TaskResource.h"

namespace ctrl {
namespace ffd {

// You can set a same buffer to src and dst.
class Task : public gl::Task
{
public:
    enum Type
    {
        Type_Deformer,
        Type_Eraser,
        Type_Focuser,
        Type_Dragger,
        Type_TERM
    };

    Task(TaskResource& aResource, core::MeshTransformerResource& aMeshRes);

    void setType(Type aType);
    void setDragIndex(int aIndex); // for Dragger

    void resetDst(int aVtxCount);
    void writeSrc(
            const core::TimeKeyExpans& aSrcExpans,
            const gl::Vector3* aSrcMesh,
            const core::LayerMesh& aOriginMesh,
            const FFDParam& aBrushParam);

    void setBrush(
            const QVector2D& aBrushCenter,
            const QVector2D& aBrushVel);

    gl::Vector3* dstMesh() const { return mDstMesh.data(); }
    size_t dstSize() const { return sizeof(gl::Vector3) * mVtxCount; }

    QVector2D dragMove() const { return mDragMove; } // for Dragger
    int focusIndex() const { return mFocusIndex; } // for Focuser

private:
    virtual void onRequested();
    virtual void onFinished();
    void requestBlur();
    gl::EasyShaderProgram& selectShaderProgram() const;

    TaskResource& mResource;

    Type mType;
    core::MeshTransformer mMeshTransformer;
    core::LayerMesh::MeshBuffer mMeshBuffer;
    util::ArrayBlock<const gl::Vector3> mSrcMesh;
    const core::TimeKeyExpans* mSrcExpans;
    QVector2D mSrcOriginOffset;
    core::LayerMesh::ArrayedConnectionList mArrayedConnectionList;
    gl::ComputeTexture1DList mSrcBlurPositions;

    gl::BufferObject mWorkInMesh;
    gl::BufferObject mWorkInWeight;
    gl::BufferObject mOutMesh;
    gl::BufferObject mOutWeight;

    const gl::Vector3* mOriginMesh;
    QScopedArrayPointer<gl::Vector3> mDstMesh;
    QScopedArrayPointer<GLfloat> mDstWeight;
    int mVtxCount;
    int mDstBufferCount;

    FFDParam mParam;
    QVector2D mBrushCenter;
    QVector2D mBrushVel;
    bool mUseBlur;

    int mFocusIndex;
    int mDragIndex;
    QVector2D mDragMove;
};

} // namespace ffd
} // namespace ctrl

#endif // CTRL_FFD_TASK_H
