#include <utility>
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
#include "core/ResourceUpdatingWorkspace.h"
#include "core/FFDKeyUpdater.h"
#include "core/ImageKeyUpdater.h"
#include "core/ClippingFrame.h"
#include "core/DestinationTexturizer.h"

namespace core
{

LayerNode::LayerNode(const QString& aName, ShaderHolder& aShaderHolder)
    : mName(aName)
    , mDepth()
    , mIsVisible(true)
    , mInitialRect()
    , mTimeLine()
    , mShaderHolder(aShaderHolder)
    , mIsClipped()
    , mMeshTransformer("./data/shader/MeshTransform.glslex")
    , mRenderDepth(0.0f)
    , mCurrentMesh()
    , mClippees()
{
}

void LayerNode::setDefaultImage(const img::ResourceHandle& aHandle)
{
    setDefaultImage(aHandle, aHandle->blendMode());
}

void LayerNode::setDefaultImage(const img::ResourceHandle& aHandle, img::BlendMode aBlendMode)
{
    XC_ASSERT(aHandle);
    XC_PTR_ASSERT(aHandle->image().data());
    XC_ASSERT(aHandle->image().pixelSize().isValid());
    if (!aHandle || !aHandle->image().data()) return; // fail safe

    auto key = new ImageKey();
    mTimeLine.grabDefaultKey(TimeKeyType_Image, key);
    key->setImage(aHandle, aBlendMode);

    mShaderHolder.reserveShaders(aBlendMode);
    mShaderHolder.reserveGridShader();
    mShaderHolder.reserveClipperShaders();
}

void LayerNode::setDefaultPos(const QVector2D& aPos)
{
    auto key = (SRTKey*)mTimeLine.defaultKey(TimeKeyType_SRT);
    if (!key)
    {
        key = new SRTKey();
        mTimeLine.grabDefaultKey(TimeKeyType_SRT, key);
    }
    key->data().pos = QVector3D(aPos);
    key->data().clampPos();
}

void LayerNode::setDefaultOpacity(float aValue)
{
    auto key = (OpaKey*)mTimeLine.defaultKey(TimeKeyType_Opa);
    if (!key)
    {
        key = new OpaKey();
        mTimeLine.grabDefaultKey(TimeKeyType_Opa, key);
    }
    key->data().opacity = aValue;
    key->data().clamp();
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

img::BlendMode LayerNode::blendMode() const
{
    auto key = (ImageKey*)mTimeLine.defaultKey(TimeKeyType_Image);
    return key ? key->data().blendMode() : img::BlendMode_Normal;
}

void LayerNode::setBlendMode(img::BlendMode aMode)
{
    auto key = (ImageKey*)mTimeLine.defaultKey(TimeKeyType_Image);
    if (key)
    {
        key->data().setBlendMode(aMode);
        mShaderHolder.reserveShaders(aMode);
    }
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
    if (!mIsVisible || !aInfo.clippingFrame) return;

    XC_PTR_ASSERT(mCurrentMesh);
    gl::Global::Functions& ggl = gl::Global::functions();
    auto& shader = mShaderHolder.clipperShader(aInfo.clippingId != 0);
    auto& expans = aAccessor.get(mTimeLine);

    if (!expans.areaTexture()) return;
    auto textureId = expans.areaTexture()->id();

    core::ClippingFrame& frame = *aInfo.clippingFrame;
    frame.updateRenderStamp();

    // bind framebuffer
    frame.bind();
    frame.setupDrawBuffers();

    // bind textures
    ggl.glActiveTexture(GL_TEXTURE0);
    ggl.glBindTexture(GL_TEXTURE_2D, textureId);
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
        shader.setUniformValue("uScreenSize", QSizeF(aInfo.camera.screenSize()));
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

    // current mesh
    LayerMesh* mesh = expans.ffdMesh();
    if (mesh->vertexCount() <= 0) return;

    // positions
    util::ArrayBlock<const gl::Vector3> positions;
    bool useInfluence = true;

    if (aInfo.originMesh) // ignore mesh deforming
    {
        useInfluence = false;
        positions = util::ArrayBlock<const gl::Vector3>(
                    mesh->positions(), mesh->vertexCount());
    }
    else
    {
        useInfluence = (mesh == expans.bone().targetMesh());
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
    if (!mCurrentMesh) return;

    gl::Global::Functions& ggl = gl::Global::functions();
    const bool isClippee = (aInfo.clippingFrame && aInfo.clippingId != 0);

    auto& expans = aAccessor.get(mTimeLine);
    if (!expans.areaImageKey() || !expans.areaTexture()) return;

    auto textureId = expans.areaTexture()->id();
    auto blendMode = expans.blendMode();
    const QMatrix4x4 viewMatrix = aInfo.camera.viewMatrix();


    auto& shader = aInfo.isGrid ?
                mShaderHolder.gridShader() :
                mShaderHolder.shader(blendMode, isClippee);

    // update destination color
    XC_PTR_ASSERT(aInfo.destTexturizer);
    auto destTextureId = aInfo.destTexturizer->texture().id();
    if (!aInfo.isGrid && blendMode != img::BlendMode_Normal)
    {
        aInfo.destTexturizer->update(
                    aInfo.framebuffer, aInfo.dest, viewMatrix,
                    *mCurrentMesh, mMeshTransformer.positions());
    }

    if (aInfo.isGrid)
    {
        ggl.glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        // blend func
        ggl.glEnable(GL_BLEND);
        ggl.glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

        ggl.glActiveTexture(GL_TEXTURE0);
        ggl.glBindTexture(GL_TEXTURE_2D, textureId);
        ggl.glActiveTexture(GL_TEXTURE1);
        ggl.glBindTexture(GL_TEXTURE_2D, destTextureId);

        if (isClippee)
        {
            ggl.glActiveTexture(GL_TEXTURE2);
            ggl.glBindTexture(GL_TEXTURE_2D, aInfo.clippingFrame->texture().id());
        }
    }

    {
        const float opacity = expans.worldOpacity();
        QColor color(255, 255, 255, xc_clamp((int)(255 * opacity), 0, 255));
        if (aInfo.isGrid) color = QColor(Qt::black);

        shader.bind();

        shader.setAttributeBuffer("inPosition", mMeshTransformer.positions(), GL_FLOAT, 3);
        shader.setAttributeArray("inTexCoord", mCurrentMesh->texCoords(), mCurrentMesh->vertexCount());

        shader.setUniformValue("uViewMatrix", viewMatrix);
        shader.setUniformValue("uScreenSize", QSizeF(aInfo.camera.screenSize()));
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

        // blend func
        ggl.glDisable(GL_BLEND);
    }

    ggl.glFlush();
}

cmnd::Vector LayerNode::createResourceUpdater(const ResourceEvent& aEvent)
{
    cmnd::Vector result;

    ResourceUpdatingWorkspacePtr workspace = std::make_shared<ResourceUpdatingWorkspace>();
    const bool createTransitions = !mTimeLine.isEmpty(TimeKeyType_FFD);

    // image key
    result.push(ImageKeyUpdater::createResourceUpdater(*this, aEvent, workspace, createTransitions));

    // ffd key should be called finally
    if (createTransitions)
    {
        result.push(FFDKeyUpdater::createResourceUpdater(*this, workspace));
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
    // initial rect
    aOut.write(mInitialRect);
    // clipping
    aOut.write(mIsClipped);
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
    // check block begin
    if (!aIn.beginBlock("LayerNd_"))
        return aIn.errored("invalid signature of layer node");

    // name
    aIn.read(mName);
    // depth
    aIn.read(mDepth);
    // visibility
    aIn.read(mIsVisible);
    // initial rect
    aIn.read(mInitialRect);
    // clipping
    aIn.read(mIsClipped);
    // timeline
    if (!mTimeLine.deserialize(aIn))
        return aIn.errored("failed to deserialize time line");

    // reserve shaders
    {
        mShaderHolder.reserveGridShader();
        mShaderHolder.reserveClipperShaders();

        auto defaultKey = (ImageKey*)mTimeLine.defaultKey(TimeKeyType_Image);
        if (defaultKey)
        {
            mShaderHolder.reserveShaders(defaultKey->data().blendMode());
        }

        auto& map = mTimeLine.map(TimeKeyType_Image);
        for (auto key : map)
        {
            mShaderHolder.reserveShaders(((ImageKey*)key)->data().blendMode());
        }
    }

    // check block end
    if (!aIn.endBlock())
        return aIn.errored("invalid end of layer node");

    return aIn.checkStream();
}

} // namespace core
