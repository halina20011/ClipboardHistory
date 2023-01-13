# Clipboard History
This is my first "Vanilla" C program so there will be a lot of things to improve.

## In development
I am currently still working on it. 
This is more archive commit for when it still works. 
It has some bugs on witch I am working towards removing them.

## Building
x11Window.c     ```gcc x11Window.c -L/usr/X11R6/lib -lX11 -o ./Build/x11Window```</br>
main.c          ```gcc main.c -o ./Build/main -lX11 -lXfixes```</br>

## Setup 
make two new files: `clipboardText` and `tmpClipboardText` in /tmp
```
touch /tmp/clipboardText
touch /tmp/tmpClipboardText
```

## How it works
This are two programs that are responsible for this to work:
main.c:         program that listens when if copied text hrow x11lib
                it saves all data in file `/tmp/clipboardText` in specific format (see below):

x11Window.c     program that uses x11lib for creating window with witch you can select what text you want to be copied to clipboard
                text that is selected "Enter pressed" is then written to file `/tmp/tmpClipboardText` and then copied with xclip to clipboard

## Storage syntax
```[sizeOfText]:[base64TextEncoted]```

## Dependencies
xclip

## Todo
Fix a lot of bugs </br>
remove the need of `os.h` </br>
remove the need of `xclip` </br>
demonize `main.c` </br>
add text wrap </br>
add image support </br>

## Acknowledgment
This programm usses for base64 encoding `base.c` and `os.o` that are under BSD license
http://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.c
http://web.mit.edu/freebsd/head/contrib/wpa/src/utils/os.o

## References
X11Lib Docs                                 https://www.x.org/releases/current/doc/libX11/libX11/libX11.html
Benchmark results Results for base64:       https://github.com/gaspardpetit/base64/
better syntax understanding:                https://github.com/QMonkey/Xlib-demo
