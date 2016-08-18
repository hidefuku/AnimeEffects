#ifndef CTRL_FFD_TASK_H
#define CTRL_FFD_TASK_H

#include <QMatrix4x4>
#include "util/Circle.h"
#include "gl/Vector2I.h"
#include "gl/Vector2.h"
#include "gl/Vector3.h"
#include "gl/Task.h"
#include "gl/BufferObject.h"
#include "gl/EasyShaderProgram.h"
#include "gl/ComputeTexture1D.h"
#include "core/TimeKeyExpans.h"
#include "core/MeshTransformer.h"
#include "core/LayerMesh.h"
#include "ctrl/FFDParam.h"

namespace ctrl {
namespace ffd {

// You can set a same buffer to src and dst.
class Task : public gl::Task
{
public:
    struct Resource
    {
        enum { kType = 2, kHardness = 3, kVariation = 6 };

        Resource();

        void setup(const QString& aBrushPath,
                   const QString& aEraserPath,
                   const QString& aBlurPath);
        gl::EasyShaderProgram& program(int aType, int aHard);
        const gl::EasyShaderProgram& program(int aType, int aHard) const;

        gl::EasyShaderProgram& blurProgram() { return mBlurProgram; }
        const gl::EasyShaderProgram& blurProgram() const { return mBlurProgram; }

    private:
        void loadFile(const QString& aPath, QString& aDstCode);
        void buildShader(
                gl::EasyShaderProgram& aProgram, const QString& aCode,
                int aType, int aHard);
        void buildBlurShader(
                gl::EasyShaderProgram& aProgram, const QString& aCode);

        gl::EasyShaderProgram mProgram[kVariation];
        gl::EasyShaderProgram mBlurProgram;
    };

    Task(Resource& aResource);

    void resetDst(int aVtxCount);
    void writeSrc(
            const core::TimeKeyExpans& aSrcExpans,
            const gl::Vector3* aSrcMesh,
            const core::LayerMesh& aOriginMesh,
            const FFDParam& aBrushParam);

    void setBrush(
            const QMatrix4x4& aWorldMtx,
            const QMatrix4x4& aWorldInvMtx,
            const QVector2D& aBrushCenter,
            const QVector2D& aBrushVel);

    gl::Vector3* dstMesh() const { return mDstMesh.data(); }
    size_t dstSize() const { return sizeof(gl::Vector3) * mVtxCount; }

private:
    virtual void onRequested();
    virtual void onFinished();
    void requestBlur();

    Resource& mResource;

    core::MeshTransformer mMeshTransformer;
    core::LayerMesh::MeshBuffer mMeshBuffer;
    util::ArrayBlock<const gl::Vector3> mSrcMesh;
    const core::TimeKeyExpans* mSrcExpans;
    core::LayerMesh::ArrayedConnectionList mArrayedConnectionList;
    gl::ComputeTexture1DList mSrcBlurPositions;

    gl::BufferObject mWorkInMesh;
    gl::BufferObject mWorkInWeight;
    gl::BufferObject mOutMesh;
    gl::BufferObject mOutWeight;

    const gl::Vector3* mOriginMesh;
    QScopedArrayPointer<gl::Vector3> mDstMesh;
    int mVtxCount;
    int mDstBufferCount;

    QMatrix4x4 mWorldMtx;
    QMatrix4x4 mWorldInvMtx;
    FFDParam mParam;
    QVector2D mBrushCenter;
    QVector2D mBrushVel;
    bool mUseBlur;
};

} // namespace ffd
} // namespace ctrl

#endif // CTRL_FFD_TASK_H
