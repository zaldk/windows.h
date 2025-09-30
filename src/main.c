#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <X11/Xlib.h>
#endif // _WIN32

#ifdef _WIN32
HBITMAP hbmp;
HANDLE hTickThread;
HWND hwnd;
HDC hdcMem;
#endif

#define WIDTH  800
#define HEIGHT 600

typedef struct {
    uint8_t b, g, r, a;
} Color;

Color pixels[HEIGHT][WIDTH];

#ifndef _WIN32
// {{{

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

    XSelectInput(display, window, KeyPressMask | PointerMotionMask | StructureNotifyMask);

    XMapWindow(display, window);

    clock_t frame_start = 0, frame_end = 0;
    double frame_dt = 0.0, frame_time_acc = 0.0;
    int quit = 0, i = 0;
    while (!quit) {
        frame_start = clock();
        while (XPending(display) > 0) {
            XEvent event = {0};
            XNextEvent(display, &event);
            switch (event.type) {
                case KeyPress: {
                    switch (XLookupKeysym(&event.xkey, 0)) {
                        case 'q': {
                            quit = 1;
                            break;
                        }
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

                case ConfigureNotify: {
                    int new_width = event.xconfigure.width;
                    int new_height = event.xconfigure.height;
                    printf("Window resized: x=%d, y=%d, width=%d, height=%d\n",
                           event.xconfigure.x,
                           event.xconfigure.y,
                           new_width,
                           new_height);
                    break;
                }

                // default: {
                //     printf("[%d]\tEvent.type=%d\n", time(NULL), event.type);
                // }
            }
        }

        XPutImage(display, window, gc, image, 0, 0, 0, 0, WIDTH, HEIGHT);
        i += 1;
        frame_end = clock();
        frame_dt = (double)(frame_end - frame_start) / CLOCKS_PER_SEC;
        frame_time_acc += frame_dt;
        if (frame_time_acc > 1.0) {
            frame_time_acc -= 1.0;

        for (int i = 0; i < WIDTH*HEIGHT; i++) {
            pixels[i] = (Color){ .r=0x80, .g=0x80, .b=0x80, .a=0xFF };
        }
            printf("FRAME DELTA TIME :: %lf\n", frame_dt);
            printf("             FPS :: %lf\n", 1.0/frame_dt);
        }
    }

    XCloseDisplay(display);

    return 0;
}

// }}}
#else
// {{{

// https://www.daniweb.com/programming/software-development/code/241875/fast-animation-with-the-windows-gdi

DWORD WINAPI tickThreadProc(HANDLE handle) {
    Sleep(50);
    ShowWindow(hwnd, SW_SHOW);
    ShowCursor(FALSE);

    HDC hdc = GetDC(hwnd);

    hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmp);

    // V2f ball1 = v2ff(400.0f);
    // V2f ball2 = v2ff(0.0f);

    for (;;) {
        for (int y = 0; y < HEIGHT; y += 1) {
            for (int x = 0; x < WIDTH; x += 1) {
                pixels[y][x].r = 0xFF;
                pixels[y][x].g = 0x00;
                pixels[y][x].b = 0x00;
                pixels[y][x].a = 0xFF;
            }
        }
        BitBlt(hdc, 0, 0, WIDTH, HEIGHT, hdcMem, 0, 0, SRCCOPY);
        break;
    }

    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdc);
}

void MakeSurface(HWND hwnd) {
    BITMAPINFO bmi;
    bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
    bmi.bmiHeader.biWidth = WIDTH;
    bmi.bmiHeader.biHeight = -HEIGHT;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;
    bmi.bmiHeader.biXPelsPerMeter = 0;
    bmi.bmiHeader.biYPelsPerMeter = 0;
    bmi.bmiHeader.biClrUsed = 0;
    bmi.bmiHeader.biClrImportant = 0;
    bmi.bmiColors[0].rgbBlue = 0;
    bmi.bmiColors[0].rgbGreen = 0;
    bmi.bmiColors[0].rgbRed = 0;
    bmi.bmiColors[0].rgbReserved = 0;

    HDC hdc = GetDC(hwnd);

    hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void **)&pixels, NULL, 0);
    DeleteDC(hdc);

    hTickThread = CreateThread(NULL, 0, &tickThreadProc, NULL, 0, NULL);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        MakeSurface(hwnd);
    }
    break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        BitBlt(hdc, 0, 0, WIDTH, HEIGHT, hdcMem, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);
    }
    break;
    case WM_CLOSE: {
        DestroyWindow(hwnd);
    }
    break;
    case WM_DESTROY: {
        TerminateThread(hTickThread, 0);
        PostQuitMessage(0);
    }
    break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    WNDCLASSEX wc;
    MSG msg;

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hbrBackground = CreateSolidBrush(0);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = "animation_class";
    wc.lpszMenuName = NULL;
    wc.style = 0;

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Failed to register window class.", "Error", MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(
               WS_EX_APPWINDOW,
               "animation_class",
               "metaballs",
               WS_MINIMIZEBOX | WS_SYSMENU | WS_POPUP | WS_CAPTION,
               CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,
               NULL, NULL, hInstance, NULL);

    while (GetMessage(&msg, 0, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
// }}}
#endif
