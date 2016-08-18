#ifndef UTIL_BINARYSPACEPARTITION2D
#define UTIL_BINARYSPACEPARTITION2D

#include <array>
#include <vector>
#include <list>
#include <QRectF>
#include "util/Triangle2D.h"
#include "util/CollDetect.h"

namespace util
{

template<typename tData>
class BinarySpacePartition2D
{
public:
    struct Object
    {
        Object(const tData& aData, const Triangle2D& aTri, const QRectF& aBox)
            : data(aData)
            , tri(aTri)
            , box(aBox)
        {}

        tData data;
        Triangle2D tri;
        QRectF box;
    };

    struct Node
    {
        explicit Node(const QRectF& aBox)
            : box(aBox)
            , child()
            , objects()
        {}

        bool hasChild() const { return child[0]; }

        QRectF box;
        std::array<Node*, 2> child;
        std::list<Object*> objects;
    };

    BinarySpacePartition2D(const QRectF& aSpace, int aMaxDepth = 10)
        : mSpace(aSpace)
        , mMaxDepth(aMaxDepth)
        , mNodeHeap()
        , mTopNode()
    {
        mNodeHeap.emplace_back(Node(mSpace));
        mTopNode = &mNodeHeap.back();
    }

    bool push(const tData& aData, const Triangle2D& aTri)
    {
        auto rect = aTri.boundingRect();

        if (!mTopNode->box.intersects(rect))
        {
            return false;
        }

        mObjHeap.emplace_back(Object(aData, aTri, rect));
        writeObject(*mTopNode, mObjHeap.back(), 1);
        return true;
    }

    Object* findOne(const QPointF& aPoint) const
    {
        return findImpl(*mTopNode, aPoint);
    }

private:
    void writeObject(Node& aNode, Object& aObject, int aDepth)
    {
        // push object if...
        auto box = aNode.box;
        if (aDepth >= mMaxDepth || aObject.box.contains(box))
        {
            aNode.objects.push_back(&aObject);
            return;
        }

        // make child
        if (!aNode.hasChild())
        {
            // make child box
            bool horizontal = box.width() >= box.height();
            QRectF childBox[2];
            if (horizontal)
            {
                childBox[0] = box;
                childBox[1] = box;
                childBox[0].setWidth(box.width() / 2.0f);
                childBox[1].setLeft(childBox[0].right());
            }
            else
            {
                childBox[0] = box;
                childBox[1] = box;
                childBox[0].setHeight(box.height() / 2.0f);
                childBox[1].setTop(childBox[0].bottom());
            }
            for (int i = 0; i < 2; ++i)
            {
                mNodeHeap.emplace_back(Node(childBox[i]));
                aNode.child[i] = &mNodeHeap.back();
            }
        }

        for (int i = 0; i < 2; ++i)
        {
            if (aObject.box.intersects(aNode.child[i]->box))
            {
                writeObject(*aNode.child[i], aObject, aDepth + 1);
            }
        }
    }

    Object* findImpl(const Node& aNode, const QPointF& aPoint) const
    {
        if (!aNode.box.contains(aPoint))
        {
            return nullptr;
        }

        const QVector2D vec(aPoint);
        for (auto obj : aNode.objects)
        {
            if (obj->box.contains(aPoint))
            {
                if (CollDetect::isInside(obj->tri, vec))
                {
                    return obj;
                }
            }
        }

        if (aNode.hasChild())
        {
            for (int i = 0; i < 2; ++i)
            {
                auto result = findImpl(*aNode.child[i], aPoint);
                if (result) return result;
            }
        }
        return nullptr;
    }

    QRectF mSpace;
    int mMaxDepth;
    std::list<Node> mNodeHeap;
    std::list<Object> mObjHeap;
    Node* mTopNode;
};

} // namespace util

#endif // UTIL_BINARYSPACEPARTITION2D

