#ifndef CORE_LAYERMESH
#define CORE_LAYERMESH

#include <QVector>
#include <QScopedPointer>
#include "util/ArrayBlock.h"
#include "util/Range.h"
#include "gl/Vector2.h"
#include "gl/Vector3.h"
#include "gl/Vector2I.h"
#include "gl/BufferObject.h"
#include "core/Frame.h"

namespace core
{

class LayerMesh
{
public:

    struct MeshBuffer
    {
        struct GLBinder { GLBinder(); };
        MeshBuffer();
        ~MeshBuffer();
        void reserve(int aVtxCount);

        GLBinder glBinder;
        gl::BufferObject workPositions;
        gl::BufferObject workXArrows;
        gl::BufferObject workYArrows;
        gl::BufferObject outPositions;
        gl::BufferObject outXArrows;
        gl::BufferObject outYArrows;
        int vtxCount;
    };

    struct ArrayedConnection
    {
        enum { kMaxCount = 1024 };
        ArrayedConnection();
        void resetPositions();
        void pushPosition(const gl::Vector2& aPos);

        QScopedArrayPointer<gl::Vector2> positions;
        int positionCount;
        util::Range vertexRange;
    };

    struct ArrayedConnectionList
    {
        ArrayedConnectionList();
        ~ArrayedConnectionList();
        void clearBlocks();
        void destroyBlocks();
        void destroyUnuseBlocks();
        ArrayedConnection* pushNewBlock();
        void resetIndexRanges(int aCount);

        QScopedArrayPointer<gl::Vector2I> indexRanges;
        QList<ArrayedConnection*> blocks;
        int indexRangeCount;
        int useBlockCount;
    };

    class ArrayedConnectionWriter
    {
        ArrayedConnectionList& mList;
        int mVertexCount;
        ArrayedConnection* mCurBlock;
        int mIndex;
    public:
        ArrayedConnectionWriter(ArrayedConnectionList& aList, int aVertexCount);
        ~ArrayedConnectionWriter();
        void beginOneVertex(int aMaxConnectionCount);
        void pushPosition(const gl::Vector2& aPos);
    };

    virtual ~LayerMesh() {}
    virtual GLenum primitiveMode() const = 0;
    virtual const gl::Vector3* positions() const = 0;
    virtual const gl::Vector2* texCoords() const = 0;
    virtual int vertexCount() const = 0;
    virtual const GLuint* indices() const = 0;
    virtual GLsizei indexCount() const = 0;    
    virtual MeshBuffer& getMeshBuffer() = 0;
    virtual void resetArrayedConnection(
            ArrayedConnectionList& aDest,
            const gl::Vector3* aPositions) const = 0;
    virtual Frame frameSign() const = 0;
};

} // namespace core

#endif // CORE_LAYERMESH

