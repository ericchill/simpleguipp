#ifndef _COM_STOCHASTIC_MOUSE_H_
#define _COM_STOCHASTIC_MOUSE_H_

#include "glutpp_internal.h"

namespace glutpp {

    class MouseButtonEventImpl : public MouseButtonEvent {
    private:
	int _type;
	int _state;
	int _button;
	Vector2D<int> _position;
	unsigned long _time;
    public:
	MouseButtonEventImpl(const x11::XEvent* event);
	~MouseButtonEventImpl() {}
	bool buttonDown() const;
	int button() const;
	bool shift() const;
	bool ctrl() const;
	bool alt() const;
	Vector2D<int> position() const;
	unsigned long time() const;
    };

    class MouseMotionEventImpl : public MouseMotionEvent {
	int _state;
	Vector2D<int> _position;
	unsigned long _time;
    public:
	MouseMotionEventImpl(const x11::XEvent* event);
	~MouseMotionEventImpl() {}
	int button() const;
	bool shift() const;
	bool ctrl() const;
	bool alt() const;
	Vector2D<int> position() const;
	unsigned long time() const;
    };

};

#endif // _COM_STOCHASTIC_MOUSE_H_
