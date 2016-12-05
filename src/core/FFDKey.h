#ifndef CORE_FFDKEY_H
#define CORE_FFDKEY_H

#include <QVector>
#include "util/Easing.h"
#include "gl/Vector3.h"
#include "core/TimeKey.h"

namespace core
{

class FFDKey : public TimeKey
{
public:
    class Data
    {
        util::Easing::Param mEasing;
        QVector<gl::Vector3> mBuffer;
        int mVtxCount;
    public:
        Data();
        void alloc(int aVtxCount);
        void write(const gl::Vector3* aSrc, int aVtxCount);
        void clear();
        void swap(QVector<gl::Vector3>& aRhs);
        util::Easing::Param& easing() { return mEasing; }
        const util::Easing::Param& easing() const { return mEasing; }
        gl::Vector3* positions();
        const gl::Vector3* positions()const;
        int count() const;
        void insertVtx(int aIndex, const gl::Vector3& aPos);
        void pushBackVtx(const gl::Vector3& aPos);
        gl::Vector3 removeVtx(int aIndex);
        gl::Vector3 popBackVtx();
    };

    FFDKey();

    Data& data() { return mData; }
    const Data& data() const { return mData; }

    bool belongsToDefaultParent() const { return !parent(); }

    virtual TimeKeyType type() const { return TimeKeyType_FFD; }
    virtual TimeKey* createClone();
    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

private:
    Data mData;
};

} // namespace core

#endif // CORE_FFDKEY_H
