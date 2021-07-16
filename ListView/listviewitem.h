#ifndef LISTVIEWITEM_H
#define LISTVIEWITEM_H

#include <QWidget>


class ListViewItem : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool hover READ hover WRITE setHover NOTIFY hoverChanged)
    Q_PROPERTY(bool pressed READ pressed WRITE setPressed NOTIFY pressedChanged)
    Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY(bool isLastItem READ isLastItem WRITE setIsLastItem NOTIFY isLastItemChanged)

public:
    Q_INVOKABLE ListViewItem(QWidget* parent);
    ~ListViewItem();

    bool hover() const;
    bool pressed() const;
    bool selected() const;
    bool isLastItem() const;

public slots:
    void setHover(bool hover);
    void setPressed(bool pressed);
    void setSelected(bool selected);
    void setIsLastItem(bool isLastRow);

signals:
    void hoverChanged(bool hover);
    void pressedChanged(bool pressed);
    void selectedChanged(bool selected);
    void isLastItemChanged(bool isLastRow);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    virtual void clickEvent(QMouseEvent *event, const QPoint& pressPos);

private:
    class ListViewItemPriv* priv;
};



#endif
