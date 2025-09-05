#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <X11/Xlib.h>

#define WIDTH  800
#define HEIGHT 600

typedef struct {
    uint8_t b, g, r, a;
} Color;

int main(void) {
    Color pixels[WIDTH*HEIGHT] = {0};
    for (int i = 0; i < WIDTH*HEIGHT; i++) {
        pixels[i] = (Color){ .r=0xFF, .g=0x00, .b=0x00, .a=0xFF };

        if (i > WIDTH*HEIGHT/2) {
            pixels[i] = (Color){ .r=0x00, .g=0xFF, .b=0x00, .a=0xFF };
        }
    }

    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "ERROR: could not open the default display\n");
        exit(1);
    }

    Window window = XCreateSimpleWindow(
        display,
        XDefaultRootWindow(display),
        0, 0,
        WIDTH, HEIGHT,
        0,
        0,
        0
    );

    XStoreName(display, window, "FLOAT");

    XWindowAttributes wa = {0};
    XGetWindowAttributes(display, window, &wa);

    XImage *image;
    image = XCreateImage(
        display,
        wa.visual,
        wa.depth,
        ZPixmap,
        0,
        (char*) pixels,
        WIDTH,
        HEIGHT,
        32,
        WIDTH * sizeof(uint32_t)
    );

    GC gc = XCreateGC(display, window, 0, NULL);

    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    XSelectInput(display, window, KeyPressMask | PointerMotionMask);

    XMapWindow(display, window);

    int quit = 0, i = 0;
    while (!quit) {
        while (XPending(display) > 0) {
            XEvent event = {0};
            XNextEvent(display, &event);
            switch (event.type) {
                case KeyPress: {
                    switch (XLookupKeysym(&event.xkey, 0)) {
                        case 'q':
                            quit = 1;
                            break;
                    }
                    break;
                }

                // case MotionNotify: {
                //     printf("%d - %d\n", event.xmotion.x, event.xmotion.y);
                //     break;
                // }

                case ClientMessage: {
                    if ((Atom) event.xclient.data.l[0] == wm_delete_window) {
                        quit = 1;
                    }
                    break;
                }

                // default: {
                //     printf("[%d]\tEvent.type=%d\n", time(NULL), event.type);
                // }
            }
        }

        XPutImage(display, window, gc, image, 0, 0, 0, 0, WIDTH, HEIGHT);
        i += 1;
    }

    XCloseDisplay(display);

    return 0;
}
