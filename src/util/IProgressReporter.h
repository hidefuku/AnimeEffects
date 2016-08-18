#ifndef UTIL_IPROGRESSREPORTER
#define UTIL_IPROGRESSREPORTER

#include <QString>

namespace util
{

class IProgressReporter
{
public:
    virtual ~IProgressReporter() {}

    virtual void setSection(const QString& aSection) = 0;
    virtual void setMaximum(int aMax) = 0;
    virtual void setProgress(int aValue) = 0;
    virtual bool wasCanceled() const = 0;
};

} // namespace util

#endif // UTIL_IPROGRESSREPORTER

