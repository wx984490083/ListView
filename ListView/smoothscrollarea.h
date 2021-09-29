#ifndef SMOOTHSCROLLAREA_H
#define SMOOTHSCROLLAREA_H

#include <QScrollArea>
#include <QTimer>

class SmoothScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    explicit SmoothScrollArea(QWidget *parent = nullptr);
    ~SmoothScrollArea();


protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    void superWheelEvent(QWheelEvent *event);
    void smoothWheelTimeout();

    QTimer _smoothWheelTimer;
    std::chrono::time_point<std::chrono::steady_clock> _wheelTime;
    std::chrono::time_point<std::chrono::steady_clock> _lastAnimationTime;
    double _wheelDurationMs;
    int _wheelDistance = 0;
    double _accelerationSpeedMs = 0;
    double inertiaSpeedMs = 0;
    Qt::MouseButtons wheelButtons;
    Qt::KeyboardModifiers wheelModifiers;
};

#endif // SMOOTHSCROLLAREA_H
