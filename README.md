# FreqDisp
## Overview
FreqDisp is a lightweight C application that visualises frequency inputs using a graphical window. This tool provides a simple, visual alternative to configuring an Arduino for frequency visualisation, making it easier to experiment with frequency and duty cycle settings.

## Requirements
- C compiler (e.g., gcc)
- Raylib library
- RayGUI library (included in the code as #define RAYGUI_IMPLEMENTATION)
- X11 server (for running on WSL)

## Compilation (if Raylib + RayGUI Installed)
```bash
make
```

## Running
```bash
./freqdisp
```