#include "keyboard.h"
#include <X11/keysym.h>

using namespace glutpp;

KeyboardEventImpl::KeyboardEventImpl(x11::XEvent* event)
    : _char(-1), _special(SpecialKey::NONE)
{
    WindowImpl* window = WindowImpl::forXWindow(event->xkey.window);
    _type = event->type;
    _state = event->xkey.state;
    // Key repeat
    x11::XComposeStatus composeStatus;
    char ascii[32];
    x11::KeySym keySym;
    if (x11::XLookupString(&event->xkey, ascii, sizeof(ascii), &keySym, &composeStatus) > 0) {
	_char = ascii[0];
    } else {
	switch (keySym) {
	case XK_KP_F1:
	case XK_F1:
	    _special = SpecialKey::F1;
	    break;
	case XK_KP_F2:
	case XK_F2:
	    _special = SpecialKey::F2;
	    break;
	case XK_KP_F3:
	case XK_F3:
	    _special = SpecialKey::F3;
	    break;
	case XK_KP_F4:
	case XK_F4:
	    _special = SpecialKey::F4;
	    break;
	case XK_F5:
	    _special = SpecialKey::F5;
	    break;
	case XK_F6:
	    _special = SpecialKey::F6;
	    break;
	case XK_F7:
	    _special = SpecialKey::F7;
	    break;
	case XK_F8:
	    _special = SpecialKey::F8;
	    break;
	case XK_F9:
	    _special = SpecialKey::F9;
	    break;
	case XK_F10:
	    _special = SpecialKey::F10;
	    break;
	case XK_F11:
	    _special = SpecialKey::F11;
	    break;
	case XK_F12:
	    _special = SpecialKey::F12;
	    break;
	case XK_KP_Left:
	case XK_Left:
	    _special = SpecialKey::LEFT;
	    break;
	case XK_KP_Right:
	case XK_Right:
	    _special = SpecialKey::RIGHT;
	    break;
	case XK_KP_Up:
	case XK_Up:
	    _special = SpecialKey::UP;
	    break;
	case XK_KP_Down:
	case XK_Down:
	    _special = SpecialKey::DOWN;
	    break;
	case XK_KP_Prior:
	case XK_Prior:
	    _special = SpecialKey::PAGE_UP;
	    break;
	case XK_KP_Next:
	case XK_Next:
	    _special = SpecialKey::PAGE_DOWN;
	    break;
	case XK_KP_Home:
	case XK_Home:
	    _special = SpecialKey::HOME;
	    break;
	case XK_KP_End:
	case XK_End:
	    _special = SpecialKey::END;
	    break;
	case XK_KP_Insert:
	case XK_Insert:
	    _special = SpecialKey::INSERT;
	    break;
	default:
	    // Otherwise flag and pass untranslated
	    _special = SpecialKey::UNKNOWN_BIT | (SpecialKey) keySym;
	}
    }
    _pointerPosition.x = event->xkey.x;
    _pointerPosition.y = event->xkey.y;
    _time = event->xkey.time;
}

bool KeyboardEventImpl::keyDown() const
{
    return _type == KeyPress;
}

bool KeyboardEventImpl::shift() const
{
    return _state & (ShiftMask | LockMask);
}

bool KeyboardEventImpl::ctrl() const
{
    return _state & ControlMask;
}

bool KeyboardEventImpl::alt() const
{
    return _state & Mod1Mask;
}

bool KeyboardEventImpl::isSpecial() const
{
    return _special != SpecialKey::NONE;
}

int KeyboardEventImpl::character() const
{
    return _char;
}

SpecialKey KeyboardEventImpl::special() const
{
    return _special;
}

Vector2D<int> KeyboardEventImpl::position() const
{
    return _pointerPosition;
}

unsigned long KeyboardEventImpl::time() const
{
    return _time;
}
