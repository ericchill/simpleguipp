#include "glutpp_internal.h"
#include "keyboard.h"
#include "mouse.h"
#include "tester.h"
#include "xstuff.h"

#include <stdio.h>
#include <sys/select.h>
#include <time.h>

static pthread_key_t pthread_key;

using namespace glutpp;

int xErrorHandler(x11::Display* xDisplay, x11::XErrorEvent* event) {
    abort();
}

int xIOErrorHandler(x11::Display* xDisplay) {
    abort();
}

/* ====== Display ===== */

Display Display::open(const char* name, int argc, char* argv[])
{
    pthread_key_create(&pthread_key, nullptr);  // Probably doesn't matter where; just need the symbol.
    return Display(DisplayImpl::open(name, argc, argv));
}

Display::~Display() {}

void Display::close()
{
    _impl->close();
}

Screen Display::defaultScreen()
{
    return _impl->defaultScreen();
}

void Display::setTimer(unsigned int time, int value)
{
    _impl->setTimer(time, value);
}

void Display::mainLoop()
{
    _impl->mainLoop();
}

bool Display::mainLoopEvent()
{
    return _impl->mainLoopEvent();
}

bool Display::pause(int millisecs)
{
    return _impl->pause(millisecs);
}


/* ====== DisplayImpl ===== */

DisplayImpl* DisplayImpl::TheOne = nullptr;

DisplayImpl* DisplayImpl::open(const char* name, int argc, char* argv[])
{
    if (!TheOne) {
	TheOne = new DisplayImpl(name, argc, argv);
    }
    return TheOne;
}

Display DisplayImpl::theDisplay()
{
    return Display(TheOne);
}

DisplayImpl::DisplayImpl(const char* name, int argc, char* argv[])
    : _name(name), _argc(argc), _argv(argv), _numWMProtocols(0), _runState(RunState::STATE_INITIAL)
{
    x11::XSetErrorHandler(xErrorHandler);
    x11::XSetIOErrorHandler(xIOErrorHandler);
    _xDisplay = x11::XOpenDisplay(name);
    if (!x11::glXQueryExtension(_xDisplay, nullptr, nullptr)) {
        throw FatalError("Failed to open display");
    }
    _defaultScreenNum = DefaultScreen(_xDisplay);
    _defaultScreen = new ScreenImpl(this, _defaultScreenNum);
    _deleteWindow = getAtom("WM_DELETE_WINDOW");
    _allWMProtocols[_numWMProtocols++] = _deleteWindow;
    if (isNetWMSupported()) {
	x11::Atom supported = getAtom("_NET_SUPPORTED");
	x11::Window rootWindow = _defaultScreen->xRootWindow();
	if (hintPresent(rootWindow, supported, getAtom("_NET_WM_STATE"))) {
	    // Things like full screen.
	}
    }
}

DisplayImpl::~DisplayImpl()
{
    TheOne = nullptr;
    delete _defaultScreen;
    x11::XSetCloseDownMode(_xDisplay, DestroyAll);
    x11::XCloseDisplay(_xDisplay);
}

void DisplayImpl::close()
{
    delete this;
}

Screen DisplayImpl::defaultScreen()
{
    return Screen(_defaultScreen);
}

void DisplayImpl::setTimer(unsigned int time, int value)
{
}

void DisplayImpl::mainLoop()
{
    while (mainLoopEvent()) {
	pause(10);
    }
}

bool DisplayImpl::mainLoopEvent()
{
    if (_runState == RunState::STATE_INITIAL) {
	_runState = RunState::STATE_RUNNING;
    }
    while (_runState == RunState::STATE_RUNNING && x11::XPending(_xDisplay)) {
	x11::XEvent event;
	x11::XNextEvent(_xDisplay, &event);
	//fprintf(stderr, "Event %s   ", xEventNames[event.type]);
	switch (event.type) {
	case ClientMessage:
	    {
		/*fprintf(stderr, "ClientMessage type=%s format=%d data.l[0]=%s\n",
			x11::XGetAtomName(_xDisplay, event.xclient.message_type),
			event.xclient.format,
			x11::XGetAtomName(_xDisplay, (x11::Atom)event.xclient.data.l[0]));*/
		WindowImpl* window = WindowImpl::forXWindow(event.xclient.window);
		if ((x11::Atom)event.xclient.data.l[0] == _deleteWindow) {
		    WindowImpl::addWindowToDestroy(window);
		    if (window->controller() && window->controller()->closeWindow(window)) {
			_runState = RunState::STATE_STOP_REQUESTED;
		    }
		}
	    }
	    break;
	case ConfigureNotify:
	case CreateNotify:
	    {
		WindowImpl* window;
		Vector2D<int> newSize;
		if (event.type == CreateNotify) {
		    window = WindowImpl::forXWindow(event.xcreatewindow.window);
		    newSize.x = event.xcreatewindow.width;
		    newSize.y = event.xcreatewindow.height;
		} else {
		    window = WindowImpl::forXWindow(event.xconfigure.window);
		    newSize.x = event.xconfigure.width;
		    newSize.y = event.xconfigure.height;
		}
		handleResize(window, newSize);
	    }
	    break;
	case DestroyNotify:
	    break;
	case Expose:
	    if (event.xexpose.count == 0) {
		WindowImpl* window = WindowImpl::forXWindow(event.xexpose.window);
		window->needsRedisplay();
		Vector2D<int> newSize(event.xexpose.width, event.xexpose.height);
		handleResize(window, newSize);
	    }
	    break;
	case MapNotify:
	    break;
	case UnmapNotify:
	    {
		WindowImpl* window = WindowImpl::forXWindow(event.xunmap.window);
		window->visibility(false);
		Controller* controller = window->controller();
		if (controller) {
		    controller->windowStatus(window, WindowVisibility::HIDDEN);
		}
	    }
	    break;
	case MappingNotify:
	    x11::XRefreshKeyboardMapping((x11::XMappingEvent*)&event);
	    break;
	case VisibilityNotify:
	    {
		WindowImpl* window = WindowImpl::forXWindow(event.xvisibility.window);
		WindowVisibility status;
		switch (event.xvisibility.state) {
		case VisibilityFullyObscured:
		    status = WindowVisibility::FULLY_COVERED;
		    window->visibility(false);
		    break;
		case VisibilityPartiallyObscured:
		    status = WindowVisibility::PARTIALLY_RETAINED;
		    window->visibility(true);
		    break;
		case VisibilityUnobscured:
		    status = WindowVisibility::FULLY_RETAINED;
		    window->visibility(true);
		    break;
		}
		Controller* controller = window->controller();
		if (controller) {
		    controller->windowStatus(window, status);
		}
	    }
	    break;
	case EnterNotify:
	case LeaveNotify:
	    {
		WindowImpl* window = WindowImpl::forXWindow(event.xcrossing.window);
		Controller* controller = window->controller();
		if (controller) {
		    controller->windowEntry(window, event.type == EnterNotify);
		}
	    }
	    break;
	case MotionNotify:
	    {
		// menu
		WindowImpl* window = WindowImpl::forXWindow(event.xmotion.window);
		if (window && window->controller()) {
		    MouseMotionEventImpl motionEvent(&event);
		    window->controller()->mouseMotion(window, motionEvent);
		}
	    }
	    break;
	case ButtonPress:
	case ButtonRelease:
	    {
		WindowImpl* window = WindowImpl::forXWindow(event.xbutton.window);
		Controller* controller = window->controller();
		if (controller) {
		    MouseButtonEventImpl buttonEvent(&event);
		    controller->mouseButton(window, buttonEvent);
		}
	    }
	    break;
	case KeyPress:
	case KeyRelease:
	    {
		if (event.xkey.keycode) {  // Mouse movement shows up here with keycode zero.
		    WindowImpl* window = WindowImpl::forXWindow(event.xkey.window);
		    Controller* controller = window->controller();
		    if (controller) {
			KeyboardEventImpl keyEvent(&event);
			controller->keyboard(window, keyEvent);
		    }
		}
	    }
	    break;
	case ReparentNotify:
	    break;
	case GravityNotify:
	    break;
	default:
	    break;
	}
    }
    if (_runState == RunState::STATE_RUNNING) {
	// Check timers
	WindowImpl::displayAllWindows();
    }
    WindowImpl::destroyWindows();
    return _runState == RunState::STATE_RUNNING;
}

bool DisplayImpl::pause(int millisecs)
{
    // Check for idle or pending redisplays
    bool foundIdle = false;
    for (auto iter = WindowImpl::allWindows(); iter->hasMore();) {
	if (iter->next()->controller()->idle()) {
	    foundIdle = true;
	}
    }
    if (foundIdle) {
	return false;
    }
    // TODO: Take timer into account for wait time.
    int msec = millisecs;
    if (!x11::XPending(_xDisplay)) {
	fd_set fdset;
	struct timeval wait;
	int socket = ConnectionNumber(_xDisplay);
	FD_ZERO(&fdset);
	FD_SET(socket, &fdset);
	wait.tv_sec = msec / 1000;
	wait.tv_usec = msec * 1000;
	int nfds = select(socket + 1, &fdset, nullptr, nullptr, &wait);
	// fprintf(stderr, "Select nfds = %d\n", nfds);
    }
}

/* ===== Internal ===== */

const x11::Atom* DisplayImpl::windowManagerProtocols(int& num)
{
    num = _numWMProtocols;
    return _allWMProtocols;
}

x11::Atom DisplayImpl::getAtom(const char* name)
{
    return x11::XInternAtom(_xDisplay, name, False);
}

bool DisplayImpl::isNetWMSupported()
{
    bool netWMSupported = false;
    x11::Atom wmCheck = getAtom("_NET_SUPPORTING_WM_CHECK");
    x11::Window** windowPtr1 = new x11::Window*;

    /*
     * Check that the window manager has set this property on the root window.
     * The property must be the ID of a child window.
     */
    int numWindows = getWindowProperty(_defaultScreen->xRootWindow(), wmCheck, XA_WINDOW, (unsigned char **) windowPtr1);
    if (numWindows == 1) {
	x11::Window** windowPtr2 = new x11::Window*;
	/* Check that the window has the same property set to the same value. */
	numWindows = getWindowProperty(**windowPtr1, wmCheck, XA_WINDOW, (unsigned char **) windowPtr2);
	if ((numWindows == 1) && (**windowPtr1 == **windowPtr2)) {
	    netWMSupported = true;
	}
	x11::XFree(*windowPtr2);
	delete windowPtr2;
    }
    x11::XFree(*windowPtr1);
    delete windowPtr1;
    return netWMSupported;
}

int DisplayImpl::getWindowProperty(x11::Window window, x11::Atom property, x11::Atom type, unsigned char** data)
{
    x11::Atom typeReturned;
    int unusedFormat;
    unsigned long numElements;
    unsigned long unusedBytesAfter;

    int status = x11::XGetWindowProperty(_xDisplay, window, property, 0, LONG_MAX, False, type, &typeReturned,
					 &unusedFormat, &numElements, &unusedBytesAfter, data);
    if (status != Success) {
	throw FatalError("XGetWindowProperty failled in Screen::getWindowProperty");
    }
    if (typeReturned != type) {
	return 0;
    }
    return numElements;
}

bool DisplayImpl::hintPresent(x11::Window window, x11::Atom property, x11::Atom hint)
{
    x11::Atom *atoms;
    bool supported = false;

    int numAtoms = getWindowProperty(window, property, XA_ATOM, (unsigned char **) &atoms);
    for (int i = 0; i < numAtoms; i++) {
	if (atoms[i] == hint) {
	    supported = true;
	    break;
	}
    }
    x11::XFree(atoms);
    return supported;
}

void DisplayImpl::handleResize(WindowImpl* window, const Vector2D<int>& newSize)
{
    auto oldSize = window->size();

    //fprintf(stderr, "handleResize old=%d,%d new=%d,%d\n", oldSize.x, oldSize.y, newSize.x, newSize.y);
    if ((oldSize.x != newSize.x) || (oldSize.y != newSize.y)) {
	PushContext context(window);
	Controller* controller = window->controller();
	if (controller) {
	    controller->reshape(window, newSize);
	}
	x11::glViewport(0, 0, newSize.x, newSize.y);
	window->size(newSize);
	window->needsRedisplay();
    }
}


/* ===== Test ===== */

class DisplayTester : public Tester {
public:
    DisplayTester() : Tester("display.cpp") {}
    void test() {
	Display display = Display::open("", s_argc, s_argv);
    };
};

DisplayTester displayTest;
