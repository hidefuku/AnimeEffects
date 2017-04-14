#include "XC.h"
#include "core/Deserializer.h"
#include "util/PackBits.h"

namespace core
{

Deserializer::Deserializer(
        util::LEStreamReader& aIn,
        IDSolverType& aSolver,
        size_t aMaxFileSize,
        QVersionNumber aVersion,
        const gl::DeviceInfo& aGLDeviceInfo,
        util::IProgressReporter& aReporter,
        int aRShiftCount)
    : mIn(aIn)
    , mIDSolver(aSolver)
    , mBlockEnds()
    , mScopes()
    , mLog()
    , mValue()
    , mMaxFileSize(aMaxFileSize)
    , mVersion(aVersion)
    , mGLDeviceInfo(aGLDeviceInfo)
    , mReporter(aReporter)
    , mRShiftCount(aRShiftCount)
    , mFileBegin()
{
    // set null to zero
    mIDSolver.pushData(0, nullptr);

    // record file begin pos
    mFileBegin = mIn.tellg();
}

void Deserializer::read(bool& aValue)
{
    aValue = (bool)mIn.readUInt32();
}

void Deserializer::read(int& aValue)
{
    aValue = mIn.readSInt32();
}

void Deserializer::read(float& aValue)
{
    aValue = mIn.readFloat32();
}

void Deserializer::read(QPoint& aValue)
{
    aValue.setX(mIn.readSInt32());
    aValue.setY(mIn.readSInt32());
}

void Deserializer::read(QVector2D& aValue)
{
    aValue.setX(mIn.readFloat32());
    aValue.setY(mIn.readFloat32());
}

void Deserializer::read(QVector3D& aValue)
{
    aValue.setX(mIn.readFloat32());
    aValue.setY(mIn.readFloat32());
    aValue.setZ(mIn.readFloat32());
}

void Deserializer::read(QVector4D& aValue)
{
    aValue.setX(mIn.readFloat32());
    aValue.setY(mIn.readFloat32());
    aValue.setZ(mIn.readFloat32());
    aValue.setW(mIn.readFloat32());
}

void Deserializer::read(util::Segment2D& aValue)
{
    read(aValue.start);
    read(aValue.dir);
}

void Deserializer::read(QSize& aValue)
{
    aValue.setWidth(mIn.readSInt32());
    aValue.setHeight(mIn.readSInt32());
}

void Deserializer::read(QRect& aValue)
{
    aValue.setX(mIn.readSInt32());
    aValue.setY(mIn.readSInt32());
    aValue.setWidth(mIn.readSInt32());
    aValue.setHeight(mIn.readSInt32());
}

void Deserializer::read(QRectF& aValue)
{
    aValue.setX(mIn.readFloat32());
    aValue.setY(mIn.readFloat32());
    aValue.setWidth(mIn.readFloat32());
    aValue.setHeight(mIn.readFloat32());
}

void Deserializer::read(QMatrix4x4 &aValue)
{
    QVector4D v;
    for (int i = 0; i < 4; ++i)
    {
        read(v);
        aValue.setColumn(i, v);
    }
}

void Deserializer::read(Frame& aValue)
{
    Frame::SerialValue s = {};
    s.value = mIn.readSInt32();
    s.milli = mIn.readSInt32();
    aValue.setSerialValue(s);
}

bool Deserializer::read(util::Easing::Param& aValue)
{
    aValue.type = (util::Easing::Type)mIn.readSInt32();
    aValue.range = (util::Easing::Range)mIn.readSInt32();
    aValue.weight = mIn.readFloat32();
    return aValue.isValidParam();
}

void Deserializer::read(QPolygonF& aValue)
{
    aValue.clear();
    const int count = mIn.readSInt32();
    if (count > 0)
    {
        aValue.resize(count);
        for (int i = 0; i < count; ++i)
        {
            QPointF vtx;
            vtx.setX(mIn.readFloat32());
            vtx.setY(mIn.readFloat32());
            aValue[i] = vtx;
        }
    }
}

void Deserializer::read(QString& aValue, int aMax)
{
    const std::string str = aMax > 0 ?
                mIn.readStringWithLimit(aMax) :
                mIn.readString();
    aValue = QString::fromStdString(str);
    alignBy(str.size() + 1);
}

void Deserializer::readFixedString(QString& aValue, int aSize)
{
    const std::string str = mIn.readString(aSize);
    aValue = QString::fromStdString(str);
}

bool Deserializer::read(const XCMemBlock& aValue)
{
    const uint64 rawSize = mIn.readUInt64();
    if (std::numeric_limits<size_t>::max() < rawSize) return false;
    const size_t size = (size_t)rawSize;

    if (size != aValue.size) return false;
    if (getRestSize() < size) return false;

    if (size > 0)
    {
        XC_PTR_ASSERT(aValue.data);
        mIn.readBuf(aValue.data, size);
    }
    return true;
}

bool Deserializer::readWithAlloc(XCMemBlock& aValue)
{
    XC_ASSERT(aValue.data == nullptr);

    const uint64 rawSize = mIn.readUInt64();
    if (std::numeric_limits<size_t>::max() < rawSize) return false;

    aValue.size = (size_t)rawSize;
    if (getRestSize() < aValue.size) return false;

    if (aValue.size > 0)
    {
        aValue.data = new uint8[aValue.size];
        if (!aValue.data) return false;

        XC_ASSERT((uint64)aValue.data % 4 == 0);
        mIn.readBuf(aValue.data, aValue.size);
        alignBy(aValue.size);
    }
    return true;
}

bool Deserializer::readWithAlloc(util::IndexTable& aEmptyValue)
{
    const int width  = mIn.readSInt32();
    const int height = mIn.readSInt32();
    if (width <= 0 || height <= 0) return false;

    const size_t size = sizeof(uint32) * (size_t)(width * height);
    if (getRestSize() < size) return false;

    aEmptyValue.alloc(width, height);
    uint32* data = aEmptyValue.data();
    if (!data) return false;

    if (size > 0)
    {
        mIn.readBuf((uint8*)data, size);
    }
    return true;
}

void Deserializer::readGL(GLint* aArray, int aCount)
{
    for (int i = 0; i < aCount; ++i)
    {
        aArray[i] = mIn.readSInt32();
    }
}

void Deserializer::readGL(GLuint* aArray, int aCount)
{
    for (int i = 0; i < aCount; ++i)
    {
        aArray[i] = mIn.readUInt32();
    }
}

void Deserializer::readGL(GLfloat* aArray, int aCount)
{
    for (int i = 0; i < aCount; ++i)
    {
        aArray[i] = mIn.readFloat32();
    }
}

void Deserializer::readGL(gl::Vector2* aArray, int aCount)
{
    for (int i = 0; i < aCount; ++i)
    {
        aArray[i].x = mIn.readFloat32();
        aArray[i].y = mIn.readFloat32();
    }
}

void Deserializer::readGL(gl::Vector3* aArray, int aCount)
{
    for (int i = 0; i < aCount; ++i)
    {
        aArray[i].x = mIn.readFloat32();
        aArray[i].y = mIn.readFloat32();
        aArray[i].z = mIn.readFloat32();
    }
}

bool Deserializer::bindIDData(void* aData)
{
    const int id = mIn.readSInt32();
    if (id >= 0) mIDSolver.pushData(id, aData);
    return id >= 0;
}

bool Deserializer::orderIDData(const IDSolverType::Solver& aSolver)
{
    const int id = mIn.readSInt32();
    if (id >= 0) mIDSolver.pushReferencer(id, aSolver);
    return id >= 0;
}

bool Deserializer::readImage(XCMemBlock& aValue)
{
    XC_ASSERT(aValue.data == nullptr);

    // compression type
    const uint32 compType = mIn.readUInt32();

    if (compType > 1)
    {
        return false;
    }

    // image size
    const uint32 w = mIn.readUInt32();
    const uint32 h = mIn.readUInt32();

    // check null data
    // (the corresponding serialize function allows a null image data)
    if (compType == 0)
    {
        if (w != 0 || h != 0)
        {
            return false;
        }
        return true;
    }

    // check the image has valid size
    const uint32 maxSize = (uint32)mGLDeviceInfo.maxTextureSize;
    if (w <= 0 || h <= 0 || maxSize < w || maxSize < h)
    {
        return false;
    }

    // total compressed length
    const uint64 length = mIn.readUInt64();
    if (std::numeric_limits<size_t>::max() < length) return false;
    if (getRestSize() < (size_t)length) return false;

    auto pos = mIn.tellg();

    // allocate dest
    const size_t dstSize = w * h * 4;
    QScopedPointer<uint8> dst(new uint8[dstSize]);
    if (dst.isNull()) return false;

    // allocate work buffer
    const size_t srcSize = util::PackBits::worstEncodedSize((size_t)w);
    const size_t wrkSize = (size_t)w;
    QScopedPointer<uint8> src(new uint8[srcSize]);
    QScopedPointer<uint8> wrk(new uint8[wrkSize]);
    XCMemBlock wrkBlock(wrk.data(), wrkSize);
    if (src.isNull()) return false;
    if (wrk.isNull()) return false;

    util::PackBits decoder;

    uint8* dstp = dst.data();

    // each line
    for (uint32 y = 0; y < h; ++y)
    {
        // each channel
        for (int i = 0; i < 4; ++i)
        {
            // line length
            const size_t linelen = (size_t)mIn.readUInt32();
            // compressed bytes
            mIn.readBuf(src.data(), linelen);

            // decode
            if (!decoder.decode(XCMemBlock(src.data(), linelen), wrkBlock))
            {
                return false;
            }

            // merge channel bytes
            const uint8* wp = wrk.data();
            const uint8* we = wp + wrkSize;
            for (uint8* dp = dstp + i; wp < we; dp += 4, ++wp) *dp = *wp;
        }
        dstp += w * 4;
    }

    // check total length
    if ((length + pos) != (uint64)mIn.tellg())
    {
        return false;
    }

    // alignment
    alignBy(length);

    // set
    aValue.size = dstSize;
    aValue.data = dst.take();

    return true;
}

bool Deserializer::beginBlock(const std::string& aSignature)
{
    std::string signature = mIn.readString(8);
    uint64 length = mIn.readUInt64();
    mBlockEnds.push_back(mIn.tellg() + static_cast<PosType>(length));
    if (signature != aSignature)
    {
        mValue = QString::fromStdString(signature);
        return false;
    }
    return true;
}

bool Deserializer::endBlock()
{
    if (mIn.isFailed()) return false;

    if (mBlockEnds.empty()) return false;

    PosType blockEnd = mBlockEnds.back();
    mBlockEnds.pop_back();
    if (mIn.tellg() != blockEnd)
    {
        QString v0, v1;
        v0.setNum(mIn.tellg());
        v1.setNum(blockEnd);
        mValue = v0 + ", " + v1;
        return false;
    }
    return true;
}

bool Deserializer::failure() const
{
    return mIn.isFailed();
}


void Deserializer::pushLogScope(const QString& aScope)
{
    mScopes.push_back(aScope);
}

void Deserializer::popLogScope()
{
    mScopes.pop_back();
}

void Deserializer::setLog(const QString& aLog)
{
    mLog.push_back(aLog);
}

const QVector<QString>& Deserializer::logScopes() const
{
    return mScopes;
}

void Deserializer::alignBy(size_t aSize)
{
    size_t tail = aSize % 4;
    if (tail) mIn.skip((int)(4 - tail));
}

void Deserializer::reportCurrent()
{
    const size_t current = (size_t)mIn.tellg();
    mReporter.setProgress((int)(current >> mRShiftCount));
}

size_t Deserializer::getRestSize() const
{
    auto curPos = mIn.tellg();
    if (curPos < mFileBegin) curPos = mFileBegin;

    const size_t curSize = (size_t)(curPos - mFileBegin);
    if (mMaxFileSize < curSize) return (size_t)0;

    return mMaxFileSize - curSize;
}

} // namespace core

