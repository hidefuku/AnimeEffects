#ifndef CTRL_BONEEDITMODE
#define CTRL_BONEEDITMODE

namespace ctrl
{

enum BoneEditMode
{
    BoneEditMode_Create,
    BoneEditMode_Delete,
    BoneEditMode_MoveJoint,
    BoneEditMode_BindNodes,
    BoneEditMode_Influence,
    BoneEditMode_PaintInfl,
    BoneEditMode_EraseInfl,
    BoneEditMode_TERM
};

} // namespace ctrl

#endif // CTRL_BONEEDITMODE

