# # Safely find assets if the directory exists
# ASSET_SRC := $(shell find assets -type f 2>/dev/null)
# ASSET_OBJ := $(ASSET_SRC:assets/%=build/assets/%.o)

# SRCS := main.c camera.c
# BUILDDIR := .

# # The target matches what the main root Makefile expects: a shared object (.so)
# $(BUILDDIR)/smb3.so: $(SRCS) $(ASSET_OBJ)
# 	mkdir -p build/assets
# 	$(CC) $(SRCS) $(ASSET_OBJ) -fPIC -shared -Wl,-z,noexecstack -o $@

# # Rule to compile binary assets using objcopy (for things like graphics/chr ROMs)
# build/assets/%.o: assets/%
# 	mkdir -p $(dir $@)
# 	objcopy -I binary -O elf64-x86-64 -B i386 $< $@
# 	build/assets/%.o: assets/%
# #	cd assets && objcopy -I binary -O elf64-x86-64 -B i386 $(notdir $<) ../$@
# clean:
# 	rm -rf build smb3.so *.o

# Safely find assets if the directory exists
# Safely find assets if the directory exists
ASSET_SRC := $(shell find assets -type f 2>/dev/null)
ASSET_OBJ := $(ASSET_SRC:assets/%=build/assets/%.o)

SRCS := main.c camera.c collision.c
BUILDDIR := .

# The target matches what the main root Makefile expects: a shared object (.so)
$(BUILDDIR)/smb3.so: $(SRCS) $(ASSET_OBJ)
	mkdir -p build/assets
	$(CC) $(SRCS) $(ASSET_OBJ) -fPIC -shared -Wl,-z,noexecstack -o $@

# Rule to compile binary assets using objcopy cleanly from the assets directory
build/assets/%.o: assets/%
	mkdir -p $(dir $@)
	cd assets && objcopy -I binary -O elf64-x86-64 -B i386 $(notdir $<) ../$@

clean:
	rm -rf build smb3.so *.o