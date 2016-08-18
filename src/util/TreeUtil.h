#ifndef UTIL_TREEUTIL_H
#define UTIL_TREEUTIL_H

#include "util/TreePos.h"

namespace util
{

class TreeUtil
{
public:
    template<typename tObj>
    static tObj* createClone(const tObj* aObj)
    {
        if (!aObj) return nullptr;

        tObj* obj = new tObj(*aObj);

        for (auto child : aObj->children())
        {
            obj->children().pushBack(createClone<tObj>(child));
        }

        return obj;
    }

    template<typename tObj, typename tShadow>
    static tShadow* createShadow(const tObj* aObj)
    {
        if (!aObj) return nullptr;

        tShadow* shadow = new tShadow(*aObj);

        for (auto child : aObj->children())
        {
            shadow->children().pushBack(createShadow<tObj, tShadow>(child));
        }

        return shadow;
    }

    template<typename tObj>
    static void deleteAll(tObj* aObj)
    {
        for (auto child : aObj->children())
        {
            deleteAll<tObj>(child);
        }
        aObj->children().clear();

        delete aObj;
    }

    template<typename tObj>
    static tObj& getTreeRoot(tObj& aObj)
    {
        tObj* root = &aObj;
        while (root->parent())
        {
            root = root->parent();
        }
        return *root;
    }

    template<typename tObj>
    static TreePos getTreePos(const tObj* aObj)
    {
        TreePos pos;
        pos.setValidity((bool)aObj);
        pushTreeRowRecursive(pos, aObj);
        return pos;
    }

    template<typename tObj>
    static bool insertTo(tObj& aRoot, const TreePos& aPos, tObj* aObj)
    {
        if (!aObj) return false;
        if (!aPos.isValid() || aPos.depth() <= 1) return false;

        tObj* parent = &aRoot;
        for (int i = 1; i < aPos.depth() - 1; ++i)
        {
            parent = *parent->children().at(aPos.row(i));
        }
        auto pos = parent->children().at(aPos.tailRow());
        parent->children().insert(pos, aObj);

        return true;
    }

    template<typename tObj>
    static tObj* eraseFrom(tObj& aRoot, const TreePos& aPos)
    {
        if (!aPos.isValid() || aPos.depth() <= 1) return nullptr;

        tObj* obj = nullptr;
        tObj* parent = &aRoot;
        for (int i = 1; i < aPos.depth() - 1; ++i)
        {
            parent = *parent->children().at(aPos.row(i));
        }
        auto pos = parent->children().at(aPos.tailRow());
        obj = *pos;
        parent->children().erase(pos);

        return obj;
    }

    template<typename tObj>
    static tObj* find(tObj& aRoot, const TreePos& aPos)
    {
        if (!aPos.isValid()) return nullptr;
        if (aPos.depth() <= 1) return &aRoot;

        tObj* obj = &aRoot;
        for (int i = 1; i < aPos.depth(); ++i)
        {
            obj = *obj->children().at(aPos.row(i));
        }
        return obj;
    }

    template<typename tObj>
    static bool leftContainsRight(tObj& aLeft, tObj& aRight)
    {
        for (tObj* p = &aRight; p; p = p->parent())
        {
            if (p == &aLeft) return true;
        }
        return false;
    }

    template<typename tPointerVector>
    static tPointerVector getUniqueRoots(const tPointerVector& aTargets)
    {
        tPointerVector roots;
        roots.reserve(aTargets.size());

        for (auto target : aTargets)
        {
            bool isNew = true;
            for (int i = 0; i < roots.size(); ++i)
            {
                auto root = roots[i];
                if (leftContainsRight(*root, *target))
                {
                    isNew = false;
                    break;
                }
                else if (leftContainsRight(*target, *root))
                {
                    roots[i] = target;
                    isNew = false;
                    break;
                }
            }
            if (isNew)
            {
                roots.push_back(target);
            }
        }
        return roots;
    }

private:
    TreeUtil();

    template<typename tObj>
    static void pushTreeRowRecursive(TreePos& aDst, const tObj* aObj)
    {
        if (!aObj) return;
        const tObj* parent = aObj->parent();
        if (parent)
        {
            pushTreeRowRecursive<tObj>(aDst, parent);
            aDst.pushRow(parent->children().indexOf(aObj));
        }
        else
        {
            aDst.pushRow(0);
        }
    }
};

} // namespace util

#endif // UTIL_TREEUTIL_H
