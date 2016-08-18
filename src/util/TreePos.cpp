#include "util/TreePos.h"
#include "XCAssert.h"

namespace util
{

TreePos::TreePos()
    : mIsValid(false)
{
}

TreePos::TreePos(const QModelIndex& aIndex)
    : mIsValid(aIndex.isValid())
{
    pushRecursive(aIndex);
}

TreePos::TreePos(const QModelIndex& aParentIndex, int aRow)
    : mIsValid(aParentIndex.isValid() && aRow >= 0)
{
    pushRecursive(aParentIndex);
    mRows.push_back(aRow);
}

void TreePos::pushRecursive(const QModelIndex& aIndex)
{
    if (aIndex.isValid())
    {
        pushRecursive(aIndex.parent());
        mRows.push_back(aIndex.row());
    }
}

void TreePos::updateByRemove(const TreePos& aRemovePos)
{
    if (!isValid() || !aRemovePos.isValid()) return;
    if (depth() < aRemovePos.depth()) return;

    for (int i = 0; i < aRemovePos.depth() - 1; ++i)
    {
        if (row(i) != aRemovePos.row(i)) return;
    }

    int depthIndex = aRemovePos.depth() - 1;

    if (row(depthIndex) > aRemovePos.row(depthIndex))
    {
        --mRows[depthIndex];
        XC_ASSERT(mRows[depthIndex] >= 0);
    }
    else if (row(depthIndex) == aRemovePos.row(depthIndex))
    {
        mIsValid = false;
    }
}

void TreePos::updateByInsert(const TreePos& aInsertPos)
{
    if (!isValid() || !aInsertPos.isValid()) return;
    if (depth() < aInsertPos.depth()) return;

    for (int i = 0; i < aInsertPos.depth() - 1; ++i)
    {
        if (row(i) != aInsertPos.row(i)) return;
    }

    int depthIndex = aInsertPos.depth() - 1;

    if (row(depthIndex) >= aInsertPos.row(depthIndex))
    {
        ++mRows[depthIndex];
    }
}

TreePos TreePos::parent() const
{
    TreePos result;

    result.mIsValid = depth() > 1;
    for (int i = 0; i < depth() - 1; ++i)
    {
        result.mRows.push_back(this->mRows[i]);
    }

    return result;
}

bool TreePos::operator==(const TreePos& aRhs) const
{
    if (depth() != aRhs.depth()) return false;

    for (int i = 0; i < depth(); ++i)
    {
        if (row(i) != aRhs.row(i)) return false;
    }
    return true;
}

void TreePos::setValidity(bool aIsValid)
{
    mIsValid = aIsValid;
}

void TreePos::pushRow(int aRow)
{
    if (aRow < 0) mIsValid = false;
    mRows.push_back(aRow);
}

void TreePos::dump() const
{
    QString text;
    for (size_t i = 0; i < mRows.size(); ++i)
    {
        QString row;
        row.sprintf("%d,", mRows[i]);
        text += row;
    }
    qDebug() << text;
}

} // namespace util
