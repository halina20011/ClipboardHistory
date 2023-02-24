// gcc x11Window.c -L/usr/X11R6/lib -lX11 -o ./Build/x11Window && ./Build/x11Window

// https://www.x.org/releases/current/doc/index.html
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdio.h>          // *print*
#include <ctype.h>          // isprint

#include "file.c"

unsigned int wWidth = 300;
unsigned int wHeight = 400;
unsigned int windowCound = 10;

char *fileName = "/tmp/clipboardText";
char *tmpClipboardText = "/tmp/tmpClipboardText";

// Minimum time[ms] that is between press of del key
const unsigned int charPressSleep = 200;

unsigned int selectIndex = 0;
unsigned int dataCount = 0;

const unsigned int keyUp    =   65362;
const unsigned int keyDown  =   65364;
const unsigned int enter    =   65293;
const unsigned int q        =   113;
const unsigned int esc      =   65307;
const unsigned int del      =   65535;
const unsigned int back     =   65288;

typedef struct {
    int x;
    int y;
} Vec2;

unsigned long toRGB(int r,int g, int b){
    return b + (g << 8) + (r << 16);
}

//                                          {Background color, Text color}
//                                          (28, 28, 28),   (255, 255, 255)
const unsigned long backgroundColor[2]  =   {1842204,       16777215};
//                                          (0, 85, 119)    (0, 0, 0)
const unsigned long selectColor[2]      =   {21879,         0};

// https://stackoverflow.com/a/3591679/14621024
Vec2 getMousePosition(Display *display){
    Vec2 mousePos = {-1, -1};
    int numberOfScreens;
    Bool result;
    Window *rootWindows;
    Window windowReturned;
    int rootX, rootY;
    int winX, winY;
    unsigned int maskReturn;

    numberOfScreens = XScreenCount(display);
    fprintf(stderr, "There are %d screens available in this X session\n", numberOfScreens);
    rootWindows = malloc(sizeof(Window) * numberOfScreens);

    for(int i = 0; i < numberOfScreens; i++){
        rootWindows[i] = XRootWindow(display, i);
    }

    for(int i = 0; i < numberOfScreens; i++){
        result = XQueryPointer(display, rootWindows[i], &windowReturned, &windowReturned, &rootX, &rootY, &winX, &winY, &maskReturn);
        if(result == True){
            break;
        }
    }

    if(result != True){
        fprintf(stderr, "Mouse wasn't found.\n");
        return mousePos;
    }

    mousePos.x = rootX;
    mousePos.y = rootY;
    printf("Mouse position: (%d, %d)\n", rootX, rootY);

    free(rootWindows);
    return mousePos;
}

const int fontWidth = 6;
const int fontHeight = 18;

const int textOffset = 5;

// This function is useless for now
Vec2 calculateHeight(int width, const int offsetX, const int offsetY, unsigned int dataLength){
    int height;

    Vec2 size = {dataLength * fontWidth, fontHeight};

    int length = dataLength * fontWidth;
    float scale = (float)fontWidth / (float)(width - offsetX * 2);
    // printf("Scale: %f\n", scale);

    if(scale < 1){
        return size;
    }
    
    // Calculate height
    size.y = ((int)scale + 1) * fontHeight;

    return size;
}

const int offsetX = 10;
const int offsetY = 5;

int drawContent(Display *display, Window window, int screenN, GC gc){
    XClearWindow(display, window);
    unsigned int *dataLength;
    unsigned int dataLengthCount = 0;
    
    printf("Reading file\n");
    dataLength = readFile(fileName, &dataLengthCount);
    if(dataLength == NULL){
        printf("Error when reading file size on line: %d\n", __FILE__);
        return 1;
    }

    // printA(dataLength, dataLengthCount);

    dataCount = dataLengthCount;

    int heightOffset = fontHeight;
    
    // Calculate what is maximum text size that fits in to one line
    unsigned int maxTextLength = (unsigned int)((wWidth - (offsetX * 2)) / fontWidth);
    unsigned int windowFitHeight = wHeight / (fontHeight * 2);
    // printf("Fit in w: %u\n", windowFitHeight);
    // printf("Max text length: %u\n", maxTextLength);

    unsigned int index = selectIndex % windowFitHeight;
    unsigned int indexFrom = ((int)selectIndex / windowFitHeight) * windowFitHeight;
    printf("Index from: %u Index: %u\n", indexFrom, index);

    for(int i = indexFrom; i < dataLengthCount; i++){
        // Calculate correct index in file, because it is looped from back
        int fileIndex = dataLengthCount - 1 - i;

        unsigned char *decodedText;
        size_t decodedTextSize;

        decodedText = getData(fileName, dataLength, dataLengthCount, fileIndex, &decodedTextSize);
        // printf("%zu\n", decodedTextSize);
        // printf("Decoded data{%zu}: >%s<\n", decodedTextSize, decodedText);
        if(decodedText == NULL){
            printf("Error when decoding text from file on line: %d\n", __FILE__);
            return 1;
        }
        
        Vec2 size = calculateHeight(wWidth, offsetY, offsetX, decodedTextSize);
        
        unsigned char *clipboardText;
        unsigned int clipboardTextSize;
        
        if(maxTextLength < decodedTextSize){
            clipboardText = malloc(sizeof(char) * (maxTextLength + 1));
            clipboardTextSize = maxTextLength;
        }
        else{
            clipboardText = malloc(sizeof(char) * (decodedTextSize + 1));
            clipboardTextSize = decodedTextSize;
        }

        memcpy(clipboardText, decodedText, clipboardTextSize);
        clipboardText[clipboardTextSize] = '\0';
        
        // printf("X: %i, Y: %i\n", size.x , size.y);
        if(i == selectIndex){
            XSetForeground(display, gc, selectColor[0]);
            XFillRectangle(display, window, gc, 0, heightOffset - size.y, wWidth, size.y * 2);
            XSetForeground(display, gc, selectColor[1]);
            XDrawString(display, window, gc,    offsetX, heightOffset,  clipboardText, clipboardTextSize);
            // printf("Text printed on screen{%u}: >%s<\n", clipboardTextSize, clipboardText);
        }
        else{
            XSetForeground(display, gc, backgroundColor[1]);
            XDrawString(display, window, gc,    offsetX, heightOffset, clipboardText, clipboardTextSize);
        }
        heightOffset += size.y * 2;
    
        free(clipboardText);
        free(decodedText);
    }

    free(dataLength);

    fflush(stdout);

    return 0;
}

int copyText(){
    unsigned int *dataLength;
    unsigned int dataLengthCount;
    
    dataLength = readFile(fileName, &dataLengthCount);
    if(dataLength == NULL){
        return 1;
    }

    unsigned char *decodedText;
    size_t decodedTextSize;
    decodedText = getData(fileName, dataLength, dataLengthCount, dataLengthCount - 1 - selectIndex, &decodedTextSize);
    if(decodedText == NULL){
        return 1;
    }
    
    // Write text to file that will be copied with xclip
    writeToTmpFile(tmpClipboardText, decodedText);

    free(dataLength);
    free(decodedText);

    // Copy text from file to clipboard with xclip
    char command[50];
    sprintf(command, "cat %s | xclip -i -sel clip", tmpClipboardText);
    // printf("Command: %s\n", command);
    system(command);
    return 0;
}

int main(void){
    // Create file if it doesn't exist
    if(fileExists(fileName) != 0){
        printf("%d File \"%s\"coudn't be created\n", __FILE__, fileName);
        exit(1);
    }
    if(fileExists(tmpClipboardText) != 0){
        printf("%d File \"%s\"coudn't be created\n", __FILE__, tmpClipboardText);
        exit(1);
    }

    Display *display;
    Window window;
    XEvent event;
    XEvent lastEvent;
    XComposeStatus comp;
    KeySym ks;
    
    // Store Xlib return value
    Status rc;

    int screenNum;

    char buf[17];
    int keyCode, len;

    // graphics context
    GC gc;

    display = XOpenDisplay(NULL);
    if(display == NULL){
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    Vec2 mousePos = {-1, -1};
    mousePos = getMousePosition(display);
    if(mousePos.x < 0 || mousePos.y < 0){
        printf("Negative mouse position exited on line: %d\n", __FILE__);
        exit(1);
    }

    screenNum = DefaultScreen(display);
    window = XCreateSimpleWindow(display, RootWindow(display, screenNum), mousePos.x, mousePos.y, wWidth, wHeight, 0, BlackPixel(display, screenNum), backgroundColor[0]);

    XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask);
    XStoreName(display, window, "Clipboard history");
    XSizeHints *sh = XAllocSizeHints();
    sh->flags = PMinSize | PMaxSize;
    sh->min_width = sh->max_width = wWidth;
    sh->min_height = sh->max_height = wHeight;
    XSetWMNormalHints(display, window, sh);
    XMapWindow(display, window);
    
    gc = DefaultGC(display, screenNum);

    printf("Window was created\n");

    while(1){
        XNextEvent(display, &event);
        //printf((char)event.type);
        if(event.type == Expose){
            printf("Drawing content\n");
            drawContent(display, window, screenNum, gc);
            printf("Content was drawn\n");
        }
        if(event.type == KeyPress){
            // printf("Key was pressed\n");
            len = XLookupString(&event.xkey, buf, 16, &ks, &comp);
            keyCode = (int)ks;
            if(len > 0 && isprint(buf[0])){
                buf[len] = 0;
                printf("Key:    %s, ", buf);
            }
            printf("Key code:   %d\n", keyCode);

            // Move selectIndex
            if(keyCode == keyUp || keyCode == keyDown){
                if(keyCode == keyUp){ // Key up
                    if(selectIndex == 0){
                        selectIndex = dataCount - 1;
                    }
                    else{
                        selectIndex -= 1;
                    }
                }
                else{ // Key down
                    if(selectIndex == dataCount - 1){
                       selectIndex = 0;
                    }
                    else{
                        selectIndex += 1;
                    }
                }
                // printf("Select index: %u\n", selectIndex);
                drawContent(display, window, screenNum, gc);
            }
            else if(keyCode == enter){
                copyText();
                break;
            }
            else if(keyCode == del || keyCode == back && XEventsQueued(display, QueuedAfterReading)){
                // printf("%i\n", event.xkey.time - lastEvent.xkey.time);
                // Check if key was released by checking if there was at least [charPressSleep] pause between last press and the current one
                if((event.xkey.time - lastEvent.xkey.time) > charPressSleep){
                    int selIndex = dataCount - 1 - selectIndex;
                    deleteLine(fileName, selIndex);
                    drawContent(display, window, screenNum, gc);
                }
            }
            else if(keyCode == q || keyCode == esc){
                printf("Exiting\n");
                break;
            }
            lastEvent = event;
        }
    }

    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
