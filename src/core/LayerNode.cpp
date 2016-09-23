#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include "XC.h"
#include "gl/Global.h"
#include "gl/ExtendShader.h"
#include "gl/Util.h"
#include "img/ResourceNode.h"
#include "img/BlendMode.h"
#include "core/LayerNode.h"
#include "core/ObjectNodeUtil.h"
#include "core/TimeKeyExpans.h"
#include "core/ResourceEvent.h"
#include "core/FFDKeyUpdater.h"
#include "core/ClippingFrame.h"

namespace core
{

LayerNode::LayerNode(const QString& aName, ShaderHolder& aShaderHolder)
    : mName(aName)
    , mDepth()
    , mIsVisible(true)
    , mImageRect()
    , mInitialCenter()
    , mImageHandle()
    , mBlendMode(img::BlendMode_Normal)
    , mGridMesh()
    , mTimeLine()
    , mShaderHolder(aShaderHolder)
    , mIsClipped()
    , mTexture()
    , mMeshTransformer("./data/shader/MeshTransform.glslex")
    , mRenderDepth(0.0f)
    , mCurrentMesh()
    , mClippees()
{
}

LayerNode::~LayerNode()
{
    clear();
}

void LayerNode::setImage(const img::ResourceHandle& aHandle)
{
    setImage(aHandle, aHandle.blendMode());
}

void LayerNode::setImage(const img::ResourceHandle& aHandle, img::BlendMode aBlendMode)
{
    XC_ASSERT(aHandle);
    XC_PTR_ASSERT(aHandle.image().data());
    XC_ASSERT(aHandle.image().pixelSize().isValid());
    if (!aHandle || !aHandle.image().data()) return; // fail safe

    mImageHandle = aHandle;

    readImageData(aHandle.image(), aHandle.pos());

    mBlendMode = aBlendMode;
    mShaderHolder.reserveShader(mBlendMode, true);
    mShaderHolder.reserveShader(mBlendMode, false);
    mShaderHolder.reserveGridShader();
    mShaderHolder.reserveClipperShader(true);
    mShaderHolder.reserveClipperShader(false);
}

void LayerNode::readImageData(const img::Buffer& aBuffer, const QPoint& aPos)
{
    mImageRect = QRect(aPos, aBuffer.pixelSize());
    //mInitialCenter = QVector2D(QRectF(mImageRect).center());

    // make a gl texture
    mTexture.create(aBuffer.pixelSize(), aBuffer.data());
    mTexture.setFilter(GL_LINEAR);
    mTexture.setWrap(GL_CLAMP_TO_BORDER, QColor(0, 0, 0, 0));

    // grid mesh
    const int cellPx = std::max(std::min(8, aBuffer.width() / 4), 2);
    mGridMesh.createFromImage(aBuffer.data(), aBuffer.pixelSize(), cellPx);
}

void LayerNode::clear()
{
    mTexture.destroy();
    mImageHandle = img::ResourceHandle();
}

void LayerNode::setClipped(bool aIsClipped)
{
    mIsClipped = aIsClipped;
}

bool LayerNode::isClipper() const
{
    if (mIsClipped) return false;

    auto prev = this->prevSib();
    if (!prev || !prev->renderer() || !prev->renderer()->isClipped())
    {
        return false;
    }
    return true;
}

void LayerNode::setBlendMode(img::BlendMode aMode)
{
    mBlendMode = aMode;
    mShaderHolder.reserveShader(mBlendMode, true);
    mShaderHolder.reserveShader(mBlendMode, false);
}

void LayerNode::prerender(const RenderInfo& aInfo, const TimeCacheAccessor& aAccessor)
{
    mCurrentMesh = nullptr;

    if (!mIsVisible) return;

    // transform shape by current keys
    transformShape(aInfo, aAccessor);
}

void LayerNode::render(const RenderInfo& aInfo, const TimeCacheAccessor& aAccessor)
{
    if (!mIsVisible) return;

    if (aAccessor.get(mTimeLine).opa().isZero()) return;

    // render shape
    renderShape(aInfo, aAccessor);

    if (aInfo.isGrid) return;

    // render clippees
    renderClippees(aInfo, aAccessor);
}

void LayerNode::renderClippees(
        const RenderInfo& aInfo, const TimeCacheAccessor& aAccessor)
{
    if (!aInfo.clippingFrame || !isClipper()) return;

    // reset clippees
    ObjectNodeUtil::collectRenderClippees(*this, mClippees);

    // clipping frame
    auto& frame = *aInfo.clippingFrame;

    const uint8 clippingId = frame.forwardClippingId();

    RenderInfo childInfo = aInfo;
    childInfo.clippingId = clippingId;

    uint32 stamp = frame.renderStamp() + 1;

    for (auto clippee : mClippees)
    {
        XC_PTR_ASSERT(clippee);

        // write clipper as necessary
        if (stamp != frame.renderStamp())
        {
            renderClipper(aInfo, aAccessor, clippingId);
            stamp = frame.renderStamp();
        }

        // render child
        clippee->render(childInfo, aAccessor);
    }
}

void LayerNode::renderClipper(
        const RenderInfo& aInfo, const TimeCacheAccessor& aAccessor, uint8 aClipperId)
{
    if (!mIsVisible || !aInfo.clippingFrame || !mImageHandle) return;

    XC_PTR_ASSERT(mCurrentMesh);
    gl::Global::Functions& ggl = gl::Global::functions();
    auto& shader = mShaderHolder.clipperShader(aInfo.clippingId != 0);
    auto& expans = aAccessor.get(mTimeLine);

    core::ClippingFrame& frame = *aInfo.clippingFrame;
    frame.updateRenderStamp();

    // bind framebuffer
    frame.bind();
    frame.setupDrawBuffers();

    // bind textures
    //ggl.glEnable(GL_TEXTURE_2D);
    ggl.glActiveTexture(GL_TEXTURE0);
    ggl.glBindTexture(GL_TEXTURE_2D, mTexture.id());
    ggl.glActiveTexture(GL_TEXTURE1);
    ggl.glBindTexture(GL_TEXTURE_2D, frame.texture().id());

    // view matrix
    const QMatrix4x4 viewMatrix = aInfo.camera.viewMatrix();
    // color
    const float opacity = expans.worldOpacity();
    QColor color(255, 255, 255, xc_clamp((int)(255 * opacity), 0, 255));
    {
        shader.bind();

        shader.setAttributeBuffer("inPosition", mMeshTransformer.positions(), GL_FLOAT, 3);
        shader.setAttributeArray("inTexCoord", mCurrentMesh->texCoords(), mCurrentMesh->vertexCount());

        shader.setUniformValue("uViewMatrix", viewMatrix);
        shader.setUniformValue("uColor", color);
        shader.setUniformValue("uClipperId", (int)aClipperId);
        shader.setUniformValue("uTexture", 0);
        shader.setUniformValue("uDestTexture", 1);

        if (aInfo.clippingId != 0)
        {
            shader.setUniformValue("uClippingId", (int)aInfo.clippingId);
        }

        ggl.glDrawElements(
                    mCurrentMesh->primitiveMode(),
                    mCurrentMesh->indexCount(),
                    GL_UNSIGNED_INT, mCurrentMesh->indices());

        shader.release();
    }
    // unbind texture
    ggl.glActiveTexture(GL_TEXTURE0);
    ggl.glBindTexture(GL_TEXTURE_2D, 0);
    //ggl.glDisable(GL_TEXTURE_2D);

    // release framebuffer
    frame.release();

    // bind default framebuffer
    ggl.glBindFramebuffer(GL_FRAMEBUFFER, aInfo.framebuffer);

    ggl.glFlush();
}

void LayerNode::transformShape(
        const RenderInfo& aInfo, const TimeCacheAccessor& aAccessor)
{
    auto& expans = aAccessor.get(mTimeLine);

    // select mesh and positions
    LayerMesh* mesh = &mGridMesh;
    util::ArrayBlock<const gl::Vector3> positions;
    bool useInfluence = true;

    if (aInfo.originMesh) // ignore mesh deforming
    {
        if (mesh->vertexCount() <= 0)
        {
            return;
        }

        useInfluence = false;
        positions = util::ArrayBlock<const gl::Vector3>(
                    mGridMesh.positions(), mGridMesh.vertexCount());
        XC_ASSERT(mesh->vertexCount() == positions.count());
    }
    else
    {
        if (expans.areaMesh())
        {
            mesh = &expans.areaMesh()->data();
        }

        if (mesh->vertexCount() <= 0)
        {
            return;
        }

        useInfluence = (mesh == expans.boneParent());

        positions = util::ArrayBlock<const gl::Vector3>(
                    expans.ffd().positions(), expans.ffd().count());
        XC_MSG_ASSERT(mesh->vertexCount() == positions.count(),
                      "vtx count = %d, %d", mesh->vertexCount(), positions.count());
    }
    XC_ASSERT(positions);

    // transform
    mMeshTransformer.callGL(
                expans, mesh->getMeshBuffer(), positions,
                aInfo.nonPosed, useInfluence);

    mCurrentMesh = mesh;
}

void LayerNode::renderShape(
        const RenderInfo& aInfo, const TimeCacheAccessor& aAccessor)
{
    //XC_PTR_ASSERT(mCurrentMesh);
    if (!mCurrentMesh) return;
    if (!mImageHandle) return;

    gl::Global::Functions& ggl = gl::Global::functions();
    const bool isClippee = (aInfo.clippingFrame && aInfo.clippingId != 0);
    auto& shader = aInfo.isGrid ?
                mShaderHolder.gridShader() :
                mShaderHolder.shader(mBlendMode, isClippee);
    auto& expans = aAccessor.get(mTimeLine);

    if (aInfo.isGrid)
    {
        ggl.glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        //ggl.glEnable(GL_TEXTURE_2D);
        ggl.glActiveTexture(GL_TEXTURE0);
        ggl.glBindTexture(GL_TEXTURE_2D, mTexture.id());
        ggl.glActiveTexture(GL_TEXTURE1);
        ggl.glBindTexture(GL_TEXTURE_2D, aInfo.dest);

        if (isClippee)
        {
            ggl.glActiveTexture(GL_TEXTURE2);
            ggl.glBindTexture(GL_TEXTURE_2D, aInfo.clippingFrame->texture().id());
        }
    }

    {
        const QMatrix4x4 viewMatrix = aInfo.camera.viewMatrix();

        const float opacity = expans.worldOpacity();
        QColor color(255, 255, 255, xc_clamp((int)(255 * opacity), 0, 255));
        if (aInfo.isGrid) color = QColor(Qt::black);

        shader.bind();

        shader.setAttributeBuffer("inPosition", mMeshTransformer.positions(), GL_FLOAT, 3);
        shader.setAttributeArray("inTexCoord", mCurrentMesh->texCoords(), mCurrentMesh->vertexCount());

        shader.setUniformValue("uViewMatrix", viewMatrix);
        shader.setUniformValue("uColor", color);
        shader.setUniformValue("uTexture", 0);
        shader.setUniformValue("uDestTexture", 1);

        if (isClippee)
        {
            shader.setUniformValue("uClippingId", (int)aInfo.clippingId);
            shader.setUniformValue("uClippingTexture", 2);
        }

        ggl.glDrawElements(
                    mCurrentMesh->primitiveMode(),
                    mCurrentMesh->indexCount(),
                    GL_UNSIGNED_INT, mCurrentMesh->indices());

        shader.release();
    }

    if (aInfo.isGrid)
    {
        ggl.glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else
    {
        ggl.glActiveTexture(GL_TEXTURE0);
        ggl.glBindTexture(GL_TEXTURE_2D, 0);
        //ggl.glDisable(GL_TEXTURE_2D);
    }

    ggl.glFlush();
}

cmnd::Vector LayerNode::createResourceUpdater(const ResourceEvent& aEvent)
{
    class ResUpdater : public cmnd::Stable
    {
        LayerNode& mOwner;
        img::Buffer mOldImage;
        QPoint mOldPos;
        QScopedPointer<cmnd::Stable> mKeyUpdater;

    public:
        ResUpdater(LayerNode& aOwner, img::Buffer& aCurImage, const QPoint& aCurPos)
            : mOwner(aOwner)
            , mOldImage()
            , mOldPos(aCurPos)
            , mKeyUpdater()
        {
            mOldImage.grab(aCurImage);
        }

        virtual void exec()
        {
            GridMesh::TransitionCreater transer(
                        mOwner.mGridMesh, mOwner.mImageRect.topLeft());

            mOwner.readImageData(
                        mOwner.mImageHandle.image(),
                        mOwner.mImageHandle.pos());

            auto trans = transer.create(
                        mOwner.mGridMesh.positions(),
                        mOwner.mGridMesh.vertexCount(),
                        mOwner.mImageRect.topLeft());

            // only ffd key requires updater
            if (!mOwner.mTimeLine.isEmpty(TimeKeyType_FFD))
            {
                mKeyUpdater.reset(FFDKeyUpdater::createResourceUpdater(
                                      mOwner, mOwner.mGridMesh, trans));
                XC_ASSERT(mKeyUpdater);
                mKeyUpdater->exec();
            }
        }

        virtual void redo()
        {
            mOwner.readImageData(
                        mOwner.mImageHandle.image(),
                        mOwner.mImageHandle.pos());

            if (mKeyUpdater)
            {
                mKeyUpdater->redo();
            }
        }

        virtual void undo()
        {
            mOwner.readImageData(mOldImage, mOldPos);

            if (mKeyUpdater)
            {
                mKeyUpdater->undo();
            }
        }
    };

    cmnd::Vector result;
    if (mImageHandle && mImageHandle.hasImage())
    {
        if (aEvent.targets().contains(mImageHandle.serialAddress()))
        {
            img::Buffer curImage(mImageHandle.image());
            result.push(new ResUpdater(*this, curImage, mImageHandle.pos()));
        }
    }

    return result;
}

bool LayerNode::serialize(Serializer& aOut) const
{
    static const std::array<uint8, 8> kSignature =
        { 'L', 'a', 'y', 'e', 'r', 'N', 'd', '_' };

    // block begin
    auto pos = aOut.beginBlock(kSignature);

    // name
    aOut.write(mName);
    // depth
    aOut.write(mDepth);
    // visibility
    aOut.write(mIsVisible);
    // image rect
    aOut.write(mImageRect);
    // center
    aOut.write(mInitialCenter);
    // clipping
    aOut.write(mIsClipped);
    // blend mode
    aOut.writeFixedString(img::getQuadIdFromBlendMode(mBlendMode), 4);

    // image id
    aOut.writeID(mImageHandle.serialAddress());

    // grid mesh
    if (!mGridMesh.serialize(aOut))
    {
        return false;
    }

    // timeline
    if (!mTimeLine.serialize(aOut))
    {
        return false;
    }

    // block end
    aOut.endBlock(pos);

    return !aOut.failure();
}

bool LayerNode::deserialize(Deserializer& aIn)
{
    clear();

    // check block begin
    if (!aIn.beginBlock("LayerNd_"))
        return aIn.errored("invalid signature of layer node");

    // name
    aIn.read(mName);
    // depth
    aIn.read(mDepth);
    // visibility
    aIn.read(mIsVisible);
    // image rect
    aIn.read(mImageRect);
    // center
    aIn.read(mInitialCenter);
    // clipping
    aIn.read(mIsClipped);
    // blend mode
    {
        QString bname;
        aIn.readFixedString(bname, 4);
        auto bmode = img::getBlendModeFromQuadId(bname);
        if (bmode == img::BlendMode_TERM)
        {
            return aIn.errored("invalid image blending mode");
        }
        mBlendMode = bmode;
    }

    // image id
    {
        auto solver = [=](void* aPtr)
        {
            this->mImageHandle = ((img::ResourceNode*)aPtr)->handle();
            this->setImage(this->mImageHandle, this->mBlendMode);
        };
        if (!aIn.orderIDData(solver))
        {
            return aIn.errored("invalid image reference id");
        }
    }

    // grid mesh
    if (!mGridMesh.deserialize(aIn))
        return aIn.errored("failed to deserialize grid mesh");

    // timeline
    if (!mTimeLine.deserialize(aIn))
        return aIn.errored("failed to deserialize time line");

    // check block end
    if (!aIn.endBlock())
        return aIn.errored("invalid end of layer node");

    return aIn.checkStream();
}

} // namespace core
