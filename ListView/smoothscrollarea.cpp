#include "smoothscrollarea.h"
#include <QWheelEvent>

static const double DefaultWheelSpeedMs = 1.0;
static const double MaxWheelSpeedMs = 10.0;
static const double DefaultAccelerationSpeedMs = 0.012;


SmoothScrollArea::SmoothScrollArea(QWidget *parent) :
    QScrollArea(parent)
{
    _smoothWheelTimer.callOnTimeout(this, [=] {smoothWheelTimeout();});
}

SmoothScrollArea::~SmoothScrollArea()
{
}


void SmoothScrollArea::wheelEvent(QWheelEvent *event)
{
    auto now = std::chrono::steady_clock::now();
    auto durationFromPrevWheel = std::chrono::duration_cast<std::chrono::milliseconds>(now - _wheelTime).count();
    _wheelTime = now;
    auto delta = event->angleDelta().y();
    if ((durationFromPrevWheel < 250 &&
         ((delta > 0 && _wheelDistance > 0) ||
          (delta < 0 && _wheelDistance < 0))))
    {
        _wheelDurationMs += durationFromPrevWheel;
        _wheelDistance += delta;
    }
    else
    {
        _wheelDistance = delta;
        _wheelDurationMs = abs(delta) / DefaultWheelSpeedMs;
    }
    inertiaSpeedMs = _wheelDistance * 2.0 / _wheelDurationMs;
    if (abs(inertiaSpeedMs) > MaxWheelSpeedMs)
    {
        inertiaSpeedMs = _wheelDistance > 0 ? MaxWheelSpeedMs : -MaxWheelSpeedMs;
    }
    if (inertiaSpeedMs)
    {
        _accelerationSpeedMs = _wheelDistance > 0 ? -DefaultAccelerationSpeedMs : DefaultAccelerationSpeedMs;
        _smoothWheelTimer.start(10);
        _lastAnimationTime = now;
    }
    wheelButtons = event->buttons();
    wheelModifiers = event->modifiers();
}

void SmoothScrollArea::superWheelEvent(QWheelEvent *event)
{
    QScrollArea::wheelEvent(event);
}

void SmoothScrollArea::smoothWheelTimeout()
{
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastAnimationTime).count();
    _lastAnimationTime = now;
    auto deltaSpeed = _accelerationSpeedMs * duration;
    if (abs(deltaSpeed) > abs(inertiaSpeedMs))
    {
        deltaSpeed = -inertiaSpeedMs;
        duration = deltaSpeed / _accelerationSpeedMs;
    }
    auto deltaDistance = (inertiaSpeedMs + inertiaSpeedMs + deltaSpeed) / 2 * duration;

    inertiaSpeedMs += deltaSpeed;

    QWheelEvent fakeEvent(QPointF(0,0), deltaDistance, wheelButtons, wheelModifiers);
    superWheelEvent(&fakeEvent);

    if (!inertiaSpeedMs)
    {
        _smoothWheelTimer.stop();
    }
}

