#ifndef CORE_DESERIALIZER
#define CORE_DESERIALIZER

#include <QStringList>
#include <QVector>
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
#include "util/IndexTable.h"
#include "util/StreamReader.h"
#include "util/IDSolver.h"
#include "util/IProgressReporter.h"
#include "gl/Vector2.h"
#include "gl/Vector3.h"
#include "gl/DeviceInfo.h"
#include "core/Frame.h"

namespace core
{

class Deserializer
{
public:
    typedef std::istream::pos_type PosType;
    typedef util::IDSolver<void*> IDSolverType;

    Deserializer(
            util::LEStreamReader& aIn,
            IDSolverType& aSolver,
            size_t aMaxFileSize,
            const gl::DeviceInfo& aGLDeviceInfo,
            util::IProgressReporter& aRepoter,
            int aRShiftCount);

    void read(bool& aValue);
    void read(int& aValue);
    void read(float& aValue);
    void read(QPoint& aValue);
    void read(QVector2D& aValue);
    void read(QVector3D& aValue);
    void read(QVector4D& aValue);
    void read(util::Segment2D& aValue);
    void read(QSize& aValue);
    void read(QRect& aValue);
    void read(QRectF& aValue);
    void read(QMatrix4x4& aValue);
    void read(Frame& aValue);
    void read(QPolygonF& aValue);
    void read(QString& aValue, int aMax = 0);
    bool read(const XCMemBlock& aValue);
    bool readWithAlloc(XCMemBlock& aEmptyValue);
    bool readWithAlloc(util::IndexTable& aEmptyValue);

    void readGL(GLint* aArray, int aCount);
    void readGL(GLuint* aArray, int aCount);
    void readGL(GLfloat* aArray, int aCount);
    void readGL(gl::Vector2* aArray, int aCount);
    void readGL(gl::Vector3* aArray, int aCount);

    bool bindIDData(void* aData);
    bool orderIDData(const IDSolverType::Solver& aSolver);

    bool readImage(XCMemBlock& aEmptyValue);
    void readFixedString(QString& aValue, int aSize);

    bool beginBlock(const std::string& aSignature);
    bool endBlock();

    bool failure() const;

    void pushLogScope(const QString& aScope);
    void popLogScope();
    void setLog(const QString& aLog);
    const QVector<QString>& logScopes() const;
    const QStringList& log() const { return mLog; }

    bool errored(const QString& aLog)
    {
        if (!mValue.isEmpty())
        {
            setLog(aLog + "(" + mValue + ")");
            mValue.clear();
        }
        else
        {
            setLog(aLog);
        }
        return false;
    }

    bool checkStream()
    {
        if (failure())
        {
            setLog("stream error");
            return false;
        }
        return true;
    }

    void reportCurrent();

private:
    void alignBy(size_t aSize);
    size_t getRestSize() const;

    util::LEStreamReader& mIn;
    IDSolverType& mIDSolver;
    QVector<PosType> mBlockEnds;
    QVector<QString> mScopes;
    QStringList mLog;
    QString mValue;
    size_t mMaxFileSize;
    gl::DeviceInfo mGLDeviceInfo;
    util::IProgressReporter& mReporter;
    int mRShiftCount;
    std::ios::pos_type mFileBegin;
};

} // namespace core

#endif // CORE_DESERIALIZER

