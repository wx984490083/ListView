#include "listviewitem_p.h"
#include "listview_p.h"
#include <QMouseEvent>
#include <QPainter>


ListViewItem::ListViewItem(QWidget *parent) : QWidget(parent), priv(new ListViewItemPriv)
{

}

ListViewItem::~ListViewItem()
{
    delete priv;
}

bool ListViewItem::hover() const
{
    return priv->hover;
}

bool ListViewItem::pressed() const
{
    return priv->pressed;
}

bool ListViewItem::selected() const
{
    return priv->selected;
}

bool ListViewItem::isLastItem() const
{
    return priv->isLastItem;
}

void ListViewItem::setHover(bool hover)
{
    if (priv->hover == hover)
        return;

    priv->hover = hover;
    emit hoverChanged(priv->hover);
    update();
}

void ListViewItem::setPressed(bool pressed)
{
    if (priv->pressed == pressed)
        return;

    priv->pressed = pressed;
    emit pressedChanged(priv->pressed);
    update();
}

void ListViewItem::setSelected(bool selected)
{
    if (priv->selected == selected)
        return;

    priv->selected = selected;
    emit selectedChanged(priv->selected);
    update();
}

void ListViewItem::setIsLastItem(bool isLastRow)
{
    if (priv->isLastItem == isLastRow)
        return;

    priv->isLastItem = isLastRow;
    emit isLastItemChanged(priv->isLastItem);
}


void ListViewItem::mousePressEvent(QMouseEvent *event)
{
    priv->pressPos = event->pos();
    priv->pressedButton |= event->button();
    if (event->button() == Qt::LeftButton)
        setPressed(true);
}

void ListViewItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        setPressed(false);

    auto didPressedTheButton = priv->pressedButton.testFlag(event->button());
    if (didPressedTheButton && rect().contains(event->pos()))
        clickEvent(event, priv->pressPos);
}

void ListViewItem::enterEvent(QEvent *)
{
    setMouseTracking(true);
    setHover(true);
}

void ListViewItem::leaveEvent(QEvent *)
{
    setMouseTracking(false);
    setHover(false);
}

void ListViewItem::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    auto bg = priv->hover ? QColor("#10808080") : QColor("#00000000");
    p.fillRect(rect(), bg);

    QString text = priv->index.isHeader() ? QString::asprintf("GroupHeader %d", priv->index.group) : QString::asprintf("Row %d.%d", priv->index.group, priv->index.item);
    auto textWidth = p.fontMetrics().width(text);
    p.drawText((width() - textWidth) / 2, (height() - p.fontMetrics().height()) / 2 + p.fontMetrics().ascent(), text);
}

void ListViewItem::clickEvent(QMouseEvent *e, const QPoint &)
{
    if (priv->listView)
        priv->listView->processItemClick(e, priv->index, priv);
}
