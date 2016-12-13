#ifndef GUI_MSVCBACKTRACER_H
#define GUI_MSVCBACKTRACER_H

#if !defined(QT_NO_DEBUG) && defined(_MSC_VER)
#define USE_MSVC_BACKTRACE
#endif

#if defined(USE_MSVC_BACKTRACE)
class BackTracer
{
public:
    BackTracer();
    ~BackTracer();
    void dumpCurrent() const;

private:
    void getSymbolText(void* aAddress, char* aOutBuffer, int aLength) const;

    void* mProcess;
    bool mReady;
};
#endif // USE_MSVC_BACKTRACE

#endif // GUI_MSVCBACKTRACER_H
