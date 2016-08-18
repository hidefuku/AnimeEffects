#ifndef CORE_CONSTANT_H
#define CORE_CONSTANT_H

namespace core
{

class Constant
{
public:
    static float transMax() { return  10000000.0f; }
    static float transMin() { return -10000000.0f; }

    static float scaleMax() { return  10000.0f; }
    static float scaleMin() { return -10000.0f; }

    static float rotateMax() { return  10000.0f; }
    static float rotateMin() { return -10000.0f; }

    static float normalizable() { return 0.01f; }
    static float dividable() { return 0.001f; }
};

} // namespace core

#endif // CORE_CONSTANT_H

