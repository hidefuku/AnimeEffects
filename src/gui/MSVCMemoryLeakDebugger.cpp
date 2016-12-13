#include "gui/MSVCMemoryLeakDebugger.h"
#include <QMutexLocker>

#if defined(USE_MSVC_MEMORYLEAK_DEBUG)

MemoryRegister gMemoryRegister;
bool gMemoryRegisterAlloc = false;
uint32 gTotalAllocCount = 0;
int gMaxAllocCount = 0;
int gAllocCount = 0;

MemoryRegister::MemoryRegister()
    : mBlocks()
    , mBlockCount()
    , mMaxBlockCount()
    , mCount()
    , mLock()
{
}

MemoryRegister::~MemoryRegister()
{
    final();
}

void MemoryRegister::final()
{
    QMutexLocker locker(&mLock);
    if (!mBlocks) return;
    for (int i = 0; i < mMaxBlockCount; ++i)
    {
        free(mBlocks[i]);
    }
    free(mBlocks);
    mBlocks = nullptr;
    mMaxBlockCount = 0;
    mBlockCount = 0;
    mCount = 0;
}

void MemoryRegister::push(const void* aPtr, uint64 aSize, uint32 aId)
{
    QMutexLocker locker(&mLock);

    const int blockIndex = mCount / kBlockSize;
    makeBlocks(blockIndex + 1);

    const int index = mCount - blockIndex * kBlockSize;
    Tag& tag = (mBlocks[blockIndex])[index];
    tag.ptr  = aPtr;
    tag.size = aSize;
    tag.id   = aId;

    ++mCount;
}

MemoryRegister::Tag MemoryRegister::pop(const void* aPtr)
{
    QMutexLocker locker(&mLock);

    for (int k = mBlockCount - 1; k >= 0; --k)
    {
        Tag* tags = mBlocks[k];
        const int tagCount = std::min((int)kBlockSize, mCount - k * kBlockSize);
        for (int i = tagCount - 1; i >= 0; --i)
        {
            if (tags[i].ptr == aPtr)
            {
                auto tag = tags[i];
                const int lastBlockIndex = (mCount - 1) / kBlockSize;
                const int lastTagIndex = (mCount - 1) - lastBlockIndex * kBlockSize;
                tags[i] = (mBlocks[lastBlockIndex])[lastTagIndex];
                --mCount;
                return tag;
            }
        }
    }
    Tag tag = {};
    return tag;
}

MemoryRegister::Tag MemoryRegister::pop(uint32 aId)
{
    QMutexLocker locker(&mLock);

    for (int k = mBlockCount - 1; k >= 0; --k)
    {
        Tag* tags = mBlocks[k];
        const int tagCount = std::min((int)kBlockSize, mCount - k * kBlockSize);
        for (int i = tagCount - 1; i >= 0; --i)
        {
            if (tags[i].id == aId)
            {
                auto tag = tags[i];
                const int lastBlockIndex = (mCount - 1) / kBlockSize;
                const int lastTagIndex = (mCount - 1) - lastBlockIndex * kBlockSize;
                tags[i] = (mBlocks[lastBlockIndex])[lastTagIndex];
                --mCount;
                return tag;
            }
        }
    }
    Tag tag = {};
    return tag;
}

MemoryRegister::Tag* MemoryRegister::find(const void* aAddress)
{
    QMutexLocker locker(&mLock);

    for (int k = mBlockCount - 1; k >= 0; --k)
    {
        Tag* tags = mBlocks[k];
        const int tagCount = std::min((int)kBlockSize, mCount - k * kBlockSize);
        for (int i = tagCount - 1; i >= 0; --i)
        {
            if (tags[i].ptr == aAddress)
            {
                return &(tags[i]);
            }
        }
    }
    return nullptr;
}

void MemoryRegister::dumpAll()
{
    qDebug("dump all memory : count = %d, block count = %d", mCount, mBlockCount);
    for (int k = 0; k < mBlockCount; ++k)
    {
        Tag* tags = mBlocks[k];
        const int tagCount = std::min((int)kBlockSize, mCount - k * kBlockSize);
        for (int i = 0; i < tagCount; ++i)
        {
            auto& tag = tags[i];
            qDebug() << "dump memory :" << tag.ptr << tag.size << tag.id;
        }
    }
}

void MemoryRegister::makeBlocks(int aBlockCount)
{
    gMemoryRegisterAlloc = true;
    if (mMaxBlockCount < aBlockCount)
    {
        Tag** newBlocks = (Tag**)malloc(sizeof(Tag*) * aBlockCount);
        XC_PTR_ASSERT(newBlocks);
        for (int i = 0; i < mMaxBlockCount; ++i)
        {
            newBlocks[i] = mBlocks[i];
        }
        for (int i = mMaxBlockCount; i < aBlockCount; ++i)
        {
            newBlocks[i] = (Tag*)malloc(sizeof(Tag) * kBlockSize);
            XC_PTR_ASSERT(newBlocks[i]);
        }
        free(mBlocks);
        mBlocks = newBlocks;
        mMaxBlockCount = aBlockCount;
    }
    mBlockCount = aBlockCount;
    gMemoryRegisterAlloc = false;
}

//-------------------------------------------------------------------------------------------------
// replaceable allocation
void* operator new(size_t aSize)
{
    void* ptr = malloc(aSize + sizeof(MyMemoryFooter));
    XC_PTR_ASSERT(ptr);
    //if (!ptr) return nullptr;

    ++gAllocCount;
    ++gTotalAllocCount;
    gMaxAllocCount = gMaxAllocCount > gAllocCount ? gMaxAllocCount : gAllocCount;

    auto foot = (MyMemoryFooter*)((uint8*)ptr + aSize);
    foot->init(gTotalAllocCount);

    gMemoryRegister.push(ptr, (uint64)aSize, (uint32)gTotalAllocCount);

    return ptr;
}

// This operator has to supporse a pointer which be allocated in a dynamic link library.
// Of course, it's illegal to use different overloading new/delete, but it's powerful for memory debugging.
void operator delete(void* aPtr)
{
    if (!aPtr) return;

    auto tag = gMemoryRegister.pop(aPtr);
    if (tag)
    {
        --gAllocCount;
        auto foot = (MyMemoryFooter*)((uint8*)aPtr + tag.size);
        XC_MSG_ASSERT(foot->hasValidSign(), "memory corruption detected. %x, %u", foot->sign, foot->id);
    }
    free(aPtr);
}

void* operator new[](size_t aSize)
{
    void* ptr = malloc(aSize + sizeof(MyMemoryFooter));
    XC_PTR_ASSERT(ptr);
    //if (!ptr) return nullptr;

    ++gAllocCount;
    ++gTotalAllocCount;
    gMaxAllocCount = gMaxAllocCount > gAllocCount ? gMaxAllocCount : gAllocCount;

    auto foot = (MyMemoryFooter*)((uint8*)ptr + aSize);
    foot->init(gTotalAllocCount);

    gMemoryRegister.push(ptr, (uint64)aSize, gTotalAllocCount);

    return ptr;
}

void operator delete[](void* aPtr)
{
    if (!aPtr) return;

    auto tag = gMemoryRegister.pop(aPtr);
    if (tag)
    {
        --gAllocCount;
        auto foot = (MyMemoryFooter*)((uint8*)aPtr + tag.size);
        XC_MSG_ASSERT(foot->hasValidSign(), "memory corruption detected. %x, %u", foot->sign, foot->id);
    }
    free(aPtr);
}

//-------------------------------------------------------------------------------------------------
#include <Windows.h>
int myAllocHook(int aAllocType, void* aData,
        size_t aSize, int aBlockUse, long aRequest,
        const unsigned char* aFileName, int aLine)
{
    (void)aSize;
    (void)aFileName;
    if (aBlockUse == _CRT_BLOCK) return TRUE;
    if (aBlockUse != _NORMAL_BLOCK) return TRUE;
    if (gMemoryRegisterAlloc) return TRUE;

    if (aAllocType == _HOOK_FREE)
    {
        char* fileName = nullptr;
        const int result = _CrtIsMemoryBlock(aData, 0, &aRequest, &fileName, &aLine);
        if (result)
        {
            gMemoryRegister.pop((uint32)aRequest);
        }
    }
    return TRUE;
}

#endif // USE_MSVC_MEMORYLEAK_DEBUG
