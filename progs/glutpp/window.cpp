#include <boost/make_shared.hpp>
#include <GL/gl.h>
#include <glutpp.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
namespace x11 {
#include <X11/Xlib.h>
};
static pthread_key_t pthread_key;

using namespace glutpp;

class TestController : public Controller {
public:
    void display(Window window) {
	glClearColor(1.0, 0.5, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();
    }
    void reshape(Window window, Vector2D<int> newSize) {
	fprintf(stderr, "reshape %d, %d\n", newSize.x, newSize.y);
    }
    void windowStatus(Window window, WindowVisibility status) {
	fprintf(stderr, "windowStatus %d\n", (int)status);
    }
    bool closeWindow(Window window) {
	fprintf(stderr, "Window should close.\n");
	return true;
    }
    void keyboard(Window window, const KeyboardEvent& event) {
	if (event.keyDown()) {
	    if (!event.isSpecial()) {
		fprintf(stderr, "%c", (char) event.character());
	    } else {
		fprintf(stderr, "<S>");
	    }
	} else {
	    fprintf(stderr, "<U%d,%d>", event.character(), (int)event.special());
	}
    }
    void mouseButton(Window window, const MouseButtonEvent& event) {
	fprintf(stderr, "Button %d %s @ %d,%d\n",
		event.button(), event.buttonDown() ? "DOWN" : "UP",
		event.position().x, event.position().y);
    }
    void mouseMotion(Window window, const MouseMotionEvent& event) {
	fprintf(stderr, "<M %d,%d>", event.position().x, event.position().y);
    }
    void windowEntry(Window window, bool entered) {
	fprintf(stderr, "Window %s\n", entered ? "entered" : "left");
    }
};

static int abortOnError(x11::Display* display, x11::XErrorEvent* event)
{
    fprintf(stderr, "Display %p Error %p\n", display, event);
    abort();
}

static int abortOnIOError(x11::Display* display)
{
    fprintf(stderr, "IO Error\n");
    abort();
}

int main(int argc, char* argv[])
{
    pthread_key_create(&pthread_key, NULL);  // Probably doesn't matter where; just need the symbol.
    Display display = Display::open("", argc, argv);
    x11::XSetErrorHandler(abortOnError);
    x11::XSetIOErrorHandler(abortOnIOError);
    Screen screen = display.defaultScreen();
    fprintf(stdout, "screen = %p\n", screen);
    Vector2D<int> size = screen.size();
    fprintf(stderr, "screen width = %d, height = %d\n", size.x, size.y);
    size.x = 800;
    size.y = 600;
    Window window = screen.createWindow("Test", NULL, &size,
					DisplayMode::RGBA | DisplayMode::SINGLE | DisplayMode::DEPTH |
					DisplayMode::MULTISAMPLE);
    TestController* controller = new TestController();
    window.registerController(controller);
    while (display.mainLoopEvent()) {
	display.pause(10);
    }
    display.close();
}
