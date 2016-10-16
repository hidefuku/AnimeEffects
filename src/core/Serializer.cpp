#include "core/Serializer.h"
#include "util/PackBits.h"

namespace core
{

Serializer::Serializer(util::StreamWriter& aOut)
    : mOut(aOut)
    , mIDAssigner()
{
    // set null to zero
    auto id = mIDAssigner.getId(nullptr);
    XC_ASSERT(id == 0);
}

void Serializer::write(bool aValue)
{
    mOut.write((uint32)aValue);
}

void Serializer::write(int aValue)
{
    mOut.write((sint32)aValue);
}

void Serializer::write(float aValue)
{
    mOut.write((float32)aValue);
}

void Serializer::write(const QPoint& aValue)
{
    mOut.write((sint32)aValue.x());
    mOut.write((sint32)aValue.y());
}

void Serializer::write(const QVector2D& aValue)
{
    mOut.write((float32)aValue.x());
    mOut.write((float32)aValue.y());
}

void Serializer::write(const QVector3D& aValue)
{
    mOut.write((float32)aValue.x());
    mOut.write((float32)aValue.y());
    mOut.write((float32)aValue.z());
}

void Serializer::write(const QVector4D& aValue)
{
    mOut.write((float32)aValue.x());
    mOut.write((float32)aValue.y());
    mOut.write((float32)aValue.z());
    mOut.write((float32)aValue.w());
}

void Serializer::write(const util::Segment2D& aValue)
{
    write(aValue.start);
    write(aValue.dir);
}

void Serializer::write(const QSize& aValue)
{
    mOut.write((sint32)aValue.width());
    mOut.write((sint32)aValue.height());
}

void Serializer::write(const QRect& aValue)
{
    mOut.write((sint32)aValue.x());
    mOut.write((sint32)aValue.y());
    mOut.write((sint32)aValue.width());
    mOut.write((sint32)aValue.height());
}

void Serializer::write(const QRectF& aValue)
{
    mOut.write((float32)aValue.x());
    mOut.write((float32)aValue.y());
    mOut.write((float32)aValue.width());
    mOut.write((float32)aValue.height());
}

void Serializer::write(const QMatrix4x4& aValue)
{
    for (int i = 0; i < 4; ++i)
    {
        write(aValue.column(i));
    }
}

void Serializer::write(const Frame& aValue)
{
    auto s = aValue.serialValue();
    mOut.write((sint32)s.value);
    mOut.write((sint32)s.milli);
}

void Serializer::write(const util::Easing::Param& aValue)
{
    mOut.write((sint32)aValue.type);
    mOut.write((sint32)aValue.range);
    mOut.write((float32)aValue.weight);
}

void Serializer::write(const QPolygonF& aValue)
{
    mOut.write((sint32)aValue.count());
    for (auto vtx : aValue)
    {
        mOut.write((float32)vtx.x());
        mOut.write((float32)vtx.y());
    }
}

void Serializer::write(const QString& aValue)
{
    mOut.writeString(aValue.toStdString(), 4);
}

void Serializer::writeFixedString(const QString& aValue, int aSize)
{
    const std::string value = aValue.toStdString();
    const int length = static_cast<int>(value.size());
    XC_ASSERT(0 <= length && length <= aSize);

    mOut.writeBytes(XCMemBlock((uint8*)value.c_str(), (size_t)length), 1);

    if (length < aSize)
    {
        mOut.writeZero(aSize - length);
    }
}

void Serializer::write(const XCMemBlock& aValue)
{
    mOut.write((uint64)aValue.size);
    if (aValue.data)
    {
        mOut.writeBytes(aValue, 4);
    }
}

void Serializer::write(const util::IndexTable& aTable)
{
    mOut.write((sint32)aTable.width());
    mOut.write((sint32)aTable.height());
    const uint32* data = aTable.data();
    const size_t size = sizeof(uint32) * aTable.count();
    if (size > 0)
    {
        mOut.writeBytes(XCMemBlock((uint8*)data, size), 4);
    }
}

void Serializer::writeGL(const GLint* aArray, int aCount)
{
    for (int i = 0; i < aCount; ++i)
    {
        mOut.write((sint32)aArray[i]);
    }
}

void Serializer::writeGL(const GLuint* aArray, int aCount)
{
    for (int i = 0; i < aCount; ++i)
    {
        mOut.write((uint32)aArray[i]);
    }
}

void Serializer::writeGL(const GLfloat* aArray, int aCount)
{
    for (int i = 0; i < aCount; ++i)
    {
        mOut.write((float32)aArray[i]);
    }
}

void Serializer::writeGL(const gl::Vector2* aArray, int aCount)
{
    for (int i = 0; i < aCount; ++i)
    {
        mOut.write((float32)aArray[i].x);
        mOut.write((float32)aArray[i].y);
    }
}

void Serializer::writeGL(const gl::Vector3* aArray, int aCount)
{
    for (int i = 0; i < aCount; ++i)
    {
        mOut.write((float32)aArray[i].x);
        mOut.write((float32)aArray[i].y);
        mOut.write((float32)aArray[i].z);
    }
}

void Serializer::writeID(const void *aData)
{
    auto id = mIDAssigner.getId(aData);
    mOut.write(static_cast<sint32>(id));
}

void Serializer::writeImage(const XCMemBlock& aImage, const QSize& aSize)
{
    const int w = aSize.width();
    const int h = aSize.height();
    XC_ASSERT(aImage.size == (size_t)(w * h * 4));

    if (!aImage.data || aImage.size == 0)
    {
        // null data
        mOut.write((uint32)0);
        mOut.write((uint32)0);
        mOut.write((uint32)0);
        return;
    }

    // compression type
    mOut.write((uint32)1);

    // image size
    mOut.write((uint32)w);
    mOut.write((uint32)h);

    const uint8* src = aImage.data;
    const size_t wrkSize = (size_t)w;
    const size_t dstSize = util::PackBits::worstEncodedSize(wrkSize);

    QScopedPointer<uint8> wrk(new uint8[wrkSize]);
    QScopedPointer<uint8> dst(new uint8[dstSize]);
    const XCMemBlock wrkBlock(wrk.data(), wrkSize);

    util::PackBits encoder;

    // total length
    auto pos = mOut.reserveLength();

    // each line
    for (int y = 0; y < h; ++y)
    {
        // each channel
        for (int i = 0; i < 4; ++i)
        {
            // separate channel bytes
            const uint8* sp = src + i;
            const uint8* we = wrk.data() + wrkSize;
            for (uint8* wp = wrk.data(); wp < we; ++wp, sp += 4) *wp = *sp;

            // encode
            size_t size = encoder.encode(wrkBlock, dst.data());

            // line length
            mOut.write((uint32)size);
            // compressed bytes
            mOut.writeBytes(XCMemBlock(dst.data(), size), 1);
        }
        src += w * 4;
    }

    // write total length
    mOut.writeLength(pos);

    // align
    mOut.alignFrom(pos, 4);
}

Serializer::PosType Serializer::beginBlock(const std::array<uint8, 8>& aSignature)
{
    mOut.write(aSignature);
    return mOut.reserveLength();
}

void Serializer::endBlock(PosType aPos)
{
    mOut.writeLength(aPos);
}

bool Serializer::failure() const
{
    return mOut.isFailed();
}

} // namespace core
