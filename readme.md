# OBJ File Parser

This is an `.obj` file parser

## Features

- Vertices
- Normals
- Texture coordinates
- Triangulated faces
- Interleaved vertex buffer
- Materials

Check `objparser.h` for context fields

## How to use

Check `main.c` for usage example.

## How to build

This project does not contain any dependencies and does not rely on any additional tools. `objparser.c` file contains code that you can use as a library. If you compile it with `main.c` you can obtain a demo file that parses provided `.obj` file and prints its contents.

If you want to use CMake:

```bash
cmake -B build -S .
cmake --build build
```

You can use the program like this:

```bash
objparser [file.obj]    # replace file.obj with path to .obj file
```