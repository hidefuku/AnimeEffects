#ifndef CMND_VECTOR
#define CMND_VECTOR

#include <vector>
#include "cmnd/Base.h"

namespace cmnd
{

class Vector : public std::vector<Base*>
{
public:
    Vector() {}
    virtual ~Vector() {}

    void push(Base* aCommand)
    {
        if (aCommand)
        {
            this->push_back(aCommand);
        }
    }
    void push(const Vector& aCommands)
    {
        if (!aCommands.empty())
        {
            this->insert(this->end(), aCommands.begin(), aCommands.end());
        }
    }
};

} // namespace cmnd

#endif // CMND_VECTOR

