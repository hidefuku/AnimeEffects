#include <fstream>
#include "core/Serializer.h"
#include "ctrl/ProjectSaver.h"

namespace ctrl
{

ProjectSaver::ProjectSaver()
    : mLog()
{
}

bool ProjectSaver::save(const QString& aFilePath, const core::Project& aProject)
{
    std::ofstream file(aFilePath.toLocal8Bit(), std::ios::out | std::ios::binary);

    if (file.fail())
    {
        mLog = "can not create a project file";
        return false;
    }

    util::StreamWriter out(file);

    if (!writeHeader(out))
    {
        mLog = "failed to write header";
        return false;
    }

    if (!writeGlobalBlock(out, aProject))
    {
        mLog = "failed to write global block";
        return false;
    }

    core::Serializer serializer(out);

    if (!aProject.resourceHolder().serialize(serializer))
    {
        mLog = "failed to write resources block";
        return false;
    }

    // write object tree
    if (!aProject.objectTree().serialize(serializer))
    {
        mLog = "failed to write object tree block";
        return false;
    }

    mLog = "success";
    return true;
}

bool ProjectSaver::writeHeader(util::StreamWriter& aOut)
{
    static const std::array<uint8, 6> kSignature{ 'A', 'N', 'I', 'M', 'F', 'X' };
    static const std::array<uint8, 2> kEndian{ 0xff, 0x00 };
    static const uint32 kMajorVersion = AE_PROJECT_FORMAT_MAJOR_VERSION;
    static const uint32 kMinorVersion = AE_PROJECT_FORMAT_MINOR_VERSION;
    static const int kReserveSize = 16;

    // signature
    aOut.write(kSignature);
    // endian (little)
    aOut.write(kEndian);
    // major version
    aOut.write(kMajorVersion);
    // minor version
    aOut.write(kMinorVersion);
    // reserved
    aOut.writeZero(kReserveSize);

    return !aOut.isFailed();
}

bool ProjectSaver::writeGlobalBlock(util::StreamWriter& aOut, const core::Project& aProject)
{
    static const std::array<sint8, 4> kSignature{ 'G', 'L', 'B', 'L' };
    static const uint32 kBlockLength = 64;
    static const int kReserveSize = 47;

    const QSize imageSize = aProject.attribute().imageSize();
    const int maxFrame = aProject.attribute().maxFrame();
    const int fps = aProject.attribute().fps();
    const bool loop = aProject.attribute().loop();
    XC_ASSERT(maxFrame > 0);
    XC_ASSERT(fps > 0);

    // block signature
    aOut.write(kSignature);
    // block length
    aOut.write(kBlockLength);

    // width
    aOut.write((uint32)imageSize.width());
    // height
    aOut.write((uint32)imageSize.height());
    // max frame
    aOut.write((uint32)maxFrame);
    // fps
    aOut.write((uint32)fps);
    // loop
    aOut.write((uint8)loop);

    // reserved
    aOut.writeZero(kReserveSize);

    return !aOut.isFailed();
}

} // namespace ctrl
