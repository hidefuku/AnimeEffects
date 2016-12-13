#ifndef GUI_MSVCMEMORYLEAKDEBUGGER_H
#define GUI_MSVCMEMORYLEAKDEBUGGER_H

#if !defined(QT_NO_DEBUG) && defined(_MSC_VER)
#define USE_MSVC_MEMORYLEAK_DEBUG
#endif

#if defined(USE_MSVC_MEMORYLEAK_DEBUG)

#include <stdlib.h>
#include <new>
#include <QMutex>
#include "XC.h"

struct MyMemoryFooter
{
    uint32 sign;
    uint32 id;
    void init(uint32 aId) { sign = 0xfa00cc8b; id = aId; }
    bool hasValidSign() const { return sign == 0xfa00cc8b; }
};

class MemoryRegister
{
    enum { kBlockSize = 4096 };

public:
    struct Tag
    {
        const void* ptr;
        uint64 size;
        uint32 id;
        explicit operator bool() const { return ptr; }
    };

    MemoryRegister();
    ~MemoryRegister();

    void final();
    void push(const void* aPtr, uint64 aSize, uint32 aId);
    Tag pop(const void* aPtr);
    Tag pop(uint32 aId);
    Tag* find(const void* aAddress);
    void dumpAll();

private:
    void makeBlocks(int aBlockCount);

    Tag** mBlocks;
    int mBlockCount;
    int mMaxBlockCount;
    int mCount;
    QMutex mLock;
};

void* operator new(size_t aSize);
void operator delete(void* aPtr);
void* operator new[](size_t aSize);
void operator delete[](void* aPtr);

int myAllocHook(int aAllocType, void* aData, size_t aSize, int aBlockUse,
                long aRequest, const unsigned char* aFileName, int aLine);

#endif // USE_MSVC_MEMORYLEAK_DEBUG

#endif // GUI_MSVCMEMORYLEAKDEBUGGER_H
