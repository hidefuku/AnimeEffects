#include <float.h>
#include "util/CollDetect.h"
#include "core/Constant.h"
#include "ctrl/bone/bone_Focuser.h"
#include "ctrl/bone/bone_Renderer.h"

using namespace core;

namespace ctrl {
namespace bone {

Focuser::Focuser()
    : mTopBones()
    , mFocusLink()
    , mSelectLink()
    , mLastFocus()
    , mFocusChanged()
    , mFocusConnector()
    , mFocusRate()
    , mTargetMtx()
{
}

void Focuser::setTopBones(QList<Bone2*>& aTopBones)
{
    mTopBones = &aTopBones;
}

void Focuser::setFocusConnector(bool aFocus)
{
    mFocusConnector = aFocus;
}

void Focuser::setTargetMatrix(const QMatrix4x4& aMtx)
{
    mTargetMtx = aMtx;
}

Bone2* Focuser::update(const CameraInfo& aCamera, const QVector2D& aPos)
{
    mFocusChanged = (bool)mLastFocus;
    auto prev = mLastFocus;
    mLastFocus = nullptr;
    mFocusRate = 0.0f;
    mFocusLink.clear();

    Bone2* bone = updateImpl(aCamera, aPos);
    if (bone)
    {
        bone->setFocus(mFocusLink);
        mFocusChanged = (prev != bone);
        mLastFocus = bone;
    }
    return bone;
}

Bone2* Focuser::updateImpl(const CameraInfo& aCamera, const QVector2D& aPos)
{
    static const float kRange = 7.5f * 7.5f;
    static const float kSegRange = 10.0f * 10.0f;

    if (!mTopBones) return nullptr;

    // find a overwrapped joint
    for (auto topBone : *mTopBones)
    {
        Bone2::Iterator itr(topBone);
        while (itr.hasNext())
        {
            auto bone = itr.next();
            const QVector2D pos = aCamera.toScreenPos(
                        mTargetMtx * QVector3D(bone->worldPos())).toVector2D();
            if ((pos - aPos).lengthSquared() <= kRange)
            {
                return bone;
            }
        }
    }

    if (!mFocusConnector) return nullptr;

    const QVector2D curPos = aCamera.toWorldPos(aPos);

    // find a overwrapped connector
    for (auto topBone : *mTopBones)
    {
        Bone2::Iterator itr(topBone);
        while (itr.hasNext())
        {
            auto bone = itr.next();
            auto parent = bone->parent();

            if (!parent) continue;

            const QVector2D ppos = (mTargetMtx * QVector3D(parent->worldPos())).toVector2D();
            const QVector2D cpos = (mTargetMtx * QVector3D(bone->worldPos())).toVector2D();

            if ((ppos - cpos).length() < FLT_EPSILON) continue;

            auto quad = Renderer::getBoneQuad(
                        ppos.toPointF(), cpos.toPointF());
            // quad
            if (util::CollDetect::isInside(quad.data(), 4, curPos.toPointF()))
            {
                const QVector2D dir(quad[0] - quad[2]);
                const float length = dir.length();

                if (length >= core::Constant::normalizable())
                {
                    const float distance =
                            QVector2D::dotProduct(dir / length, curPos - cpos);

                    mFocusRate = xc_clamp(distance / length, 0.0f, 1.0f);
                }

                return bone;
            }
        }
    }

    // find a overwrapped connector area
    for (auto topBone : *mTopBones)
    {
        // each bone
        Bone2::Iterator itr(topBone);
        while (itr.hasNext())
        {
            auto bone = itr.next();
            auto parent = bone->parent();

            if (!parent) continue;

            const QVector2D ppos = (mTargetMtx * QVector3D(parent->worldPos())).toVector2D();
            const QVector2D cpos = (mTargetMtx * QVector3D(bone->worldPos())).toVector2D();
            const QVector2D dir(ppos - cpos);
            const float length = dir.length();

            if (length < core::Constant::normalizable()) continue;

            auto quad = Renderer::getBoneQuad(ppos.toPointF(), cpos.toPointF());

            const float segRange = std::min(kSegRange, 0.1f * length * length);

            for (int i = 0; i < 4; ++i)
            {
                const QVector2D segdir(quad[(i + 1) % 4] - quad[i]);
                const util::Segment2D seg(QVector2D(quad[i]), segdir);

                if (segdir.length() < core::Constant::normalizable()) continue;

                if (util::CollDetect::getMinDistanceSquared(seg, curPos) > segRange) continue;

                const float distance =
                        QVector2D::dotProduct(dir / length, curPos - cpos);

                mFocusRate = xc_clamp(distance / length, 0.0f, 1.0f);

                return bone;
            }
        }
    }

    return nullptr;
}

void Focuser::clearFocus()
{
    mFocusLink.clear();
}

bool Focuser::focusChanged() const
{
    return mFocusChanged;
}

void Focuser::select(Bone2& aBone)
{
    aBone.setSelect(mSelectLink);
}

core::Bone2* Focuser::selectingBone()
{
    if (!mTopBones) return nullptr;

    for (auto topBone : *mTopBones)
    {
        Bone2::Iterator itr(topBone);
        while (itr.hasNext())
        {
            auto bone = itr.next();
            if (bone->isSelected()) return bone;
        }
    }
    return nullptr;
}

void Focuser::clearSelection()
{
    mSelectLink.clear();
}

} // namespace bone
} // namespace ctrl
