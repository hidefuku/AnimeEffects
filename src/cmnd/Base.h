#ifndef CMND_BASE_H
#define CMND_BASE_H

#include <QString>

namespace cmnd
{

class Base
{
public:
    Base() {}
    virtual ~Base() {}
    virtual QString name() const { return QString(); }
    virtual bool isUseless() const { return false; }
    virtual bool tryExec() { return true; }
    virtual bool tryRedo() { return true; }
    virtual bool tryUndo() { return true; }

private:
    Base(const Base&);
    Base& operator=(const Base&);
};

} // namespace cmnd

#endif // CMND_BASE_H
