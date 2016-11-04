#include "mouse.h"

using namespace glutpp;

// MouseButtonEventImpl

MouseButtonEventImpl::MouseButtonEventImpl(const x11::XEvent* event)
{
    _type = event->type;
    _state = event->xbutton.state;
    _button = event->xbutton.button;
    _position.x = event->xbutton.x;
    _position.y = event->xbutton.y;
    _time = event->xbutton.time;
}

bool MouseButtonEventImpl::buttonDown() const
{
    return _type == ButtonPress;
}

int MouseButtonEventImpl::button() const
{
    return _button;
}

bool MouseButtonEventImpl::shift() const
{
    return _state & (ShiftMask | LockMask);
}

bool MouseButtonEventImpl::ctrl() const
{
    return _state & ControlMask;
}

bool MouseButtonEventImpl::alt() const
{
    return _state & Mod1Mask;
}

Vector2D<int> MouseButtonEventImpl::position() const
{
    return _position;
}

unsigned long MouseButtonEventImpl::time() const
{
    return _time;
}

/* ===== MouseMotionEventImpl ===== */

MouseMotionEventImpl::MouseMotionEventImpl(const x11::XEvent* event)
{
    _state = event->xmotion.state;
    _position.x = event->xbutton.x;
    _position.y = event->xbutton.y;
    _time = event->xbutton.time;
}

int MouseMotionEventImpl::button() const
{
    if (_state & Button1Mask) {
	return 1;
    } else if (_state & Button2Mask) {
	return 2;
    } else if (_state & Button3Mask) {
	return 3;
    } else if (_state & Button4Mask) {
	return 4;
    } else if (_state & Button5Mask) {
	return 5;
    } else {
	return 0;
    }
}

bool MouseMotionEventImpl::shift() const
{
    return _state & (ShiftMask | LockMask);
}

bool MouseMotionEventImpl::ctrl() const
{
    return _state & ControlMask;
}

bool MouseMotionEventImpl::alt() const
{
    return _state & Mod1Mask;
}

Vector2D<int> MouseMotionEventImpl::position() const
{
    return _position;
}

unsigned long MouseMotionEventImpl::time() const
{
    return _time;
}
