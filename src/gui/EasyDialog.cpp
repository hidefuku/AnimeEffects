#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include "XC.h"
#include "gui/EasyDialog.h"

namespace gui
{

EasyDialog::EasyDialog(const QString& aTitle, QWidget* aParent, bool aIsModal)
    : QDialog(aParent)
    , mLayout()
    , mOk()
{
    this->setWindowTitle(aTitle);
    this->setModal(aIsModal);

    // top layout
    mLayout = new QVBoxLayout(this);
    this->setLayout(mLayout);
}

void EasyDialog::setMenuBar(QMenuBar* aMenuBar)
{
    mLayout->setMenuBar(aMenuBar);
}

void EasyDialog::setMainWidget(QWidget* aWidget, bool aAlignLeft)
{
    mLayout->addWidget(aWidget);
    if (aAlignLeft)
    {
        mLayout->setAlignment(aWidget, Qt::AlignLeft | Qt::AlignTop);
    }
    mLayout->addSpacing(16);
}

void EasyDialog::setOkCancel(const std::function<bool(int)>& aFunction)
{
    auto buttonBox = new QHBoxLayout();
    auto ok = new QPushButton("Ok");
    auto cc = new QPushButton("Cancel");

    ok->setObjectName("standardButton");
    cc->setObjectName("standardButton");
    ok->setDefault(false);
    cc->setDefault(false);
    ok->setAutoDefault(false);
    cc->setAutoDefault(false);

    connect(ok, &QPushButton::clicked, [=](bool)
    {
        if (aFunction(0))
        {
            this->accept();
        }
    });
    connect(cc, &QPushButton::clicked, [=](bool)
    {
        if (aFunction(1))
        {
            this->reject();
        }
    });

    buttonBox->addWidget(ok);
    buttonBox->addWidget(cc);

    mOk = ok;
    mLayout->addLayout(buttonBox);
    mLayout->setAlignment(buttonBox, Qt::AlignBottom | Qt::AlignRight);
}

void EasyDialog::setOkEnable(bool aIsEnable)
{
    XC_PTR_ASSERT(mOk);
    mOk->setEnabled(aIsEnable);
}

void EasyDialog::fixSize()
{
    this->setFixedSize(this->sizeHint());
    this->layout()->setSizeConstraint( QLayout::SetFixedSize );
}

} // namespace gui

