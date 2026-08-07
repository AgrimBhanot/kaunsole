#pragma once

#include <stdbool.h>
#include <stdint.h>

#define X_RESOLUTION 256
#define Y_RESOLUTION 256
// #define X_RESOLUTION 320
// #define Y_RESOLUTION 240

#define TILE_SIZE 8
#define BLOCK_SIZE (2 * TILE_SIZE)
#define BPP 2
#define PALETTE_SIZE (1 << BPP)
#define ROW_OFFSET (TILESET_SIZE * BPP / 8)

struct render_command {
    uint8_t priority;
};

struct texture {
    uint8_t width;
    uint8_t height;
    uint8_t num_frames;
    const uint16_t *tiles;
};

struct hitbox {
    uint8_t x;
    uint8_t y;
    uint8_t height;
    uint8_t width;
};

struct sprite {
    const struct texture *texture;
    struct hitbox hitbox;
    uint8_t frame;
    uint8_t attributes;
    uint8_t palette;
    uint8_t y;
    uint8_t x;
    int8_t falling;
    uint8_t screen;
};

struct tileset {
    const uint8_t *data;
};

struct font {
    uint16_t offset;
};

typedef uint16_t *fnt;

void load_tileset(void *);

void draw_text(const char *text, struct font *font, uint16_t y, uint16_t x,
               uint8_t palette);
void draw_chars(const char *text, fnt font, uint8_t y, uint8_t x,
                uint8_t palette);
void draw_sprite(struct sprite *);
void next_frame(struct sprite *sprite);
void draw_tile(uint16_t tile, uint16_t y, uint16_t x, uint8_t palette,
               uint8_t attributes);
void clear_pixelbuf();
