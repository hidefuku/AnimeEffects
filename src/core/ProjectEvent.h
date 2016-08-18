#ifndef CORE_PROJECTEVENT_H
#define CORE_PROJECTEVENT_H

namespace core { class Project; }

namespace core
{

class ProjectEvent
{
public:
    enum Type
    {
        Type_ChangeImageSize,
        Type_ChangeMaxFrame,
        Type_ChangeLoop,
        Type_TERM
    };

    static ProjectEvent imageSizeChangeEvent(Project& aProject)
    {
        return ProjectEvent(aProject, Type_ChangeImageSize);
    }

    static ProjectEvent maxFrameChangeEvent(Project& aProject)
    {
        return ProjectEvent(aProject, Type_ChangeMaxFrame);
    }

    static ProjectEvent loopChangeEvent(Project& aProject)
    {
        return ProjectEvent(aProject, Type_ChangeLoop);
    }

    ProjectEvent(Project& aProject, Type aType);

    Project& project() const { return mProject; }
    Type type() const { return mType; }

private:
    Project& mProject;
    Type mType;
};

} // namespace core

#endif // CORE_PROJECTEVENT_H
