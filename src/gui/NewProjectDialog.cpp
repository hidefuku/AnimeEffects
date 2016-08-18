#include <QSpinBox>
#include <QGroupBox>
#include <QFormLayout>
#include "gui/NewProjectDialog.h"

namespace gui
{

NewProjectDialog::NewProjectDialog(const QString&, QWidget* aParent)
    : EasyDialog("New Project Dialog", aParent)
    , mAttribute()
{
    // initialize attribute
    {
        mAttribute.setMaxFrame(60 * 10);
    }

    this->setMainWidget(createOption());
    this->setOkCancel([=](int)->bool
    {
        return true;
    });
    this->fixSize();
}

QWidget* NewProjectDialog::createOption()
{
    auto form = new QFormLayout();
    form->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
    form->setLabelAlignment(Qt::AlignRight);

    // frame
    {
        auto frame = new QSpinBox();
        frame->setRange(1, std::numeric_limits<int>::max());
        frame->setValue(mAttribute.maxFrame());

        this->connect(frame, &QSpinBox::editingFinished, [=]()
        {
            this->mAttribute.setMaxFrame(frame->value());
        });

        form->addRow("frame :", frame);
    }

    auto group = new QGroupBox("Options");
    group->setLayout(form);

    return group;
}

} // namespace gui
