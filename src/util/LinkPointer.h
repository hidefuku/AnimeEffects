#ifndef UTIL_LINKPOINTER_H
#define UTIL_LINKPOINTER_H

#include "util/LifeLink.h"
#include "XC.h"

namespace util
{

// If you control this class from multiple threads, you need to synchronize it yourself.
template <typename tObject>
class LinkPointer : public LifeLink::Node
{
    typedef void (LinkPointer::*SafeBoolType)() const;
    void dummyFuncForSafeBoolIdiom() const {}
public:
    LinkPointer()
        : mAddress(NULL)
    {
    }

    LinkPointer(const LinkPointer<tObject>& aOther)
    {
        LifeLink::Node::operator=(aOther);
        mAddress = aOther.mAddress;
    }

    template<typename tPointeeObject>
    explicit LinkPointer(const LifeLink::Pointee<tPointeeObject>& aPointee)
    {
        if (aPointee)
        {
            LifeLink::Node::operator=(*aPointee.lifeLink);
        }
        else
        {
            *static_cast<LifeLink::Node*>(this) = LifeLink::Node();
        }
        mAddress = aPointee.address;
    }

    template<typename tPointeeObject>
    LinkPointer& operator=(const LifeLink::Pointee<tPointeeObject>& aPointee)
    {
        if (aPointee)
        {
            LifeLink::Node::operator=(*aPointee.lifeLink);
        }
        else
        {
            *static_cast<LifeLink::Node*>(this) = LifeLink::Node();
        }
        mAddress = aPointee.address;
        return *this;
    }

    void reset()
    {
        *static_cast<LifeLink::Node*>(this) = LifeLink::Node();
        mAddress = NULL;
    }

    bool isLink() const
    {
        return LifeLink::Node::isLinking() && mAddress != NULL;
    }

    operator SafeBoolType() const
    {
        return isLinking() ? &LinkPointer::dummyFuncForSafeBoolIdiom : 0;
    }

    tObject* operator->() const
    {
        return isLinking() ? mAddress : NULL;
    }

    tObject& operator*() const
    {
        XC_ASSERT(isLinking());
        return isLinking() ? *mAddress : *static_cast<tObject*>(NULL);
    }

    tObject* get() const
    {
        return isLinking() ? mAddress : NULL;
    }

private:
    tObject* mAddress;
};

} // namespace util

#endif // UTIL_LINKPOINTER_H
