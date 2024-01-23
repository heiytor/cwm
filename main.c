#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Simple panic util
 */
void panic(char *msg) {
  puts(msg);
  exit(EXIT_FAILURE);
}

/* X11 connection */
Display *dpy;
/* Display's root */
Window root;

/* Managed windows */
Window windows[64] = {};
/* The index that the next window must be stored */
int next_window = 0;

void grabKey(char *key, unsigned int mod) {
  // Grab keys to receive events
  // > a. Get a keysymbol of a string.
  KeySym sym = XStringToKeysym(key);
  // > b. Get the keycode of this keysym.
  KeyCode code = XKeysymToKeycode(dpy, sym);
  // > c. Grab the key on the root window.
  XGrabKey(dpy, code, mod, root, false, GrabModeAsync, GrabModeAsync);
  // > d. Synchronize the changes.
  XSync(dpy, false);
}

void onCreateNotify(XCreateWindowEvent *e) {
  windows[next_window] = e->window;
  printf("windows %lu\n", windows[next_window]);
  ++next_window;
}

void onMapRequest(XMapRequestEvent *e) {
  printf("map %lu\n", e->window);
  XMapWindow(dpy, e->window);
}

int main(void) {
  // Open a display
  dpy = XOpenDisplay(NULL);
  if (dpy == NULL) {
    panic("Unable to open a X display.");
  }

  root = DefaultRootWindow(dpy);

  // Request the X server to send events related to `SubstructureRedirectMask`,
  // `ResizeRedirectMask` and `SubstructureNotifyMask`.
  // > a. Select the event mask for the root.
  XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask);
  // > b. Synchronize the changes.
  XSync(dpy, 0);

  // Create and define the cursor.
  // > a. Create a cursor using the standard `X11/cursorfont.h`.
  Cursor cursor = XCreateFontCursor(dpy, XC_left_ptr);
  // > b. Define the cursor for the root window.
  XDefineCursor(dpy, root, cursor);
  // > c. Synchronize the changes.
  XSync(dpy, 0);

  // Tells X to send ButtonPress events on the root.
  XGrabButton(dpy, Button1, 0, root, 0, ButtonPressMask, GrabModeSync,
              GrabModeAsync, NULL, NULL);

  // Receive a `KeyPress` event when pressing "a" + "Shift".
  grabKey("a", ShiftMask);

  XEvent e;
  for (;;) {
    XNextEvent(dpy, &e);

    switch (e.type) {
    case ButtonPress:
      // Unfreeze X to allow more mouse events for the root and all children.
      XAllowEvents(dpy, ReplayPointer, CurrentTime);
      XSync(dpy, 0);
      puts("Button pressed!");
      break;

    case KeyPress:
      puts("Key pressed!");
      break;

    // When a window is created, a `CreateNotify` is received
    case CreateNotify:
      onCreateNotify(&e.xcreatewindow);
      break;

    // When the created window needs to be mapped, a `MapRequest` is received
    // This request also can be triggered by already created clients.
    case MapRequest:
      onMapRequest(&e.xmaprequest);
      break;

    default:
      puts("Unexpected event.");
      break;
    }

    // Ensures that X will proccess the event.
    XSync(dpy, 0);
  }

  // We need to manualy close the display.
  XCloseDisplay(dpy);
}
