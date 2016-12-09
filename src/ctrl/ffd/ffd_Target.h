#ifndef CTRL_FFD_TARGET_H
#define CTRL_FFD_TARGET_H

#include <QScopedPointer>
#include "ctrl/ffd/ffd_KeyOwner.h"
#include "ctrl/ffd/ffd_Task.h"

namespace ctrl {
namespace ffd {

class Target
{
public:
    Target();
    Target(core::ObjectNode* aNode);
    ~Target();
    bool isValid() const;
    core::ObjectNode* node;
    ffd::KeyOwner keyOwner;
    QScopedPointer<ffd::Task> task;
};

class Targets : public QVector<Target*>
{
public:
    bool hasValidTarget() const;
};

} // namespace ffd
} // namespace ctrl

#endif // CTRL_FFD_TARGET_H
