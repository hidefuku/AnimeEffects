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

    static int imageSizeMax() { return 30000; }
    static int imageSizeMin() { return 1; }

    static int imageCellSizeMax() { return 30000; }
    static int imageCellSizeMin() { return 2; }

    static int imageCellCountMax() { return 1000000; }
};

} // namespace core

#endif // CORE_CONSTANT_H

