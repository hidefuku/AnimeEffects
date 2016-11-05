#include "ctrl/Driver.h"

namespace
{

struct ScopeCounter
{
    int& count;
    ScopeCounter(int& aCount) : count(aCount) { ++count; }
    ~ScopeCounter() { --count; }
};

}

namespace ctrl
{

Driver::Driver(core::Project& aProject, DriverResources& aResources, GraphicStyle& aGraphicStyle)
    : mProject(aProject)
    , mResources(aResources)
    , mGraphicStyle(aGraphicStyle)
    , mToolType(ToolType_TERM)
    , mBlender(aProject.objectTree())
    , mEditor()
    , mCurrentNode()
    , mOnUpdating(0)
{
    // initialize blending
    mBlender.updateCurrents(
                mProject.objectTree().topNode(),
                mProject.currentTimeInfo());
}

void Driver::setTarget(core::ObjectNode* aNode)
{
    ScopeCounter counter(mOnUpdating);
    mCurrentNode = aNode;

    if (mEditor)
    {
        mEditor->setTarget(mCurrentNode);
    }
}

void Driver::setTool(ToolType aType)
{
    ScopeCounter counter(mOnUpdating);
    mToolType = aType;
    mEditor.reset();

    if (mToolType == ToolType_Cursor)
    {
    }
    else if (mToolType == ToolType_SRT)
    {
        mEditor.reset(new SRTEditor(mProject));
    }
    else if (mToolType == ToolType_Bone)
    {
        mEditor.reset(new BoneEditor(mProject, mGraphicStyle));
    }
    else if (mToolType == ToolType_Pose)
    {
        mEditor.reset(new PoseEditor(mProject));
    }
    else if (mToolType == ToolType_Mesh)
    {
        mEditor.reset(new MeshEditor(mProject));
    }
    else if (mToolType == ToolType_FFD)
    {
        mEditor.reset(new FFDEditor(mProject, mResources));
    }

    setTarget(mCurrentNode);
}

bool Driver::updateCursor(const core::AbstractCursor& aCursor, const core::PenInfo& aPenInfo, const core::CameraInfo& aCamera)
{
    ScopeCounter counter(mOnUpdating);
    (void)aPenInfo;

    // stop animation
    if (aCursor.emitsPressedEvent())
    {
        mProject.animator().stop();
    }

    if (mEditor)
    {
        return mEditor->updateCursor(aCamera, aCursor);
    }

    return false;
}

void Driver::updateFrame()
{
    ScopeCounter counter(mOnUpdating);

    // update blending
    mBlender.updateCurrents(
                mProject.objectTree().topNode(),
                mProject.currentTimeInfo());

    if (mEditor)
    {
        mEditor->updateEvent(IEditor::EventType_Frame);
    }
}

void Driver::updateKey(core::TimeLineEvent& aEvent, bool aUndo)
{
#if 0
    qDebug() << "type: " << aEvent.type() << (aUndo ? " undo" : " redo");
    for (const TimeLineEvent::Target& target : aEvent.targets())
    {
        qDebug() << target.pos.index() << target.subIndex;
    }
#endif
    (void)aUndo;

    // reset blending
    mBlender.clearCaches(aEvent);
    mBlender.updateCurrents(
                mProject.objectTree().topNode(),
                mProject.currentTimeInfo());

    if (mOnUpdating > 0) return;

    if (mEditor)
    {
        mEditor->updateEvent(IEditor::EventType_TimeKey);
    }
}

void Driver::updateTree(core::ObjectTreeEvent& aEvent, bool aUndo)
{
    (void)aUndo;
    ScopeCounter counter(mOnUpdating);

    // reset blending
    for (auto root : aEvent.roots())
    {
        mBlender.clearCaches(root);
    }

    mBlender.updateCurrents(
                mProject.objectTree().topNode(),
                mProject.currentTimeInfo());

    if (mEditor)
    {
        mEditor->updateEvent(IEditor::EventType_Tree);
    }
}

void Driver::updateResource(core::ResourceEvent& aEvent, bool aUndo)
{
    (void)aEvent;
    (void)aUndo;
    ScopeCounter counter(mOnUpdating);

    // reset blending
    mBlender.clearCaches(mProject.objectTree().topNode());
    mBlender.updateCurrents(
                mProject.objectTree().topNode(),
                mProject.currentTimeInfo());

    if (mEditor)
    {
        mEditor->updateEvent(IEditor::EventType_Resource);
    }
}

void Driver::updateProjectAttribute()
{
    ScopeCounter counter(mOnUpdating);

    // reset blending
    mBlender.clearCaches(mProject.objectTree().topNode());
    mBlender.updateCurrents(
                mProject.objectTree().topNode(),
                mProject.currentTimeInfo());

    if (mEditor)
    {
        mEditor->updateEvent(IEditor::EventType_ProjectAttribute);
    }
}

void Driver::renderGL(
        const core::RenderInfo& aRenderInfo, core::ObjectNode* aGridTarget)
{
    auto& tree = mProject.objectTree();
    auto info = aRenderInfo;
    if (mToolType == ToolType_Bone)
    {
        info.nonPosed = true;
    }
    else if (mToolType == ToolType_Mesh)
    {
        info.originMesh = true;
    }

    tree.render(info, false);

    if (aGridTarget && aGridTarget->renderer())
    {
        info.isGrid = true;
        core::TimeCacheAccessor accessor(
                    *aGridTarget, tree.timeCacheLock(), info.time, false);
        aGridTarget->renderer()->render(info, accessor);
    }
}

void Driver::renderQt(const core::RenderInfo& aRenderInfo, QPainter& aPainter)
{
    auto info = aRenderInfo;
    if (mToolType == ToolType_Bone)
    {
        info.nonPosed = true;
    }

    if (mEditor)
    {
        mEditor->renderQt(info, aPainter);
    }

    // draw outline
    drawOutline(info, aPainter);
}

void Driver::drawOutline(const core::RenderInfo& aRenderInfo, QPainter& aPainter)
{
    aPainter.setBrush(Qt::NoBrush);
    {
        QPen pen(QBrush(QColor(255, 255, 255, 128)), 1.0f, Qt::CustomDashLine);
        QVector<qreal> dashes;
        dashes << 4 << 4;
        pen.setDashPattern(dashes);
        aPainter.setPen(pen);
    }
    aPainter.setRenderHint(QPainter::Antialiasing);

    auto quad = aRenderInfo.camera.screenImageQuadangle();

    aPainter.drawLine(quad[0].toPointF(), quad[1].toPointF());
    aPainter.drawLine(quad[1].toPointF(), quad[2].toPointF());
    aPainter.drawLine(quad[2].toPointF(), quad[3].toPointF());
    aPainter.drawLine(quad[3].toPointF(), quad[0].toPointF());
}

void Driver::updateParam(const SRTParam& aParam)
{
    XC_ASSERT(mToolType == ToolType_SRT);
    if (mToolType != ToolType_SRT) return;

    ((SRTEditor*)mEditor.data())->updateParam(aParam);
}

void Driver::updateParam(const FFDParam& aParam)
{
    XC_ASSERT(mToolType == ToolType_FFD);
    if (mToolType != ToolType_FFD) return;

    ((FFDEditor*)mEditor.data())->updateParam(aParam);
}

void Driver::updateParam(const BoneParam& aParam)
{
    XC_ASSERT(mToolType == ToolType_Bone);
    if (mToolType != ToolType_Bone) return;

    ((BoneEditor*)mEditor.data())->updateParam(aParam);
}

void Driver::updateParam(const MeshParam& aParam)
{
    XC_ASSERT(mToolType == ToolType_Mesh);
    if (mToolType != ToolType_Mesh) return;

    ((MeshEditor*)mEditor.data())->updateParam(aParam);
}

} // namespace core
