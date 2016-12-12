#include <QSpinBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QCheckBox>
#include "gui/NewProjectDialog.h"

namespace gui
{

NewProjectDialog::NewProjectDialog(QWidget* aParent)
    : EasyDialog(tr("New Project Dialog"), aParent)
    , mFileName()
    , mAttribute()
    , mSpecifiesCanvasSize()
{
    // initialize attribute
    {
        mAttribute.setMaxFrame(60 * 10);
        mAttribute.setImageSize(QSize(512, 512));
    }

    this->setMainWidget(createOption(), false);
    this->setOkCancel([=](int)->bool { return true; });
    //this->fixSize();
}

QWidget* NewProjectDialog::createOption()
{
    auto form = new QFormLayout();
    form->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
    form->setLabelAlignment(Qt::AlignRight);

    // resource
    {
        auto layout = new QHBoxLayout();

        auto line = new QLineEdit(this);
        //line->setEnabled(false);
        line->setReadOnly(true);
        line->setContextMenuPolicy(Qt::NoContextMenu);
        line->setFocusPolicy(Qt::NoFocus);
        layout->addWidget(line);

        auto button = new QPushButton(this);
        this->connect(button, &QPushButton::clicked, [=]()
        {
            this->mFileName = QFileDialog::getOpenFileName(
                        this, tr("Open File"), "", "ImageFile (*.psd *.jpg *.jpeg *.png *.gif)");

            line->setText(mFileName);
        });
        button->setObjectName("browser");
        button->setFocusPolicy(Qt::NoFocus);
        layout->addWidget(button);

        form->addRow(tr("initial resource :"), layout);
    }

    // frame
    {
        auto frame = new QSpinBox();
        frame->setRange(1, std::numeric_limits<int>::max());
        frame->setValue(mAttribute.maxFrame());

        this->connect(frame, &QSpinBox::editingFinished, [=]()
        {
            this->mAttribute.setMaxFrame(frame->value());
        });

        form->addRow(tr("max frame :"), frame);
    }

    // canvas size
    {
        auto width = new QSpinBox();
        width->setEnabled(false);
        width->setRange(1, std::numeric_limits<int>::max());
        width->setValue(mAttribute.imageSize().width());
        this->connect(width, &QSpinBox::editingFinished, [=](){
            auto size = this->mAttribute.imageSize();
            size.setWidth(width->value());
            this->mAttribute.setImageSize(size);
        });

        auto height = new QSpinBox();
        height->setEnabled(false);
        height->setRange(1, std::numeric_limits<int>::max());
        height->setValue(mAttribute.imageSize().height());
        this->connect(height, &QSpinBox::editingFinished, [=](){
            auto size = this->mAttribute.imageSize();
            size.setHeight(height->value());
            this->mAttribute.setImageSize(size);
        });

        auto check = new QCheckBox();
        this->connect(check, &QCheckBox::clicked, [=](bool aChecked)
        {
            this->mSpecifiesCanvasSize = aChecked;
            width->setEnabled(aChecked);
            height->setEnabled(aChecked);
        });

        form->addRow(tr("specify canvas size :"), check);
        form->addRow(tr("canvas width :"), width);
        form->addRow(tr("canvas height :"), height);
    }

    auto group = new QGroupBox(tr("Parameters"));
    group->setLayout(form);

    return group;
}

} // namespace gui
