#ifndef CORE_TIMEKEYBLENDER_H
#define CORE_TIMEKEYBLENDER_H

#include <array>
#include <QVector3D>
#include "util/ITreeSeeker.h"
#include "core/ObjectTree.h"
#include "core/TimeInfo.h"
#include "core/TimeLineEvent.h"
#include "core/TimeKeyExpans.h"
#include "core/TimeKeyGatherer.h"
#include "core/TimeCacheLock.h"

namespace core
{

class TimeKeyBlender
{
public:
    struct SeekData
    {
        const ObjectNode* objNode;
        TimeKeyExpans* expans;
    };
    typedef util::ITreeSeeker<SeekData> SeekerType;
    typedef SeekerType::Position PositionType;

    static SRTExpans getSRTExpans(
            ObjectNode& aNode, const TimeInfo& aTime);
    static QMatrix4x4 getWorldMatrix(
            ObjectNode& aNode, const TimeInfo& aTime);
    static QMatrix4x4 getRelativeMatrix(
            ObjectNode& aNode, const TimeInfo& aTime,
            const ObjectNode* aParent);
    static LayerMesh* getAreaMesh(
            ObjectNode& aNode, const TimeInfo& aTime);

    TimeKeyBlender(ObjectTree& aTree);
    TimeKeyBlender(ObjectNode& aRootNode, bool aUseWorking);
    TimeKeyBlender(SeekerType& aSeeker, PositionType aRoot);

    void updateCurrents(ObjectNode* aRootNode, const TimeInfo& aTime);
    void clearCaches(ObjectNode* aRootNode);
    void clearCaches(TimeLineEvent& aEvent);

private:
    static SRTKey::Data getDefaultSRT(const ObjectNode& aNode);
    static std::array<QVector3D, 2> catmullRomVels(const TimeKeyGatherer& aBlend);
    static void getSRTData(
            TimeKeyExpans& aCurrent,
            const ObjectNode& aNode,
            const TimeInfo& aTime);
    static MeshKey* getMeshKey(
            const ObjectNode& aNode,
            const TimeInfo& aTime);

    //QMatrix4x4 getParentMatrix(PositionType aPos, const TimeInfo& aTime, int aCacheFrame);
    void blendSRTKey(PositionType aPos, const TimeInfo& aTime);
    void blendOpaKey(PositionType aPos, const TimeInfo& aTime);
    void blendBoneKey(PositionType aPos, const TimeInfo& aTime);
    void blendPoseKey(PositionType aPos, const TimeInfo& aTime);
    void blendMeshKey(PositionType aPos, const TimeInfo& aTime);
    void blendFFDKey(PositionType aPos, const TimeInfo& aTime);
    void buildPosePalette(ObjectNode& aNode, PosePalette::KeyPairs& aPairs);
    void setBoneInfluenceMaps(ObjectNode& aNode, const BoneKey* aKey,
                              const TimeInfo& aTime);

    SeekerType* mSeeker;
    SeekerType::Position mRoot;
};

} // namespace core

#endif // CORE_TIMEKEYBLENDER_H
