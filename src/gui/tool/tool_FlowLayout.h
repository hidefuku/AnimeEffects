#ifndef GUI_TOOL_FLOWLAYOUT_H
#define GUI_TOOL_FLOWLAYOUT_H

#include <QLayout>
#include <QRect>
#include <QWidgetItem>
#include <QStyle>

namespace gui {
namespace tool {

class FlowLayout : public QLayout
{
public:
    FlowLayout(QWidget* aParent, int aMargin = -1,
               int aHSpacing = -1, int aVSpacing = -1);
    FlowLayout(int aMargin = -1, int aHSpacing = -1, int aVSpacing = -1);
    ~FlowLayout();

    void addItem(QLayoutItem* aItem);
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const;
    bool hasHeightForWidth() const;
    int heightForWidth(int) const;
    int count() const;
    QLayoutItem* itemAt(int aIndex) const;
    QSize minimumSize() const;
    void setGeometry(const QRect& aRect);
    QSize sizeHint() const;
    QLayoutItem* takeAt(int aIndex);

private:
    int doLayout(const QRect& aRect, bool aTestOnly) const;
    int smartSpacing(QStyle::PixelMetric aMetric) const;

    QList<QLayoutItem*> mItemList;
    int mHSpace;
    int mVSpace;
};

} // namespace tool
} // namespace gui

#endif // GUI_TOOL_FLOWLAYOUT_H
