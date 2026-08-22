#include "graphics.h"
#include "engine.h"
#include "sprite.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void load_tileset(void *_data) {
    uint8_t *data = _data;
    engine.tileset.data = _data;
}

void clear_pixelbuf() { memset(engine.pixelbuf, 0, sizeof(engine.pixelbuf)); }

void draw_tile(uint16_t tile, int16_t y, int16_t x, uint8_t palette,
               uint8_t attributes) {
    if (x < 0 || x >= X_RESOLUTION || y < 0 || y >= Y_RESOLUTION) {
        return;
    }

    if (tile & 1 << 15)
        attributes ^= FLIP_X;

    if (tile & 1 << 14)
        attributes ^= FLIP_Y;

    tile &= 0b0011111111111111;

    if (!tile) {
        fprintf(stderr, "drawing not tile\n");
    }

    // size_t offset = (size_t)tile * TILE_SIZE * BPP;
    // fprintf(stderr, "%zu\n", (size_t)tile * 16);

    // const uint8_t* lower = engine.tileset.data + offset;
    const uint8_t *lower = &engine.tileset.data[tile * TILE_SIZE * BPP];
    const uint8_t *higher = lower + TILE_SIZE;

    for (int row = 0; row < TILE_SIZE; row++) {
        if (y + row >= Y_RESOLUTION) {
            return;
        }

        for (int col = 0; col < TILE_SIZE; col++) {
            if (x + col >= X_RESOLUTION) {
                continue;
            }

            uint8_t ylower;
            uint8_t yhigher;

            if (attributes & FLIP_Y) {
                ylower = lower[TILE_SIZE - row - 1];
                yhigher = higher[TILE_SIZE - row - 1];
            } else {
                ylower = lower[row];
                yhigher = higher[row];
            }

            uint8_t index;
            if (attributes & FLIP_X)
                index = (ylower >> col & 1u) | ((yhigher >> col & 1u) << 1);
            else
                index = (ylower >> (7 - col) & 1u) |
                        ((yhigher >> (7 - col) & 1u) << 1);

            if (index)
                engine.pixelbuf[y + row][x + col] =
                    index + palette * PALETTE_SIZE;
        }
    }
}

void draw_chars(const char *text, fnt font, uint8_t y, uint8_t x,
                uint8_t palette) {
    uint8_t c;
    while ((c = *text)) {
        if (font[c]) {
            draw_tile(font[c], y * TILE_SIZE, x, palette, 0);
        }
        x += TILE_SIZE;
        text++;
    }
}

void draw_text(const char *text, struct font *font, uint16_t y, uint16_t x,
               uint8_t palette) {
    while (*text) {
        uint8_t c = (*text) - 'A';
        draw_tile(font->offset + c, y, x, palette, 0);
        x += TILE_SIZE;
        text++;
    }
}

void draw_sprite(struct sprite *sprite) {
    // Add flips
    const struct texture *texture = sprite->texture;
    uint8_t num_tiles = texture->height * texture->width;

    uint8_t attributes = sprite->attributes;
    uint8_t tile =
        (sprite->frame % sprite->texture->num_frames) *
        texture->width * texture->height;

    if (attributes & FLIP_Y) {
        for (int row = texture->height; row > 0; --row) { //This is likely a bug unless your sprite coordinate convention intentionally defines sprite->y differently.
            if (attributes & FLIP_X) {
                for (int column = texture->width; column > 0; --column) {
                    draw_tile(texture->tiles[tile], sprite->y + row * TILE_SIZE,
                              sprite->x + column * TILE_SIZE, sprite->palette,
                              sprite->attributes);
                    tile++;
                }
            } else {
                for (int column = 0; column < texture->width; column++) {
                    draw_tile(texture->tiles[tile], sprite->y + row * TILE_SIZE,
                              sprite->x + column * TILE_SIZE, sprite->palette,
                              sprite->attributes);
                    tile++;
                }
            }
        }
    } else {
        for (int row = 0; row < texture->height; row++) {
            if (attributes & FLIP_X) {
                for (int column = texture->width - 1; column >= 0; --column) {
                    draw_tile(texture->tiles[tile], sprite->y + row * TILE_SIZE,
                              sprite->x + column * TILE_SIZE, sprite->palette,
                              sprite->attributes);
                    tile++;
                }
            } else {
                for (int column = 0; column < texture->width; column++) {
                    draw_tile(texture->tiles[tile], sprite->y + row * TILE_SIZE,
                              sprite->x + column * TILE_SIZE, sprite->palette,
                              sprite->attributes);
                    tile++;
                }
            }
        }
    }
}

void next_frame(struct sprite *sprite) {
    sprite->frame = (sprite->frame + 1) % sprite->texture->num_frames;
}

