#ifndef CMND_SCALABLE_H
#define CMND_SCALABLE_H

#include <functional>
#include <QVector>
#include "cmnd/Base.h"
#include "cmnd/Vector.h"
#include "cmnd/Listener.h"

namespace cmnd
{

class Scalable : public cmnd::Base
{
public:
    Scalable();
    virtual ~Scalable();

    virtual void initialize() {}
    virtual bool initializeAndExecute();
    void grabListener(Listener* aListener);
    Vector& commands() { return mCommands; }
    const Vector& commands() const { return mCommands; }

private:
    virtual bool isUseless() const;
    virtual bool tryExec();
    virtual bool tryRedo();
    virtual bool tryUndo();

    Vector mCommands;
    QVector<cmnd::Listener*> mListeners;
    bool mExecuted;
};

class LambdaScalable : public cmnd::Scalable
{
    typedef std::function<void(Vector&)> InitializerType;
    InitializerType mInitializer;

    virtual void initialize() { mInitializer(commands()); }

public:
    LambdaScalable(const InitializerType& aInitializer)
        : mInitializer(aInitializer) {}
};

} // namespace cmnd

#endif // CMND_SCALABLE_H
