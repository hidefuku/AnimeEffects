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
        ObjectNode* objNode;
        TimeKeyExpans* expans;
    };
    typedef util::ITreeSeeker<SeekData, ObjectNode*> SeekerType;
    typedef SeekerType::Position PositionType;


    static QMatrix4x4 getLocalSRMatrix(
            const ObjectNode& aNode, const TimeInfo& aTime);

    static QMatrix4x4 getWorldMatrix(
            ObjectNode& aNode, const TimeInfo& aTime);
    static QMatrix4x4 getRelativeMatrix(
            ObjectNode& aNode, const TimeInfo& aTime,
            const ObjectNode* aParent);
    static LayerMesh* getAreaMesh(
            ObjectNode& aNode, const TimeInfo& aTime);
    static BoneKey* getAreaBone(
            ObjectNode& aNode, const TimeInfo& aTime);
    static BoneKey* getNearestInfluencerBone(
            ObjectNode& aNode, const TimeInfo& aTime);
    static QVector2D getImageOffset(
            ObjectNode& aNode, const TimeInfo& aTime);
    static QVector2D getOriginOffset(
            ObjectNode& aNode, const TimeInfo& aTime);

    TimeKeyBlender(ObjectTree& aTree);
    TimeKeyBlender(ObjectNode& aRootNode, bool aUseWorking);
    TimeKeyBlender(SeekerType& aSeeker, PositionType aRoot);

    void updateCurrents(ObjectNode* aRootNode, const TimeInfo& aTime);
    void clearCaches(ObjectNode* aRootNode);
    void clearCaches(TimeLineEvent& aEvent);

private:
    static std::pair<TimeKey*, LayerMesh*> getAreaMeshImpl(ObjectNode& aNode, const TimeInfo& aTime);
    static MeshKey* getMeshKey(const ObjectNode& aNode, const TimeInfo& aTime);
    static ImageKey* getImageKey(const ObjectNode& aNode, const TimeInfo& aTime);

    static void getMoveExpans(SRTExpans& aExpans, const ObjectNode& aNode, const TimeInfo& aTime);
    static void getRotateExpans(SRTExpans& aExpans, const ObjectNode& aNode, const TimeInfo& aTime);
    static void getScaleExpans(SRTExpans& aExpans, const ObjectNode& aNode, const TimeInfo& aTime);

    void blendSRTKeys(PositionType aPos, const TimeInfo& aTime);
    void blendDepthKey(PositionType aPos, const TimeInfo& aTime);
    void blendOpaKey(PositionType aPos, const TimeInfo& aTime);
    void blendBoneKey(PositionType aPos, const TimeInfo& aTime);
    void blendPoseKey(PositionType aPos, const TimeInfo& aTime);
    void blendMeshKey(PositionType aPos, const TimeInfo& aTime);
    void blendFFDKey(PositionType aPos, const TimeInfo& aTime);
    void blendImageKey(PositionType aPos, const TimeInfo& aTime);
    //void buildPosePalette(ObjectNode& aNode, PosePalette::KeyPairs& aPairs);
    void buildPosePalette(ObjectNode& aNode, PosePalette::KeyPair aPair);
    void setBoneInfluenceMaps(ObjectNode& aNode, const BoneKey* aKey,
                              const TimeInfo& aTime);
    void setBinderBones(ObjectNode& aRootNode);
    void setBindingMatrices(ObjectNode& aNode, bool aAffectedByBinding,
                            bool aUnderOfBinding, QMatrix4x4 aBindingMtx);

    SeekerType* mSeeker;
    SeekerType::Position mRoot;
};

} // namespace core

#endif // CORE_TIMEKEYBLENDER_H
