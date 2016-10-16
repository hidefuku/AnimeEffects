#include "core/FFDKey.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
FFDKey::Data::Data()
    : mEasing()
    , mBuffer()
    , mVtxCount(0)
{
}

void FFDKey::Data::alloc(int aVtxCount)
{
    XC_ASSERT(aVtxCount > 0);
    mVtxCount = aVtxCount;

    if (mBuffer.count() != aVtxCount)
    {
        mBuffer.resize(aVtxCount);
    }
}

void FFDKey::Data::write(const gl::Vector3* aSrc, int aVtxCount)
{
    XC_ASSERT(aVtxCount > 0);
    const size_t newSize = sizeof(gl::Vector3) * aVtxCount;
    alloc(aVtxCount);
    memcpy(mBuffer.data(), aSrc, newSize);
}

void FFDKey::Data::clear()
{
    mVtxCount = 0;
    mBuffer.clear();
}

void FFDKey::Data::swap(QVector<gl::Vector3>& aRhs)
{
    XC_ASSERT(mVtxCount == mBuffer.count());
    mVtxCount = aRhs.count();
    mBuffer.swap(aRhs);
}

gl::Vector3* FFDKey::Data::positions()
{
    XC_ASSERT(mVtxCount > 0);
    XC_PTR_ASSERT(mBuffer.data());
    return mBuffer.data();
}

const gl::Vector3* FFDKey::Data::positions() const
{
    XC_ASSERT(mVtxCount > 0);
    XC_PTR_ASSERT(mBuffer.data());
    return mBuffer.data();
}

int FFDKey::Data::count() const
{
    return mVtxCount;
}

void FFDKey::Data::insertVtx(int aIndex, const gl::Vector3& aPos)
{
    XC_ASSERT(0 <= aIndex && aIndex <= count());
    mBuffer.insert(aIndex, aPos);
    ++mVtxCount;
}

void FFDKey::Data::pushBackVtx(const gl::Vector3& aPos)
{
    mBuffer.push_back(aPos);
    ++mVtxCount;
}

gl::Vector3 FFDKey::Data::removeVtx(int aIndex)
{
    XC_ASSERT(count() > 0);
    XC_ASSERT(0 <= aIndex && aIndex < count());

    auto pos = mBuffer.at(aIndex);
    mBuffer.removeAt(aIndex);
    --mVtxCount;
    return pos;
}

gl::Vector3 FFDKey::Data::popBackVtx()
{
    XC_ASSERT(count() > 0);
    auto pos = mBuffer.at(count() - 1);
    mBuffer.pop_back();
    --mVtxCount;
    return pos;
}

//-------------------------------------------------------------------------------------------------
FFDKey::FFDKey()
    : mData()
{
}

bool FFDKey::serialize(Serializer& aOut) const
{
    XC_PTR_ASSERT(mData.positions());
    XC_ASSERT(mData.count() >= 0);

    // easing
    aOut.write(mData.easing());

    // vertex count
    aOut.write(mData.count());
    // positions
    aOut.writeGL(mData.positions(), mData.count());

    return aOut.checkStream();
}

bool FFDKey::deserialize(Deserializer& aIn)
{
    aIn.pushLogScope("FFDKey");

    if (!aIn.read(mData.easing()))
    {
        return aIn.errored("invalid easing param");
    }

    // vertex count
    int count = 0;
    aIn.read(count);

    // allocate
    mData.alloc(count);

    // positions
    aIn.readGL(mData.positions(), mData.count());

    aIn.popLogScope();
    return aIn.checkStream();
}

} // namespace core
