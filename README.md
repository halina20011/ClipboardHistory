# Clipboard History
Simple c program with which you can access text before that was copied before

## Building
x11Window.c &emsp; ```gcc x11Window.c -L/usr/X11R6/lib -lX11 -o ./Build/x11Window```</br>
main.c &emsp;&emsp; ```gcc main.c -o ./Build/main -lX11 -lXfixes```</br>

## Setup 
Run `main` for capturing copied text: ```./Build/main``` </br>
Add keybinding in your "tiling window manager" or "desktop environment" for execute x11Window </br>

## Preview
![Preview](/Images/preview.png)

## Control
`Arrow up`              move through clipboard history one up </br>
`Arrow down`            move through clipboard history one down </br>
`Esc` or `q`            exit without anything to be copied </br>
`Enter`                 copy selected text and exit </br>
`Del` or `Backspace`    delete selected text from clipboard history </br>

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
add list of programs from witch main.c should ignore copy </br>
text to be in UTF-8 </br>
add image support </br>
remove the need of `xclip` </br>
demonize `main.c` </br>

## Acknowledgment
This programm usses for base64 encoding `base.c` that is under BSD license </br>
http://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.c </br>

## References
X11Lib Docs                                 https://www.x.org/releases/current/doc/libX11/libX11/libX11.html </br>
Benchmark results Results for base64:       https://github.com/gaspardpetit/base64/ </br>
better syntax understanding:                https://github.com/QMonkey/Xlib-demo </br>
