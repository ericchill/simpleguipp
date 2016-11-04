#include "glutpp_internal.h"
#include "xstuff.h"
namespace x11 {
#define class foo
#include <GL/glxint.h>
#undef class
};

#include <string.h>

#define NUM_AA_SAMPLES 4

using namespace glutpp;

/* ===== Window ===== */

Window::~Window()
{}

void Window::enterContext()
{
    _impl->enterContext();
}

void Window::registerController(Controller* controller)
{
    _impl->registerController(controller);
}


Controller* Window::controller()
{
    return _impl->controller();
}

Screen Window::screen()
{
    return _impl->screen();
}

Window Window::parent()
{
    return _impl->parent();
}

Iterator<Window>* Window::children()
{
    return _impl->children();
}

DisplayMode Window::fbMode()
{
    return _impl->fbMode();
}

Window Window::createSubwindow(Vector2D<int>* position, Vector2D<int>* size)
{
    return _impl->createSubwindow(position, size);
}

void Window::setTitle(const char* title)
{
    _impl->setTitle(title);
}

void Window::setIconTitle(const char* title)
{
    _impl->setIconTitle(title);
}

void Window::position(const Vector2D<int>& position)
{
    _impl->position(position);
}

Vector2D<int> Window::position()
{
    return _impl->position();
}

void Window::size(const Vector2D<int>& size)
{
    _impl->size(size);
}

Vector2D<int> Window::size()
{
    return _impl->size();
}

void Window::show()
{
    _impl->show();
}

void Window::hide()
{
    _impl->hide();
}

void Window::iconify()
{
    _impl->iconify();
}

void Window::toFront()
{
    _impl->toFront();
}

void Window::toBack()
{
    _impl->toBack();
}

void Window::close()
{
    _impl->close();
}

void Window::swap()
{
    _impl->swap();
}


/* ===== WindowImpl ===== */

int WindowImpl::s_nextId = 0;
x11::GLXContext WindowImpl::s_menuContext = nullptr;
List<WindowImpl*> WindowImpl::s_allWindows;
List<WindowImpl*> WindowImpl::s_windowsToDestroy;

PushContext* PushContext::s_currentContext = nullptr;


static int windowIsVisible(x11::Display* display, x11::XEvent* event, x11::XPointer arg)
{
    return (event->type == MapNotify) && (event->xmap.window == (x11::Window)arg);
}

void WindowImpl::addWindow(WindowImpl* window)
{
    s_allWindows.append(new ListNode<WindowImpl*>(window));
}

Iterator<WindowImpl*>* WindowImpl::allWindows()
{
    return new IteratorImpl<WindowImpl*,WindowImpl*>(s_allWindows);
}

WindowImpl* WindowImpl::forXWindow(x11::Window winId)
{
    for (ListNode<WindowImpl*>* node = s_allWindows.first(); node; node = node->next()) {
	WindowImpl* window = node->value();
	if (window->xWindow() == winId) {
	    return window;
	}
    }
    return nullptr;
}

void WindowImpl::addWindowToDestroy(WindowImpl* window)
{
    s_windowsToDestroy.append(new ListNode<WindowImpl*>(window));
}

void WindowImpl::destroyWindows()
{
    for (ListNode<WindowImpl*>* node = s_windowsToDestroy.first(); node;) {
	ListNode<WindowImpl*>* next = node->next();
	WindowImpl* window = node->value();
	if (window->controller()) {
	    window->controller()->closeWindow(Window(window));
	    x11::glFlush();
	}
	delete node;
	delete window;
	node = next;
    }
}

void WindowImpl::displayAllWindows()
{
    for (ListNode<WindowImpl*>* node = s_allWindows.first(); node; node = node->next()) {
	WindowImpl* window = node->value();
	if (window->controller()) {
	    window->needsRedisplay(false);
	    window->controller()->display(Window(window));
	    x11::glFlush();
	}
    }
}

void WindowImpl::redisplayWindows()
{
    for (ListNode<WindowImpl*>* node = s_allWindows.first(); node; node = node->next()) {
	WindowImpl* window = node->value();
	if (window->needsRedisplay() && window->controller()) {
	    window->needsRedisplay(false);
	    window->controller()->display(Window(window));
	    x11::glFlush();
	}
    }
}

WindowImpl::WindowImpl(ScreenImpl* screen, WindowImpl* parent, const char* title,
		       Vector2D<int>* position, Vector2D<int>* size,
		       DisplayMode fbMode, bool isMenu)
    : _screen(screen), _parent(parent), _fbMode(fbMode), _isMenu(isMenu), _isVisible(false), _needsRedisplay(false),
      _controller(nullptr), _fbConfigs(nullptr)
{
    _id = ++s_nextId;  // First window is 1.
    _xDisplay = screen->display()._impl->xDisplay();
    // Not root window
    int numConfigs;
    _fbConfigs = chooseFBConfig(fbMode, &numConfigs);
    if (nullptr == _fbConfigs) {
	throw FatalError("Display mode unavailable.");
    }
    x11::XVisualInfo* vInfo = nullptr;
    for (int i = 0; i < numConfigs; i++) {
	vInfo = x11::glXGetVisualFromFBConfig(_xDisplay, _fbConfigs[i]);
	if (vInfo) {
	    break;
	}
    }
    if (nullptr == vInfo) {
	x11::XFree(_fbConfigs);
	throw FatalError("No VisualInfo for display mode.");
    }
    x11::XSetWindowAttributes swa;
    swa.event_mask = (StructureNotifyMask | SubstructureNotifyMask | ExposureMask |
		      ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask |
		      VisibilityChangeMask | EnterWindowMask | LeaveWindowMask |
		      PointerMotionMask | ButtonMotionMask | FocusChangeMask);
    swa.background_pixmap = None;
    swa.background_pixel  = BlackPixel(_xDisplay, _screen->screenNum());
    swa.backing_pixel     = swa.background_pixel;
    swa.border_pixel      = 0;
    swa.colormap = x11::XCreateColormap(_xDisplay, _screen->xRootWindow(),
					vInfo->visual, AllocNone);
    unsigned long mask = CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask;
    if (isMenu) {
	mask |= CWOverrideRedirect;
    }
    _xWindow = x11::XCreateWindow(_xDisplay, (nullptr == parent) ? _screen->xRootWindow() : parent->xWindow(),
				  position ? position->x : -1,
				  position ? position->y : -1,
				  size ? size->x : -1,
				  size ? size->y : -1,
				  0, vInfo->depth, InputOutput, vInfo->visual, mask, &swa);
    // FreeGlut puts a fake event here for when there may be no window manager.
    if (isMenu) {
	if (nullptr == s_menuContext) {
	    s_menuContext = x11::glXCreateNewContext(_xDisplay, _fbConfigs[0], GLX_RGBA_TYPE, nullptr, 1);
	}
	_glXContext = s_menuContext;
    } else {
	_glXContext = x11::glXCreateNewContext(_xDisplay, _fbConfigs[0], GLX_RGBA_TYPE, nullptr, 1);
    }
    // Cursor
    x11::XSizeHints sizeHints;
    sizeHints.flags = 0;
    if (position) {
	sizeHints.flags |= USPosition;
	sizeHints.x = position->x;
	sizeHints.y = position->y;
    } else {
	sizeHints.x = -1;
	sizeHints.y = -1;
    }
    if (size) {
	sizeHints.flags |= USSize;
	sizeHints.width = size->x;
	sizeHints.height = size->y;
    } else {
	sizeHints.width = -1;
	sizeHints.height = -1;
    }
    x11::XWMHints wmHints;
    wmHints.flags = StateHint;
    wmHints.initial_state = NormalState; // or IconicState; see freeglut_window open window.
    x11::XTextProperty textProperty;
    x11::XStringListToTextProperty(const_cast<char**>(&title), 1, &textProperty);
    x11::XSetWMProperties(_xDisplay, _xWindow, &textProperty, &textProperty,
			  0, 0, &sizeHints, &wmHints, nullptr);
    x11::XFree(textProperty.value);
    int numProtocols;
    const x11::Atom* protocols = _screen->display()._impl->windowManagerProtocols(numProtocols);
    x11::XSetWMProtocols(_xDisplay, _xWindow, const_cast<x11::Atom*>(protocols), numProtocols);
    _glXWindow = x11::glXCreateWindow(_xDisplay, _fbConfigs[0], _xWindow, nullptr);
    enterContext();
    x11::XMapWindow(_xDisplay, _xWindow);
    x11::XFree(vInfo);
    if (nullptr == parent) {
	// Not a subwindow
	x11::XEvent event;
	x11::XPeekIfEvent(_xDisplay, &event, windowIsVisible, (x11::XPointer)_xWindow);
    }
    if (!any(fbMode & DisplayMode::DOUBLE)) {
	x11::glDrawBuffer(GL_FRONT);
	x11::glReadBuffer(GL_FRONT);
    }
    if (nullptr != parent) {
	parent->addChild(this);
    }
    setSizeFromAttrs();
    addWindow(this);
}

WindowImpl::~WindowImpl()
{
    if (_glXContext) {
	x11::glXDestroyContext(_xDisplay, _glXContext);
    }
    x11::XDestroyWindow(_xDisplay, _xWindow);
    if (_fbConfigs) {
	delete _fbConfigs;
    }
}

void WindowImpl::enterContext()
{
    x11::glXMakeContextCurrent(_xDisplay, _xWindow, _glXWindow, _glXContext);
}

Iterator<Window>* WindowImpl::children()
{
    return new IteratorImpl<Window, WindowImpl*>(_children);
}

Window WindowImpl::createSubwindow(Vector2D<int>* position, Vector2D<int>* size)
{
    return Window(nullptr);
}

void WindowImpl::setTitle(const char* title)
{
    if (nullptr == _parent) {
	x11::XTextProperty text;
	text.value = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(title));
	text.encoding = XA_STRING;
	text.format = 8;
	text.nitems = strlen(title);
	x11::XSetWMName(_xDisplay, _xWindow, &text);
	x11::XFlush(_xDisplay);
    }
}

void WindowImpl::setIconTitle(const char* title)
{
    if (nullptr == _parent) {
	x11::XTextProperty text;
	text.value = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(title));
	text.encoding = XA_STRING;
	text.format = 8;
	text.nitems = strlen(title);
	x11::XSetWMIconName(_xDisplay, _xWindow, &text);
	x11::XFlush(_xDisplay);
    }
}

void WindowImpl::size(const Vector2D<int>& size)
{
    x11::XResizeWindow(_xDisplay, _xWindow, size.x, size.y);
    x11::XFlush(_xDisplay);
    setSizeFromAttrs();
}

Vector2D<int> WindowImpl::size()
{
    return _size;
}

void WindowImpl::position(const Vector2D<int>& position)
{
    x11::XMoveWindow(_xDisplay, _xWindow, position.x, position.y);
    x11::XFlush(_xDisplay);
}

Vector2D<int> WindowImpl::position()
{
    x11::XWindowAttributes attrs;
    x11::XGetWindowAttributes(_xDisplay, _xWindow, &attrs);
    return Vector2D<int>(attrs.x, attrs.y);
}

void WindowImpl::show()
{
    x11::XMapWindow(_xDisplay, _xWindow);
    x11::XFlush(_xDisplay);
    needsRedisplay(true);
}

void WindowImpl::hide()
{
    if (nullptr == _parent) {
	x11::XWithdrawWindow(_xDisplay, _xWindow, _screen->xRootWindow());
    } else {
	x11::XUnmapWindow(_xDisplay, _xWindow);
    }
    x11::XFlush(_xDisplay);
    needsRedisplay(false);
}

void WindowImpl::iconify()
{
    x11::XIconifyWindow(_xDisplay, _xWindow, _screen->screenNum());
    x11::XFlush(_xDisplay);
}

void WindowImpl::toFront()
{
    x11::XRaiseWindow(_xDisplay, _xWindow);
}

void WindowImpl::toBack()
{
    x11::XLowerWindow(_xDisplay, _xWindow);
}

void WindowImpl::close()
{
    addWindowToDestroy(this);
}

void WindowImpl::swap()
{
    x11::glFlush();
    if (any(_fbMode & DisplayMode::DOUBLE)) {
	x11::glXSwapBuffers(_xDisplay, _xWindow);
    }
}


/* ===== Private ===== */

void WindowImpl::addChild(WindowImpl* child)
{
    _children.append(new ListNode<WindowImpl*>(child));
}

x11::GLXFBConfig* WindowImpl::chooseFBConfig(DisplayMode fbMode, int* numConfigs)
{
    GLAttributeList attrs;

    attrs.add(GLX_RED_SIZE, 8);
    attrs.add(GLX_GREEN_SIZE, 8);
    attrs.add(GLX_BLUE_SIZE, 8);
    if (any(fbMode & DisplayMode::ALPHA)) {
	attrs.add(GLX_ALPHA_SIZE, 8);
    }
    if (any(fbMode & DisplayMode::DOUBLE)) {
	attrs.add(GLX_DOUBLEBUFFER, True);
    }
    if (any(fbMode & DisplayMode::STEREO)) {
	attrs.add(GLX_STEREO, True);
    }
    if (any(fbMode & DisplayMode::DEPTH)) {
	attrs.add(GLX_DEPTH_SIZE, 24);
    }
    if (any(fbMode & DisplayMode::ACCUM)) {
	attrs.add(GLX_ACCUM_RED_SIZE, 8);
	attrs.add(GLX_ACCUM_GREEN_SIZE, 8);
	attrs.add(GLX_ACCUM_BLUE_SIZE, 8);
	if (any(fbMode & DisplayMode::ALPHA)) {
	    attrs.add(GLX_ACCUM_ALPHA_SIZE, 8);
	}
    }
    if (any(fbMode & DisplayMode::MULTISAMPLE)) {
	attrs.add(GLX_SAMPLE_BUFFERS, 1);
	attrs.add(GLX_SAMPLES, NUM_AA_SAMPLES);
    }
    // Aux buffers for later...
    return x11::glXChooseFBConfig(_xDisplay, _screen->screenNum(), attrs.attributes(), numConfigs);
}

void WindowImpl::setSizeFromAttrs()
{
    x11::XWindowAttributes attrs;
    x11::XGetWindowAttributes(_xDisplay, _xWindow, &attrs);
    _size = Vector2D<int>(attrs.width, attrs.height);
}

// ===== PushContext =====

PushContext::PushContext(Window window)
    : _window(window._impl)
{
    enter();
}

PushContext::PushContext(WindowImpl* window)
    : _window(window)
{
    enter();
}

PushContext::~PushContext()
{
    s_currentContext = _prevContext;
    if (_prevContext) {
	_prevContext->_window->enterContext();
    }
}

void PushContext::enter()
{
    _prevContext = s_currentContext;
    s_currentContext = this;
    _window->enterContext();
}
