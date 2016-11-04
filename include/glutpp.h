#ifndef __COM_STOCHASTIC_GLUTPLUSPLUS_H__
#define __COM_STOCHASTIC_GLUTPLUSPLUS_H__ 1

/*
 *
 * glutpp.h
 *
 * A C++ re-abstraction of the freeglut library.
 *
 * Each Window object has a GL context.
 */

#include <exception>
#include <memory>

namespace glutpp {

    class Screen;
    class Window;
    class Controller;

    template<typename T>
    struct Vector2D {
	Vector2D() {}
	Vector2D(T _x, T _y) {
	    x = _x;
	    y = _y;
	}
	T x;
	T y;
    };

    /**
     * Type safe boolean operator for enumerated bit values.
     */
    template<typename intType>
    intType operator~(intType a) {
	return (intType) ~(int)a;
    }

    /**
     * Type safe boolean operator for enumerated bit values.
     */
    template<typename intType>
    intType operator&(intType a, intType b) {
	return (intType) (((int)a) & ((int)b));
    }

    /**
     * Type safe boolean operator for enumerated bit values.
     */
    template<typename intType>
    intType operator|(intType a, intType b) {
	return (intType) (((int)a) | ((int)b));
    }

    /**
     * Type safe boolean operator for enumerated bit values.
     */
    template<typename intType>
    intType operator^(intType a, intType b) {
	return (intType) (((int)a) ^ ((int)b));
    }

    /**
     * Type safe boolean operator for enumerated bit values.
     */
    template<typename intType>
    bool any(intType a) {
	return a != intType::NONE;
    }

    enum class DisplayMode : unsigned {
	NONE        = 0,     // Mnemonic for root window
	RGB         = 0x0000,
	RGBA        = 0x0000,
	SINGLE      = 0x0000,
	DOUBLE      = 0x0002,
	ACCUM       = 0x0004,
	ALPHA       = 0x0008,
	DEPTH       = 0x0010,
	STENCIL     = 0x0020,
	MULTISAMPLE = 0x0080,
	STEREO      = 0x0100,
	LUMINANCE   = 0x0200
    };

    enum class WindowVisibility {
	HIDDEN,
	FULLY_RETAINED,
	PARTIALLY_RETAINED,
	FULLY_COVERED
    };

    enum class SpecialKey : unsigned {
	NONE      = 0,
	F1        = 0x0001,
	F2        = 0x0002,
	F3        = 0x0003,
	F4        = 0x0004,
	F5        = 0x0005,
	F6        = 0x0006,
	F7        = 0x0007,
	F8        = 0x0008,
	F9        = 0x0009,
	F10       = 0x000A,
	F11       = 0x000B,
	F12       = 0x000C,
	LEFT      = 0x0064,
	UP        = 0x0065,
	RIGHT     = 0x0066,
	DOWN      = 0x0067,
	PAGE_UP   = 0x0068,
	PAGE_DOWN = 0x0069,
	HOME      = 0x006A,
	END       = 0x006B,
	INSERT    = 0x006C,
	UNKNOWN_BIT = 0x10000
    };

    template<typename T>
    class Iterator {
    public:
	virtual bool hasMore() = 0;
	virtual T next() = 0;
    };

    /**
     * Interface classes for Display, Screen & Window apply to a handle class and an implementation class each.
     */

    class Display;
    class Screen;
    class Window;

    class DisplayInterface {
    public:
	virtual ~DisplayInterface() {}
	virtual void close() = 0;
	virtual Screen defaultScreen() = 0;
	virtual void setTimer(unsigned int time, int value) = 0;

	/*
	 * Process events until primary window is closed or other exit event occurs.
	virtual void mainLoop() = 0;
	/*
	 * Returns false if exit requested.
	 */
	virtual bool mainLoopEvent() = 0;
	/*
	 * Waits until an event occurs, a timer goes off, or a time delay.
	 */
	virtual bool pause(int millisecs) = 0;
    };

    class ScreenInterface {
    public:
	virtual ~ScreenInterface() {}
	virtual Display display() = 0;
	virtual Window rootWindow() = 0;
	virtual Window createWindow(const char* title, Vector2D<int>* position=nullptr, Vector2D<int>* size=nullptr,
				    DisplayMode mode = DisplayMode::RGB | DisplayMode::SINGLE | DisplayMode::DEPTH) = 0;

	virtual Vector2D<int> size() = 0;
	virtual Vector2D<int> sizeMM() = 0;
    };

    class WindowInterface {
    public:
	virtual ~WindowInterface() {}

	void enterContext();
	virtual void registerController(Controller* controller) = 0;
	virtual Controller* controller() = 0;
	virtual Screen screen() = 0;
	virtual Window parent() = 0;
	virtual Iterator<Window>* children() = 0;
	virtual DisplayMode fbMode() = 0;
	virtual Window createSubwindow(Vector2D<int>* position=nullptr, Vector2D<int>* size=nullptr) = 0;
	virtual void setTitle(const char* title) = 0;
	virtual void setIconTitle(const char* title) = 0;
	virtual void position(const Vector2D<int>& position) = 0;
	virtual Vector2D<int> position() = 0;
	virtual void size(const Vector2D<int>& size) = 0;
	virtual Vector2D<int> size() = 0;

	virtual void show() = 0;
	virtual void hide() = 0;
	virtual void iconify() = 0;
	virtual void toFront() = 0;
	virtual void toBack() = 0;
	virtual void close() = 0;

	virtual void swap() = 0;
    };


    /**
     * Handle classes for Display, Screen & Window
     */
    class Display : public DisplayInterface {
    public:
	static Display open(const char* name, int argc, char *argv[]);

	~Display();
	void close();
	Screen defaultScreen();
	void setTimer(unsigned int time, int value);

	void mainLoop();
	/*
	 * Returns false if exit requested.
	 */
	bool mainLoopEvent();
	bool pause(int millisecs);

	/* Basic operators */
	Display(const Display& other) : _impl(other._impl) {}
	Display& operator=(const Display& other) { _impl = other._impl; }

    protected:
	Display(class DisplayImpl* impl) : _impl(impl) {}
    private:
	class DisplayImpl* _impl;

	friend class DisplayImpl;
	friend class ScreenImpl;
	friend class WindowImpl;
    };

    class Screen : public ScreenInterface {
    public:
	~Screen();
	Display display();
	Window rootWindow();
	Window createWindow(const char* title, Vector2D<int>* position=nullptr, Vector2D<int>* size=nullptr,
			    DisplayMode mode = DisplayMode::RGB | DisplayMode::SINGLE | DisplayMode::DEPTH);
	Vector2D<int> size();
	Vector2D<int> sizeMM();

	/* Basic operators */
	Screen(const Screen& other) : _impl(other._impl) {}
	Screen& operator=(const Screen& other) { _impl = other._impl; }
	bool operator==(const Screen& other) { return _impl == other._impl; }

    protected:
	Screen(class ScreenImpl* impl) : _impl(impl) {}
    private:
	class ScreenImpl* _impl;

	friend class DisplayImpl;
	friend class ScreenImpl;
	friend class WindowImpl;
    };

    class Window : public WindowInterface {
    public:
	/*
	 * Window constructor is public for templated Iterator classes.
	 */
	Window(class WindowImpl* impl) : _impl(impl) {}
	~Window();
	void enterContext();
	void registerController(Controller* controller);
	Controller* controller();
	Screen screen();
	Window parent();
	Iterator<Window>* children();
	DisplayMode fbMode();
	Window createSubwindow(Vector2D<int>* position=nullptr, Vector2D<int>* size=nullptr);
	void setTitle(const char* title);
	void setIconTitle(const char* title);
	void position(const Vector2D<int>& position);
	Vector2D<int> position();
	void size(const Vector2D<int>& size);
	Vector2D<int> size();

	void show();
	void hide();
	void iconify();
	void toFront();
	void toBack();
	void close();

	void swap();

	/* Basic operators */
	Window(const Window& other) : _impl(other._impl) {}
	Window& operator=(const Window& other) { _impl = other._impl; }
	bool operator==(const Window& other) { return _impl == other._impl; }

    private:
	class WindowImpl* _impl;

	friend class DisplayImpl;
	friend class ScreenImpl;
	friend class WindowImpl;
	friend class PushContext;
    };

    class InputEvent {
    public:
	virtual ~InputEvent() {}
	virtual bool shift() const = 0;
	virtual bool ctrl() const = 0;
	virtual bool alt() const = 0;
	virtual Vector2D<int> position() const = 0;
	virtual unsigned long time() const = 0;
    };

    class MouseButtonEvent : public InputEvent {
    public:
	~MouseButtonEvent() {}
	virtual bool buttonDown() const = 0;
	virtual int button() const = 0;
    };

    class MouseMotionEvent : public InputEvent {
    public:
	~MouseMotionEvent() {}
	virtual int button() const = 0;
    };

    class KeyboardEvent : public InputEvent {
    public:
	~KeyboardEvent() {}
	virtual bool keyDown() const = 0;
	virtual bool isSpecial() const = 0;
	virtual int character() const = 0;
	virtual SpecialKey special() const = 0;
    };

    /*
     * Subclasses only need to implement relavent parts of interface.
     */
    class Controller {
    public:
	virtual ~Controller() {}

	virtual void display(Window window) {}
	virtual void reshape(Window window, Vector2D<int> newSize) {}
	virtual void windowStatus(Window window, WindowVisibility status) {}
	// Return true if program should exit afterwards.
	virtual bool closeWindow(Window window) {}

	virtual void keyboard(Window window, const KeyboardEvent& event) {}

	virtual void mouseButton(Window window, const MouseButtonEvent& event) {}
	virtual void mouseMotion(Window window, const MouseMotionEvent& event) {}
	virtual void windowEntry(Window window, bool entered) {}

	// Joystick stuff may go here.
	// Spaceball stuff may go here.

	// Menu stuff

	virtual void timer(int id) {}

	// Return true if actual idle
	virtual bool idle() { return false; }
    };

    class PushContext {
	static PushContext* s_currentContext;
    public:
	PushContext(Window window);
	PushContext(class WindowImpl* impl);
	~PushContext();
    private:
	void enter();

	PushContext*      _prevContext;
	class WindowImpl* _window;
    };

    class Menu {
    public:
	virtual ~Menu();
	virtual Window* window();
    };

    class Font {
    public:
	virtual ~Font();
	virtual void bitmapCharacter(int character) = 0;
	virtual int bitmapWidth(int character) = 0;
	virtual void strokeCharacter(int character) = 0;
	virtual int strokeWidth(int character) = 0;
	virtual int bitmapLength(const unsigned char* string) = 0;
	virtual int strokeLength(const unsigned char* string) = 0;
    };

    class Exception : public std::exception {
	const char* _what;
    public:
	Exception(const char* what) {
	    _what = what;
	}
	const char* what() {
	    return _what;
	}
    };

    class FatalError : public Exception {
    public:
	FatalError(const char* what) : Exception(what) {}
    };
};


#endif // __COM_STOCHASTIC_GLUTPLUSPLUS_H__ 1
