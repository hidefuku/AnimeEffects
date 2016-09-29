#include "util/MathUtil.h"
#include "core/Bone2.h"

namespace core
{

Bone2::Bone2()
    : TreeNodeBase(this)
    , mOrigin()
    , mLocalPos()
    , mLocalAngle()
    , mRange()
    , mShape()
    , mBindingNodes()
    , mWorldPos()
    , mWorldAngle()
    , mRotate()
    , mFocus()
    , mSelect()
{
}

Bone2::Bone2(const Bone2& aRhs)
    : TreeNodeBase(this)
    , mOrigin(aRhs.mOrigin)
    , mLocalPos(aRhs.mLocalPos)
    , mLocalAngle(aRhs.mLocalAngle)
    , mRange(aRhs.mRange)
    , mShape(aRhs.mShape)
    , mBindingNodes(aRhs.mBindingNodes)
    , mWorldPos(aRhs.mWorldPos)
    , mWorldAngle(aRhs.mWorldAngle)
    , mRotate(aRhs.mRotate)
    , mFocus()
    , mSelect()
{
}

Bone2::~Bone2()
{
    qDeleteAll(children().begin(), children().end());
}

void Bone2::setWorldPos(const QVector2D& aWorldPos, const Bone2* aParent)
{
    XC_ASSERT(!mOrigin);

    if (aParent)
    {
        const QVector2D dir = aWorldPos - aParent->worldPos();
        mLocalPos = QVector2D(dir.length(), 0.0f);
        mLocalAngle = util::MathUtil::getAngleDifferenceRad(
                    aParent->worldAngle(), util::MathUtil::getAngleRad(dir)) - mRotate;
    }
    else
    {
        mLocalPos = aWorldPos;
        mLocalAngle = 0.0f - mRotate;
    }
}

void Bone2::setShape(const BoneShape& aShape)
{
    XC_ASSERT(!mOrigin);
    mShape = aShape;
}

const BoneShape& Bone2::shape() const
{
    return mOrigin ? mOrigin->mShape : mShape;
}

QList<ObjectNode*>& Bone2::bindingNodes()
{
    XC_ASSERT(!mOrigin);
    return mBindingNodes;
}
const QList<ObjectNode*>& Bone2::bindingNodes() const
{
    XC_ASSERT(!mOrigin);
    return mBindingNodes;
}

bool Bone2::isBinding(const ObjectNode& aNode) const
{
    for (auto node : mBindingNodes)
    {
        if (node == &aNode) return true;
    }
    return false;
}

void Bone2::setRotate(float aRotate)
{
    mRotate = aRotate;
}

float Bone2::rotate() const
{
    return mRotate;
}

const QVector2D& Bone2::localPos() const
{
    return mOrigin ? mOrigin->mLocalPos : mLocalPos;
}

float Bone2::localAngle() const
{
    return mOrigin ? mOrigin->mLocalAngle : mLocalAngle;
}

void Bone2::setRange(int aIndex, const QVector2D& aRange)
{
    XC_ASSERT(!mOrigin);
    XC_ASSERT(0 <= aIndex && aIndex < 2);
    mRange.at(aIndex) = aRange;
}

const QVector2D& Bone2::range(int aIndex) const
{
    XC_ASSERT(0 <= aIndex && aIndex < 2);
    return mOrigin ? mOrigin->mRange.at(aIndex) : mRange.at(aIndex);
}

bool Bone2::hasValidRange() const
{
    return mOrigin ?
                mOrigin->hasValidRange() :
                (!mRange[0].isNull() || !mRange[1].isNull());
}

QVector2D Bone2::blendedRange(float aRate) const
{
    return mOrigin ?
                mOrigin->blendedRange(aRate) :
                (mRange[0] * (1.0f - aRate) + mRange[1] * aRate);
}

void Bone2::updateWorldTransform()
{
    if (parent())
    {
        mWorldAngle = parent()->worldAngle() + localAngle() + mRotate;
        const QVector2D dir = util::MathUtil::getRotateVectorRad(localPos(), mWorldAngle);
        mWorldPos = parent()->worldPos() + dir;
    }
    else
    {
        mWorldPos = localPos();
        mWorldAngle = localAngle();
    }

    for (auto child : children())
    {
        child->updateWorldTransform();
    }
}

const QVector2D& Bone2::worldPos() const
{
    return mWorldPos;
}

float Bone2::worldAngle() const
{
    return mWorldAngle;
}

QMatrix4x4 Bone2::transformationMatrix(const QVector2D& aToPos, float aToAngle) const
{
    static const QVector3D kRotateAxis(0.0f, 0.0f, 1.0f);
    const float rotate = util::MathUtil::getDegreeFromRadian(aToAngle - worldAngle());

    QMatrix4x4 mtx;
    mtx.translate(aToPos);
    mtx.rotate(rotate, kRotateAxis);
    mtx.translate(-worldPos());
    return mtx;
}

QMatrix4x4 Bone2::transformationMatrix(const Bone2& aTo) const
{
    return transformationMatrix(aTo.worldPos(), aTo.worldAngle());
}

QMatrix4x4 Bone2::transformationMatrix(const QMatrix4x4& aToMtx) const
{
    QMatrix4x4 myInvMtx;
    {
        static const QVector3D kRotateAxis(0.0f, 0.0f, 1.0f);
        myInvMtx.rotate(-util::MathUtil::getDegreeFromRadian(worldAngle()), kRotateAxis);
        myInvMtx.translate(-worldPos());
    }

    return aToMtx * myInvMtx;
}

Bone2* Bone2::createShadow() const
{
    if (mOrigin)
    {
        return mOrigin->createShadow();
    }

    Bone2* shadow = new Bone2(*this);

    if (shadow)
    {
        shadow->mOrigin = this;
        shadow->mFocus = util::LifeLink::Node();
        shadow->mSelect = util::LifeLink::Node();

        // recursive
        for (auto child : children())
        {
            Bone2* childShadow = child->createShadow();
            if (childShadow)
            {
                shadow->children().pushBack(childShadow);
            }
        }
    }
    return shadow;
}

bool Bone2::serialize(Serializer& aOut) const
{
    aOut.writeID(this);
    aOut.writeID(mOrigin);

    aOut.write(mLocalPos);
    aOut.write(mLocalAngle);
    aOut.write(mRange[0]);
    aOut.write(mRange[1]);

    if (!mShape.serialize(aOut))
    {
        return false;
    }

    aOut.write(mBindingNodes.count());
    for (auto node : mBindingNodes)
    {
        aOut.writeID(node);
    }

    aOut.write(mWorldPos);
    aOut.write(mWorldAngle);
    aOut.write(mRotate);

    return aOut.checkStream();
}

bool Bone2::deserialize(Deserializer& aIn)
{
    if (!aIn.bindIDData(this))
    {
        return aIn.errored("failed to bind reference id");
    }

    // order to write a pointer later
    {
        auto solver = [=](void* aPtr){this->mOrigin = (Bone2*)aPtr; };
        if (!aIn.orderIDData(solver))
        {
            return aIn.errored("invalid reference id");
        }
    }

    aIn.read(mLocalPos);
    aIn.read(mLocalAngle);
    aIn.read(mRange[0]);
    aIn.read(mRange[1]);

    if (!mShape.deserialize(aIn))
    {
        return false;
    }

    {
        int bindCount = 0;
        aIn.read(bindCount);
        if (bindCount < 0) return false;

        for (int i = 0; i < bindCount; ++i)
        {
            auto solver = [=](void* aPtr){ if (aPtr) { this->mBindingNodes.push_back((ObjectNode*)aPtr); } };
            if (!aIn.orderIDData(solver))
            {
                return aIn.errored("invalid reference id");
            }

            if (aIn.failure())
            {
                return aIn.errored("stream error");
            }
        }
    }

    aIn.read(mWorldPos);
    aIn.read(mWorldAngle);
    aIn.read(mRotate);

    return aIn.checkStream();
}

} // namespace core
