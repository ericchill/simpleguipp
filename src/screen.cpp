#include "glutpp_internal.h"

using namespace glutpp;

/* ====== Screen ===== */

Screen::~Screen()
{}

Display Screen::display()
{
    return _impl->display();
}

Window Screen::rootWindow()
{
    return _impl->rootWindow();
}

Window Screen::createWindow(const char* title, Vector2D<int>* position, Vector2D<int>* size, DisplayMode mode)
{
    return _impl->createWindow(title, position, size, mode);
}

Vector2D<int> Screen::size()
{
    return _impl->size();
}

Vector2D<int> Screen::sizeMM()
{
    return _impl->sizeMM();
}


/* ====== ScreenImpl ===== */

ScreenImpl::ScreenImpl(DisplayImpl* display, int screenNum)
    : _display(display), _screenNum(screenNum), _xRootWindow(RootWindow(display->xDisplay(), screenNum)),
      _singletonRootWindow(nullptr)
{
    int mask = x11::XParseGeometry(nullptr, &_defaultWindowX, &_defaultWindowY, &_defaultWindowWidth, &_defaultWindowHeight);
    _useDefaultWindowSize = ((mask & (WidthValue|HeightValue)) == (WidthValue|HeightValue));
    if (mask & XNegative) {
	_defaultWindowX += DisplayWidth(_display->xDisplay(), _screenNum) - _defaultWindowWidth;
    }
    if (mask & YNegative) {
	_defaultWindowY += DisplayHeight(_display->xDisplay(), _screenNum) - _defaultWindowHeight;
    }
    _useDefaultWindowPosition = ((mask & (XValue|YValue)) == (XValue|YValue));
}

ScreenImpl::~ScreenImpl()
{
}

Window ScreenImpl::rootWindow()
{
    return Window(_singletonRootWindow);
}

Window ScreenImpl::createWindow(const char* title, Vector2D<int>* position, Vector2D<int>* size, DisplayMode fbMode)
{
    return Window(new WindowImpl(this, nullptr, title, position, size, fbMode, false));
}

Vector2D<int> ScreenImpl::size()
{
    return Vector2D<int>(DisplayWidth(_display->xDisplay(), _screenNum),
			 DisplayHeight(_display->xDisplay(), _screenNum));
}

Vector2D<int> ScreenImpl::sizeMM()
{
    return Vector2D<int>(DisplayWidthMM(_display->xDisplay(), _screenNum),
			 DisplayHeightMM(_display->xDisplay(), _screenNum));
}


/* ===== Private ===== */

x11::Window ScreenImpl::xRootWindow() const
{
    return _xRootWindow;
}
