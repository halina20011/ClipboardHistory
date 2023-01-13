// gcc main.c -o ./Build/main -lX11 -lXfixes && ./Build/main

#include <stdio.h>
#include <limits.h>

// https://www.x.org/releases/current/doc/index.html
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>

#include <assert.h>

#include "file.c"

char *fileName = "/tmp/clipboardText";

Bool PrintSelection(Display *display, Window window, const char *bufname, const char *fmtname){
    unsigned char *result;
    unsigned long ressize, restail;
    int resbits;

    Atom bufid = XInternAtom(display, bufname, False),
    fmtid = XInternAtom(display, fmtname, False),
    propid = XInternAtom(display, "XSEL_DATA", False),
    incrid = XInternAtom(display, "INCR", False);
    XEvent event;

    XConvertSelection(display, bufid, fmtid, propid, window, CurrentTime);
    do{
        XNextEvent(display, &event);
    } while (event.type != SelectionNotify || event.xselection.selection != bufid);

    if(event.xselection.property){
        XGetWindowProperty(display, window, propid, 0, LONG_MAX/4, False, AnyPropertyType,
            &fmtid, &resbits, &ressize, &restail, (unsigned char**)&result);

        if(fmtid == incrid){
            printf("Buffer is too large and INCR reading is not implemented yet.\n");
        }
        else{
            writeToFile(fileName, result, ressize);
            // printf("%.*s\n", (int)ressize, result);
        }

        XFree(result);
        return True;
    }
    else{ // request failed, e.g. owner can't convert to the target format
        return False;
    }
}

// https://stackoverflow.com/a/44992967/14621024
void WatchSelection(Display *display, Window window, const char *bufname){
    int event_base, error_base;
    XEvent event;
    Atom bufid = XInternAtom(display, bufname, False);

    assert( XFixesQueryExtension(display, &event_base, &error_base) );
    XFixesSelectSelectionInput(display, DefaultRootWindow(display), bufid, XFixesSetSelectionOwnerNotifyMask);

    while (True){
        XNextEvent(display, &event);

        if(event.type == event_base + XFixesSelectionNotify && ((XFixesSelectionNotifyEvent*)&event)->selection == bufid){
            if(!PrintSelection(display, window, bufname, "UTF8_STRING")){
                PrintSelection(display, window, bufname, "STRING");
            }

            fflush(stdout);
        }
    }
}

int main(){
    // FILE *fp;
    // char *fileName = "/tmp/test";

    printf("Starting\n");
    fflush(stdout);

    Display *display = XOpenDisplay(NULL);
    unsigned long color = BlackPixel(display, DefaultScreen(display));
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0,0, 1,1, 0, color, color);
    // Bool result = PrintSelection(display, window, "CLIPBOARD", "UTF8_STRING") ||
    // PrintSelection(display, window, "CLIPBOARD", "STRING");

    WatchSelection(display, window, "CLIPBOARD");
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
