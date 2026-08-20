LIBS := sdl3
BACKENDS := sdl
ROMS := smb3

ENGINES := $(foreach b,$(BACKENDS),build/$(b))
ROMSS := $(foreach b,$(ROMS),build/roms/$(b).so)

CFLAGS := $(shell pkg-config --libs --cflags sdl3)

DEBUG := -g -O0

OBJS := src/audio.c src/engine.c src/graphics.c src/collision.c

all: engines roms

build/sdl: $(OBJS) src/backends/sdl.c
	$(CC)  $(DEBUG) $^ -o $@ -rdynamic $(CFLAGS)

build/roms/%.so:
	mkdir -p build/roms
	$(MAKE) -C roms/$* BUILDDIR=../../build/roms

clean:
	rm build/sdl build/roms/*

engines: $(ENGINES)
roms: $(ROMSS)

.PHONY: engine/%
engine/%: build/%

.PHONY: roms/%
roms/%: build/roms/%.so

