#ifndef __COM_STOCHASTIC_GLUTPLUSPLUS_INTERNAL_H__
#define __COM_STOCHASTIC_GLUTPLUSPLUS_INTERNAL_H__ 1

#include <glutpp.h>

#include <limits.h>
// Put includes for system stuff before this line to avoid confusion from glx and Xlib.
// x11 namespace scopes generic names used by Xlib.
namespace x11 {
#   include <GL/gl.h>
#   include <GL/glext.h>
#   include <GL/glx.h>
#   include <X11/Xatom.h>
};
// Need these for Xlib macros.
typedef x11::Atom Atom;
typedef x11::_XPrivDisplay _XPrivDisplay;

#include "glattributes.h"
#include "list.h"

namespace glutpp {

    class ScreenImpl;
    class WindowImpl;

    enum class RunState {
	STATE_INITIAL,
	STATE_RUNNING,
	STATE_STOP_REQUESTED,
	STATE_STOPPED
    };

    class DisplayImpl : public DisplayInterface {
    private:
	static DisplayImpl* TheOne;

	const char*   _name;
	int           _argc;
	char**        _argv;

	x11::Display* _xDisplay;
	int           _connection;
	int           _defaultScreenNum;
	ScreenImpl*   _defaultScreen;

	static const int s_maxProtocols = 1;
	x11::Atom     _deleteWindow;
	int           _numWMProtocols;
	x11::Atom     _allWMProtocols[s_maxProtocols];

	RunState        _runState;
    private:
	// Prevent others from instantiating to enforce singleton.
	DisplayImpl(const char* name, int argc, char* argv[]);
    public:
	static DisplayImpl* open(const char* name, int argc, char* argv[]);
	static Display theDisplay();

	~DisplayImpl();
	void close();
	Screen defaultScreen();
	void setTimer(unsigned int time, int value);

	void mainLoop();
	bool mainLoopEvent();
	bool pause(int millisecs);

	// Internal

	x11::Display* xDisplay() const { return _xDisplay; }
	const x11::Atom* windowManagerProtocols(int& num);
	x11::Atom getAtom(const char* name);
	bool isNetWMSupported();

	/*
	 * data must be released with XFree.
	 */
	int getWindowProperty(x11::Window window, x11::Atom property, x11::Atom type, unsigned char** data);
	bool hintPresent(x11::Window window, x11::Atom property, x11::Atom hint);

    private:
	void handleResize(WindowImpl* window, const Vector2D<int>& newSize);
    };

    class ScreenImpl : public ScreenInterface {
    private:
	DisplayImpl*  _display;
	int           _screenNum;
	x11::Window   _xRootWindow;
	WindowImpl*   _singletonRootWindow;

	bool          _useDefaultWindowSize;
	unsigned int  _defaultWindowWidth;
	unsigned int  _defaultWindowHeight;
	bool          _useDefaultWindowPosition;
	int           _defaultWindowX;
	int           _defaultWindowY;

    public:
	ScreenImpl(DisplayImpl* display, int screenNum);
	~ScreenImpl();

	Display display() { return _display; }
	Window rootWindow();
	Window createWindow(const char* title, Vector2D<int>* position, Vector2D<int>* size, DisplayMode fbMode);

	Vector2D<int> size();
	Vector2D<int> sizeMM();

	// Internal

	int screenNum() { return _screenNum; }
	x11::Window xRootWindow() const;
    };

    class WindowImpl : public WindowInterface {
    private:
	static int s_nextId;
	static x11::GLXContext s_menuContext;
    	static List<WindowImpl*> s_allWindows;
	static List<WindowImpl*> s_windowsToDestroy;
	int _id;
	ScreenImpl* _screen;
	WindowImpl* _parent;
	DisplayMode _fbMode;
	bool _isMenu;
	bool _isVisible;
	bool _needsRedisplay;
	x11::Window _xWindow;
	Controller* _controller;
	List<WindowImpl*> _children;
	x11::GLXFBConfig* _fbConfigs;
	x11::GLXWindow _glXWindow;
	x11::GLXContext _glXContext;
	x11::Display* _xDisplay;
	Vector2D<int> _size;

    public:
	static void addWindow(WindowImpl* window);
	static Iterator<WindowImpl*>* allWindows();
	static WindowImpl* forXWindow(x11::Window winID);
	static void addWindowToDestroy(WindowImpl* window);
	static void destroyWindows();
	static void displayAllWindows();
	static void redisplayWindows();

	WindowImpl(ScreenImpl* screen, WindowImpl* parent, const char* title,
		   Vector2D<int>* position, Vector2D<int>* size, DisplayMode fbMode, bool isMenu);
	~WindowImpl();

	void enterContext();
	void registerController(Controller* controller) { _controller = controller; }
	Controller* controller() { return _controller; }

	Screen screen() { return Screen(_screen); }
	Window parent() { return Window(_parent); }
	DisplayMode fbMode() { return _fbMode; }
	Iterator<Window>* children();
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

	// Internal

	x11::Window xWindow() const { return _xWindow; }
	void addChild(WindowImpl* child);
	void visibility(bool isVisible) { _isVisible = isVisible; }
	void needsRedisplay(bool flag) { _needsRedisplay = flag; }
	bool needsRedisplay() const { return _needsRedisplay; }
	bool isMenu() const { return _isMenu; }
    private:
	x11::GLXFBConfig* chooseFBConfig(DisplayMode fbMode, int* numConfigs);
	x11::Display* xDisplay() const { return _screen->display()._impl->xDisplay(); }
	void setSizeFromAttrs();
    };

};

#endif // __COM_STOCHASTIC_GLUTPLUSPLUS_INTERNAL_H__
