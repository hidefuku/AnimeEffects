#include "gui/MSVCBackTracer.h"

#if defined(USE_MSVC_BACKTRACE)
#include <QDebug>
#include <Windows.h>
#include <stdlib.h>
#include <imagehlp.h>
#include <excpt.h>

#pragma comment(lib, "imagehlp.lib")

BackTracer gBackTracer;

BackTracer::BackTracer()
    : mProcess()
    , mReady()
{
    mProcess = ::GetCurrentProcess();
    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
    if (SymInitialize((HANDLE)mProcess, NULL, TRUE))
    {
        mReady = true;
    }
}

BackTracer::~BackTracer()
{
    if (mReady)
    {
        mReady = false;
        SymCleanup((HANDLE)mProcess);
    }
}

void BackTracer::dumpCurrent() const
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

void BackTracer::getSymbolText(void* aAddress, char* aOutBuffer, int aLength) const
{
    if (!mReady) return;

#ifdef _WIN64
    typedef DWORD64 XDWord;
#else
    typedef DWORD XDWord;
#endif

    // image module
    IMAGEHLP_MODULE imageModule = { sizeof(IMAGEHLP_MODULE) };
    if (!SymGetModuleInfo((HANDLE)mProcess, (XDWord)aAddress, &imageModule))
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
    if (!SymGetSymFromAddr((HANDLE)mProcess, (XDWord)aAddress, &displacement, &imageSymbol))
    {
        _snprintf_s(aOutBuffer, aLength, _TRUNCATE, "%s @ ??? @ ???", imageModule.ModuleName);
        return;
    }

    // image line
    IMAGEHLP_LINE imageLine = { sizeof(IMAGEHLP_LINE) };
    DWORD disp32 = (DWORD)displacement;
    if (!SymGetLineFromAddr((HANDLE)mProcess, (XDWord)aAddress, &disp32, &imageLine))
    {
        _snprintf_s(aOutBuffer, aLength, _TRUNCATE, "%s @ %s @ %s+%d",
                    imageModule.ModuleName, imageSymbol.Name, imageSymbol.Name,
                    (int)((char*)aAddress - (char*)imageLine.Address));
        return;
    }

    _snprintf_s(aOutBuffer, aLength, _TRUNCATE, "%s @ %s @ %s:%d",
                imageModule.ModuleName, imageSymbol.Name, imageLine.FileName, imageLine.LineNumber);
}

#endif // USE_MSVC_BACKTRACE
