#!/usr/bin/env zsh

gcc main.c -Iraylib-5.0_macos/include -Lraylib-5.0_macos/lib -lraylib -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
