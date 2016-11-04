#ifndef _COM_STOCHASTIC_GLUTPP_KEYBOARD_H_
#define _COM_STOCHASTIC_GLUTPP_KEYBOARD_H_ 1

#include "glutpp_internal.h"

namespace glutpp {

    class KeyboardEventImpl : public KeyboardEvent {
    private:
	int _type;
	int _state;
	int _char;
	SpecialKey _special;
	Vector2D<int> _pointerPosition;
	unsigned long _time;
    public:
	KeyboardEventImpl(x11::XEvent* event);
	bool keyDown() const;
	bool shift() const;
	bool ctrl() const;
	bool alt() const;
	bool isSpecial() const;
	int character() const;
	SpecialKey special() const;
	Vector2D<int> position() const;
	unsigned long time() const;
    };

};

#endif // _COM_STOCHASTIC_GLUTPP_KEYBOARD_H_
