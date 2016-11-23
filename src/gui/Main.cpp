#if !defined(QT_NO_DEBUG) && defined(_MSC_VER)
#define USE_MSVC_MEMORYLEAK_DEBUG
#define USE_MSVC_BACKTRACE
#endif

#if defined(USE_MSVC_MEMORYLEAK_DEBUG)

#include <stdlib.h>
#include <new>
#include <QMutex>
#include <QMutexLocker>
#include "XC.h"

struct MyMemoryFooter
{
    uint32 sign;
    uint32 id;

    void init(uint32 aId)
    {
        sign = 0xfa00cc8b;
        id = aId;
    }
    bool hasValidSign() const
    {
        return sign == 0xfa00cc8b;
    }
};

static bool sMemoryRegisterAlloc = false;

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

    MemoryRegister()
        : mBlocks()
        , mBlockCount()
        , mMaxBlockCount()
        , mCount()
        , mLock()
    {
    }

    void final()
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

    ~MemoryRegister()
    {
        final();
    }

    void push(const void* aPtr, uint64 aSize, uint32 aId)
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

    Tag pop(const void* aPtr)
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

    Tag pop(uint32 aId)
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

    Tag* find(const void* aAddress)
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

    void dumpAll()
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

private:
    void makeBlocks(int aBlockCount)
    {
        sMemoryRegisterAlloc = true;
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
        sMemoryRegisterAlloc = false;
    }

    Tag** mBlocks;
    int mBlockCount;
    int mMaxBlockCount;
    int mCount;
    QMutex mLock;
};

static MemoryRegister sMemoryRegister;
static uint32 sTotalAllocCount = 0;
static int sMaxAllocCount = 0;
static int sAllocCount = 0;

// replaceable allocation
void* operator new(size_t aSize)
{
    void* ptr = malloc(aSize + sizeof(MyMemoryFooter));
    XC_PTR_ASSERT(ptr);
    //if (!ptr) return nullptr;

    ++sAllocCount;
    ++sTotalAllocCount;
    sMaxAllocCount = sMaxAllocCount > sAllocCount ? sMaxAllocCount : sAllocCount;

    auto foot = (MyMemoryFooter*)((uint8*)ptr + aSize);
    foot->init(sTotalAllocCount);

    sMemoryRegister.push(ptr, (uint64)aSize, (uint32)sTotalAllocCount);

    return ptr;
}

// This operator has to supporse a pointer which be allocated in a dynamic link library.
// Of course, it's illegal to use different overloading new/delete, but it's powerful for memory debugging.
void operator delete(void* aPtr)
{
    if (!aPtr) return;

    auto tag = sMemoryRegister.pop(aPtr);
    if (tag)
    {
        --sAllocCount;
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

    ++sAllocCount;
    ++sTotalAllocCount;
    sMaxAllocCount = sMaxAllocCount > sAllocCount ? sMaxAllocCount : sAllocCount;

    auto foot = (MyMemoryFooter*)((uint8*)ptr + aSize);
    foot->init(sTotalAllocCount);

    sMemoryRegister.push(ptr, (uint64)aSize, sTotalAllocCount);

    return ptr;
}

void operator delete[](void* aPtr)
{
    if (!aPtr) return;

    auto tag = sMemoryRegister.pop(aPtr);
    if (tag)
    {
        --sAllocCount;
        auto foot = (MyMemoryFooter*)((uint8*)aPtr + tag.size);
        XC_MSG_ASSERT(foot->hasValidSign(), "memory corruption detected. %x, %u", foot->sign, foot->id);
    }
    free(aPtr);
}
#endif

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QScopedPointer>
#include <QDir>
#include <QMessageBox>
#include "XC.h"
#include "gl/Global.h"
#include "ctrl/System.h"
#include "gui/MainWindow.h"
#include "gui/GUIResources.h"

#if defined(USE_MSVC_MEMORYLEAK_DEBUG)
#include <Windows.h>
int myAllocHook(int aAllocType, void* aData,
        size_t aSize, int aBlockUse, long aRequest,
        const unsigned char* aFileName, int aLine)
{
    (void)aSize;
    (void)aFileName;
    if (aBlockUse == _CRT_BLOCK) return TRUE;
    if (aBlockUse != _NORMAL_BLOCK) return TRUE;
    if (sMemoryRegisterAlloc) return TRUE;

    if (aAllocType == _HOOK_FREE)
    {
        char* fileName = nullptr;
        const int result = _CrtIsMemoryBlock(aData, 0, &aRequest, &fileName, &aLine);
        if (result)
        {
            sMemoryRegister.pop((uint32)aRequest);
        }
    }
    return TRUE;
}
#endif


#if defined(USE_MSVC_BACKTRACE)
#include <Windows.h>
#include <stdlib.h>
#include <imagehlp.h>
#include <excpt.h>

#pragma comment(lib, "imagehlp.lib")

class BackTracer
{
public:
    BackTracer()
        : mProcess()
        , mReady()
    {
        mProcess = ::GetCurrentProcess();
        SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
        if (SymInitialize(mProcess, NULL, TRUE))
        {
            mReady = true;
        }
    }

    ~BackTracer()
    {
        if (mReady)
        {
            mReady = false;
            SymCleanup(mProcess);
        }
    }

    void dumpCurrent() const
    {
        if (!mReady)
        {
            qDebug("BackTracer::dumpCurrent : BackTracer is not initialized.");
            return;
        }

        const int kMaxStack = 62; // must be less than 63.
        const int kMaxText = 256;

        void* stack[kMaxStack];
        auto count = CaptureStackBackTrace(0, kMaxStack, stack, NULL);
        char text[kMaxText];

        for(int i = 0; i < count; i++)
        {
            getSymbolText(stack[i], text, kMaxText);
            qDebug("%d : %018p @ %s", i, stack[i], text);
        }
    }

private:
    void getSymbolText(void* aAddress, char* aOutBuffer, int aLength) const
    {
        if (!mReady) return;

#ifdef _WIN64
        typedef DWORD64 XDWord;
#else
        typedef DWORD XDWord;
#endif

        // image module
        IMAGEHLP_MODULE imageModule = { sizeof(IMAGEHLP_MODULE) };
        if (!SymGetModuleInfo(mProcess, (XDWord)aAddress, &imageModule))
        {
            _snprintf_s(aOutBuffer, aLength, _TRUNCATE, "??? @ ??? @ ???");
            return;
        }

        // image symbol
        struct SymbolBuffer
        {
            IMAGEHLP_SYMBOL symbol;
            BYTE buffer[MAX_PATH];
        };
        SymbolBuffer symBuffer;
        memset(&symBuffer, 0, sizeof(SymbolBuffer));
        symBuffer.symbol.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        symBuffer.symbol.MaxNameLength = MAX_PATH;
        IMAGEHLP_SYMBOL& imageSymbol = symBuffer.symbol;

        XDWord displacement = 0;
        if (!SymGetSymFromAddr(mProcess, (XDWord)aAddress, &displacement, &imageSymbol))
        {
            _snprintf_s(aOutBuffer, aLength, _TRUNCATE, "%s @ ??? @ ???", imageModule.ModuleName);
            return;
        }

        // image line
        IMAGEHLP_LINE imageLine = { sizeof(IMAGEHLP_LINE) };
        DWORD disp32 = (DWORD)displacement;
        if (!SymGetLineFromAddr(mProcess, (XDWord)aAddress, &disp32, &imageLine))
        {
            _snprintf_s(aOutBuffer, aLength, _TRUNCATE, "%s @ %s @ %s+%d",
                        imageModule.ModuleName, imageSymbol.Name, imageSymbol.Name,
                        (int)((char*)aAddress - (char*)imageLine.Address));
            return;
        }

        _snprintf_s(aOutBuffer, aLength, _TRUNCATE, "%s @ %s @ %s:%d",
                    imageModule.ModuleName, imageSymbol.Name, imageLine.FileName, imageLine.LineNumber);
    }

    HANDLE mProcess;
    bool mReady;
};
static BackTracer sBackTracer;
#endif


class AEAssertHandler : public XCAssertHandler
{
public:
    virtual void failure() const
    {
#if defined(USE_MSVC_BACKTRACE)
        sBackTracer.dumpCurrent();
#endif
    }
};

class AEErrorHandler : public XCErrorHandler
{
public:
    virtual void critical(
            const QString& aText, const QString& aInfo,
            const QString& aDetail) const
    {
        XC_REPORT() << aText << "\n" << aInfo << "\n" << aDetail;
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Fatal Error");

        msgBox.setText(aText);
        msgBox.setInformativeText(aInfo);
        if (!aDetail.isEmpty())
        {
            msgBox.setDetailedText(aDetail);
        }

        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();

        std::exit(EXIT_FAILURE);
    }
};

XCAssertHandler* gXCAssertHandler = nullptr;
XCErrorHandler* gXCErrorHandler = nullptr;
static AEAssertHandler sAEAssertHandler;


int entryPoint(int argc, char *argv[]);

#if defined(USE_MSVC_BACKTRACE)
#define TRY_ACTION_WITH_EXCEPT(action) \
    __try{ action; } \
    __except(EXCEPTION_EXECUTE_HANDLER) \
        { qDebug("exception occurred.(%x)", GetExceptionCode()); sBackTracer.dumpCurrent(); std::abort(); }
#else
#define TRY_ACTION_WITH_EXCEPT(action) action
#endif

int main(int argc, char *argv[])
{
    gXCAssertHandler = &sAEAssertHandler;

#if defined(USE_MSVC_MEMORYLEAK_DEBUG)
    _CrtSetAllocHook(myAllocHook);
#endif

    int result = 0;
    TRY_ACTION_WITH_EXCEPT(result = entryPoint(argc, argv));

#if defined(USE_MSVC_MEMORYLEAK_DEBUG)
    sMemoryRegister.dumpAll();
    sMemoryRegister.final();
#endif

    return result;
}

int entryPoint(int argc, char *argv[])
{
    int result = 0;

    // create qt application
    QApplication app(argc, argv);
    // unicode argments
    const QStringList uniArgs = app.arguments();

    // application path
    const QString exeFilePath(uniArgs.at(0));
    const QString appDir(QFileInfo(exeFilePath).dir().path());
    const QString resourceDir(appDir + "/data");
    const QString cacheDir(appDir + "/data/cache");

    XC_DEBUG_REPORT() << "exe path =" << exeFilePath;

    // set fatal error handler
    static AEErrorHandler aeErrorHandler;
    gXCErrorHandler = &aeErrorHandler;

    // set organization and application name for the application setting
    QCoreApplication::setOrganizationName("AnimeEffectsProject");
    QCoreApplication::setApplicationName("AnimeEffects");

    // language
    {
        QString translation;
        auto language = QLocale::system().language();
        if (language == QLocale::Japanese)
        {
            translation = "translation_ja";
        }

        if (!translation.isEmpty())
        {
            auto translator = new QTranslator();
            translator->load(translation, "data/locale");
            app.installTranslator(translator);
        }
    }

    {
        // load constant gui resources
        QScopedPointer<gui::GUIResources> resources(new gui::GUIResources(resourceDir));

        // create system logic core
        QScopedPointer<ctrl::System> system(new ctrl::System(resourceDir, cacheDir));

        // create main window
        QScopedPointer<gui::MainWindow> mainWindow(new gui::MainWindow(*system, *resources));

        qDebug() << "show main window";
        // show main window
        mainWindow->showWithSettings();
        // set opengl device info
        system->setGLDeviceInfo(gl::DeviceInfo::instance());

#if !defined(QT_NO_DEBUG)
        qDebug() << "test new project";
        const QString testPath = resourceDir + "/sample.psd";
        mainWindow->testNewProject(testPath);
#endif

        // execute application
        result = app.exec();

        // save settings(window status, etc.)
        mainWindow->saveCurrentSettings(result);

        // bind gl context for destructors
        gl::Global::makeCurrent();
        qDebug() << "dst main window";
        mainWindow.reset();
        qDebug() << "dst system";
        system.reset();
        qDebug() << "dst resources";
        resources.reset();
        qDebug() << "dst major components";
    }

    return result;
}
