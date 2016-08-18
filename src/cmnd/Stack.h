#ifndef CMND_STACK_H
#define CMND_STACK_H

#include <QUndoStack>
#include <QVector>
#include <QList>
#include <QMutableListIterator>
#include <QListIterator>
#include "util/LifeLink.h"
#include "cmnd/Base.h"
#include "cmnd/Listener.h"

namespace cmnd
{

class ScopedMacro;
class ScopedUndoSuspender;

class Stack
{
public:
    Stack();
    ~Stack();

    bool isSuspended() const { return mSuspendCount > 0; }
    void push(Base* aCommand);
    void push(const std::vector<Base*>&& aCommands);
    QString undo();
    QString redo();
    void clear();

    bool isModifiable(const Base* aBase) const;

private:
    class Macro : public Base
    {
    public:
        Macro(const QString& aName);
        ~Macro();
        void push(Base* aCommand);
        void setValidLink(util::LifeLink& aLink);
        void grabListener(Listener* aListener);
        virtual QString name() const;
        virtual bool tryExec();
        virtual bool tryRedo();
        virtual bool tryUndo();
        virtual bool isUseless() const;
    private:
        void killListeners();
        QList<Base*> mCommands;
        bool mHasValidLink;
        util::LifeLink::Node mValidLink;
        QVector<Listener*> mListeners;
        QString mName;
    };

    friend class ScopedMacro;
    void beginMacro(const QString& aText);
    void setMacroValidLink(util::LifeLink& aLifeLink);
    void grabMacroListener(Listener* aListener);
    void endMacro();

    friend class ScopedUndoSuspender;
    void suspendUndo() { ++mSuspendCount; }
    void resumeUndo() { --mSuspendCount; }

    void pushImpl(Base* aCommand);

    const int mLimit;
    QList<Base*> mCommands;
    QList<Base*>::Iterator mCurrent;
    Macro* mMacro;
    int mSuspendCount;
    Base* mModifiable;
};

} // namespace cmnd

#endif // CMND_STACK_H
