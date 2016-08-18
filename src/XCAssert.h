#ifndef XCASSERT_H
#define XCASSERT_H

#include <string>
#include <QString>
#include "XCReport.h"

class XCAssertHandler
{
public:
    virtual void failure() const = 0;
    virtual ~XCAssertHandler() {}
};

extern XCAssertHandler* gXCAssertHandler;

#define XC_ASSERT(cond) \
    do { if (!(cond)) { if (gXCAssertHandler) gXCAssertHandler->failure(); Q_ASSERT(0); } } while(0)

#define XC_MSG_ASSERT(cond, ...) \
    do { if (!(cond)) { XC_DEBUG_REPORT(__VA_ARGS__); if (gXCAssertHandler) gXCAssertHandler->failure(); Q_ASSERT(0); } } while (0)

#define XC_PTR_ASSERT(ptr) \
    do { if (!(ptr)) { if (gXCAssertHandler) gXCAssertHandler->failure(); Q_ASSERT(0); } } while(0)


class XCErrorHandler
{
public:
    virtual void critical(
            const QString& aText, const QString& aInfo,
            const QString& aDetail) const = 0;
    virtual ~XCErrorHandler() {}
};

extern XCErrorHandler* gXCErrorHandler;

#define XC_FATAL_ERROR(text, info, detail) \
    do { if (gXCErrorHandler) gXCErrorHandler->critical(text, info, detail); } while (0)

#endif // XCASSERT_H
