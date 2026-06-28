#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput2.h>
#include <stdio.h>

typedef struct {
    int enabled;
    int xi_opcode;
    Window window;
    Display *display;
    float yaw;
    float pitch;
    float sensitivity;
} MouseLook;

static int init_raw_mouse(Display *dpy, Window win, MouseLook *ml)
{
    int xi_major = 2;
    int xi_minor = 0;

    if (XIQueryVersion(dpy, &xi_major, &xi_minor) != Success) {
        fprintf(stderr, "XI2 not available\n");
        return 0;
    }

    int opcode = 0, event = 0, error = 0;
    if (!XQueryExtension(dpy, "XInputExtension", &opcode, &event, &error)) {
        fprintf(stderr, "XInput extension not available\n");
        return 0;
    }

    ml->xi_opcode = opcode;
    ml->display = dpy;
    ml->window = win;

    unsigned char mask[XIMaskLen(XI_RawMotion)] = {0};

    XIEventMask m;
    m.deviceid = XIAllMasterDevices;
    m.mask_len = sizeof(mask);
    m.mask = mask;

    XISetMask(m.mask, XI_RawMotion);
    XISelectEvents(dpy, DefaultRootWindow(dpy), &m, 1);

    XFlush(dpy);
    return 1;
}

static void enter_mouse_look(MouseLook *ml)
{
    XFixesHideCursor(ml->display, ml->window);
    XGrabPointer(
            ml->display,
            ml->window,
            True,
            0,
            GrabModeAsync,
            GrabModeAsync,
            ml->window,
            None,
            CurrentTime
            );
    ml->enabled = 1;
}

static void handle_xi_event(Display *dpy, XEvent *event, MouseLook *ml)
{
    if (event->xcookie.extension != ml->xi_opcode) {
        return;
    }

    if (!XGetEventData(dpy, &event->xcookie)) {
        return;
    }

}

