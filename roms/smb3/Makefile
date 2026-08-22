ASSET_SRC := $(shell find assets -type f)
ASSET_OBJ := $(ASSET_SRC:assets/%=build/assets/%.o)
SRCS := main.c camera.c enemy.c enemy_types.c collision.c
BUILDDIR := .

$(BUILDDIR)/smb3: $(SRCS) $(ASSETS)
	$(CC) $(ASSET_SRC) -fPIC -shared $^ -Wl,-z,noexecstack -o $@

clean:
	rm game *.o