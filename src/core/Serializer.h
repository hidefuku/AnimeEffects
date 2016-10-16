#ifndef CORE_SERIALIZER
#define CORE_SERIALIZER

#include <QPoint>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QRect>
#include <QRectF>
#include <QMatrix4x4>
#include <QPolygonF>
#include <QGL>
#include "XC.h"
#include "util/Segment2D.h"
#include "util/Easing.h"
#include "util/IndexTable.h"
#include "util/StreamWriter.h"
#include "util/IDAssigner.h"
#include "gl/Vector2.h"
#include "gl/Vector3.h"
#include "core/Frame.h"

namespace core
{

class Serializer
{
public:
    typedef std::ostream::pos_type PosType;

    Serializer(util::StreamWriter& aOut);

    void write(bool aValue);
    void write(int aValue);
    void write(float aValue);
    void write(const QPoint& aValue);
    void write(const QVector2D& aValue);
    void write(const QVector3D& aValue);
    void write(const QVector4D& aValue);
    void write(const util::Segment2D& aValue);
    void write(const QSize& aValue);
    void write(const QRect& aValue);
    void write(const QRectF& aValue);
    void write(const QMatrix4x4& aValue);
    void write(const Frame& aValue);
    void write(const util::Easing::Param& aValue);
    void write(const QPolygonF& aValue);
    void write(const QString& aValue);
    void write(const XCMemBlock& aValue);
    void write(const util::IndexTable& aTable);

    void writeGL(const GLint* aArray, int aCount);
    void writeGL(const GLuint* aArray, int aCount);
    void writeGL(const GLfloat* aArray, int aCount);
    void writeGL(const gl::Vector2* aArray, int aCount);
    void writeGL(const gl::Vector3* aArray, int aCount);

    void writeID(const void* aData);
    void writeImage(const XCMemBlock& aImage, const QSize& aSize);
    void writeFixedString(const QString& aValue, int aSize);

    PosType beginBlock(const std::array<uint8, 8>& aSignature);
    void endBlock(PosType aPos);

    bool failure() const;

    bool checkStream()
    {
        if (failure())
        {
            //setLog("stream error");
            return false;
        }
        return true;
    }

private:
    util::StreamWriter& mOut;
    util::IDAssigner<const void*> mIDAssigner;
};

} // namespace core

#endif // SERIALIZER

