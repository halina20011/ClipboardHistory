// gcc main.c -o ./Build/main -lX11 -lXfixes && ./Build/main

#include <stdio.h>
#include <limits.h>

// https://www.x.org/releases/current/doc/index.html
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>

#include <assert.h>

#include "file.c"

char *fileName = "/tmp/clipboardText";

#define EVER ;;

Bool printSelection(Display *display, Window window, const char *bufname, const char *fmtname){
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
            char *dataToCopy = malloc(sizeof(char) * (ressize + 1));
            memcpy(dataToCopy, result, ressize);
            dataToCopy[ressize] = '\0';
            unsigned int dataToCopySize = ressize;

            // Delete all spaces before first letter
            if(result[0] == ' '){
                printf("Removing spaces before\n");
                char *pos = result;
                int numberOfSpaces = 0;
                for(int i = 0; i < ressize; i++){
                    if(*pos++ == ' '){
                        numberOfSpaces += 1;
                        // printf("On index %i is space\n", i);
                    }
                    else{
                        break;
                    }
                }

                dataToCopySize = ressize - numberOfSpaces;
                dataToCopy = (char *)realloc(dataToCopy, sizeof(char) * (dataToCopySize + 1));
                memcpy(dataToCopy, result + numberOfSpaces, dataToCopySize);
                dataToCopy[dataToCopySize] = '\0';
                // printf("Data without spaces: >%s<\n", dataToCopy);
            }

            // If last char is new line then delete it
            if(dataToCopy[dataToCopySize - 1] == '\n'){
                printf("Removing new line\n");
                dataToCopySize = (dataToCopySize - 1);
                dataToCopy = (char *)realloc(dataToCopy, sizeof(char) * (dataToCopySize + 1)); 

                dataToCopy[dataToCopySize] = '\0';
            }

            writeToFile(fileName, dataToCopy, dataToCopySize);
            free(dataToCopy);
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
void watchSelection(Display *display, Window window, const char *bufname){
    int event_base, error_base;
    XEvent event;
    Atom bufid = XInternAtom(display, bufname, False);

    assert(XFixesQueryExtension(display, &event_base, &error_base));
    XFixesSelectSelectionInput(display, DefaultRootWindow(display), bufid, XFixesSetSelectionOwnerNotifyMask);
    
    for(EVER){
        XNextEvent(display, &event);

        if(event.type == event_base + XFixesSelectionNotify && ((XFixesSelectionNotifyEvent*)&event)->selection == bufid){
            if(!printSelection(display, window, bufname, "UTF8_STRING")){
                printSelection(display, window, bufname, "STRING");
            }

            fflush(stdout);
        }
    }
}

int main(){
    // Create file if it doesn't exist
    if(fileExists(fileName) != 0){
        printf("%d File \"%s\"coudn't be created\n", __FILE__, fileName);
        exit(1);
    }
    printf("Started\n");
    fflush(stdout);

    Display *display = XOpenDisplay(NULL);
    unsigned long color = BlackPixel(display, DefaultScreen(display));
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0,0, 1,1, 0, color, color);

    watchSelection(display, window, "CLIPBOARD");
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
