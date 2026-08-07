# Architecture

## Engine
- Deals with core game independent routines, like:
    - Framerate locking (TODO)
    - Common rom selection UI

## Backend
- Fullfill backend.h API
- Along with engine, compiles into a platform specific target
    - In case of desktop yield a executable which loads roms from a specific directory
    - In case of embedded targets have a make target which flashes the microcontroller

## Roms
- Fullfill rom.h API

## Helper lib
- Refactor reusable code into a helper lib, like collision, camera, map streaming
- Define sane default formats, like map format, audio data format, metasprites

## Tooling
- Scripts/programs to compile popular formats (like Tiled maps) into binary formats parseable by helper libs.

## Build
- A main makefile which builds/flashes a specific target, has targets for roms, defines the rom directory structure like process rom/assets/maps/ or other paths by tooling for supported filetypes.

