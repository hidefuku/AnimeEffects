#ifndef UTIL_TREEPOS_H
#define UTIL_TREEPOS_H

#include <QModelIndex>

namespace util
{

class TreePos
{
public:
    TreePos();
    explicit TreePos(const QModelIndex& aIndex);
    TreePos(const QModelIndex& aParentIndex, int aRow);

    bool isValid() const { return mIsValid; }
    TreePos parent() const;
    int depth() const { return (int)mRows.size(); }
    int row(int aDepth) const { return mRows.at(aDepth); }
    int tailRow() const { return depth() > 0 ? mRows[depth() - 1] : 0; }
    const std::vector<int>& rows() const { return mRows; }
    bool operator==(const TreePos& aRhs) const;

    void updateByRemove(const TreePos& aRemovePos);
    void updateByInsert(const TreePos& aInsertPos);

    void setValidity(bool aIsValid);
    void pushRow(int aRow);

    void dump() const;
private:
    void pushRecursive(const QModelIndex& aIndex);
    std::vector<int> mRows;
    bool mIsValid;
};

typedef std::vector<TreePos> TreePosVector;

} // namespace util

#endif // UTIL_TREEPOS_H
