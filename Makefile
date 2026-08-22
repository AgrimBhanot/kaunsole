LIBS := sdl3
BACKENDS := sdl
ROMS := smb3

ENGINES := $(foreach b,$(BACKENDS),build/$(b))
ROMSS := $(foreach b,$(ROMS),build/roms/$(b).so)

# Separate flags properly: --cflags for compilation, --libs for linking
CFLAGS := $(shell pkg-config --cflags sdl3)
LDFLAGS := $(shell pkg-config --libs sdl3)

DEBUG := -g -O0

OBJS := src/audio.c src/engine.c src/graphics.c

all: engines roms

build/sdl: $(OBJS) src/backends/sdl.c
	$(CC)  $(DEBUG) $^ -o $@ -rdynamic $(CFLAGS) $(LDFLAGS)

build/roms/%.so:
	mkdir -p build/roms
	$(MAKE) -C roms/$* BUILDDIR=../../build/roms

clean:
	rm -rf build/*

engines: $(ENGINES)
roms: $(ROMSS)

.PHONY: engine/%
engine/%: build/%

.PHONY: roms/%
roms/%: build/roms/%.so