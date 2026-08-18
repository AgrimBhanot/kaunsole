#include "camera.h"
#include "tile.h"
#include "obj.h"
#include <stdbool.h>
#include <stdio.h>

#include "defs.h"

#include "../../src/graphics.h"
// MAP

extern uint16_t font[256];

const uint16_t blocks[256][4] = {
    { TILE(0, 3, 0), TILE(0, 3, 0), TILE(0, 3, 0), TILE(0, 3, 0) }, /* blank */
    { TILE(6, 5, 3), TILE(6, 5, 4), TILE(6, 5, 5), TILE(6, 5, 6) }, /* xbox */
    { TILE(6, 5, 7), TILE(6, 5, 7), TILE(6, 5, 8), TILE(6, 5, 8) }, /* pillar */
    { TILE(1, 17, 6), TILE(1, 17, 7), TILE(1, 18, 6), TILE(1, 18, 7) }, /* grass_tl */
    { TILE(1, 17, 8), TILE(1, 17, 9), TILE(1, 18, 8), TILE(1, 18, 9) }, /* grass_tm */
    { TILE(1, 17, 10), TILE(1, 17, 11), TILE(1, 18, 10), TILE(1, 18, 11) }, /* grass_tr */
    { TILE(1, 18, 6), TILE(1, 18, 7), TILE(1, 18, 6), TILE(1, 18, 7) }, /* grass_ml */
    { TILE(1, 18, 8), TILE(1, 18, 9), TILE(1, 18, 8), TILE(1, 18, 9) }, /* grass_mm */
    { TILE(1, 18, 10), TILE(1, 18, 11), TILE(1, 18, 10), TILE(1, 18, 11) }, /* grass_mr */
    { TILE(1, 16, 0), TILE(1, 17, 0), TILE(1, 16, 1), TILE(1, 17, 1) }, /* hwood_tl */
    { TILE(1, 16, 1), TILE(1, 17, 1), TILE(1, 16, 1), TILE(1, 17, 1) }, /* hwood_tm */
    { TILE(1, 16, 1), TILE(1, 17, 1), TILE(1, 16, 2), TILE(1, 17, 2) }, /* hwood_tr */
    { TILE(1, 18, 0), TILE(1, 19, 0), TILE(1, 18, 1), TILE(1, 19, 1) }, /* hwood_bl */
    { TILE(1, 18, 1), TILE(1, 19, 1), TILE(1, 18, 1), TILE(1, 19, 1) }, /* hwood_bm */
    { TILE(1, 18, 1), TILE(1, 19, 1), TILE(1, 18, 2), TILE(1, 19, 2) }, /* hwood_br */
};

const struct obj1d objs1d[256] = {
    (struct obj1d) /* O_XBLOCK */ {
        .start = 1,
        .flags = O1_MIDDLE,
    },
    (struct obj1d) /* O_PILLAR */ {
        .start = 2,
        .flags = O1_TERMINAL,
    },
    (struct obj1d) /* O_GRASS_TOP */{
        .start = 3,
        .flags = O1_TERMINAL | O1_MIDDLE,
    },
    (struct obj1d) /* O_GRASS_MIDDLE */{
        .start = 6,
        .flags = O1_TERMINAL | O1_MIDDLE,
    },
    (struct obj1d) /* O_GRASS_END */{
        .start = 6,
        .flags = O1_TERMINAL | O1_MIDDLE,
    },
    (struct obj1d) /* O_WOOD_TOP */{
        .start = 9,
        .flags = O1_TERMINAL | O1_MIDDLE,
    },
    (struct obj1d) /* O_WOOD_BOTTOM */{
        .start = 12,
        .flags = O1_TERMINAL | O1_MIDDLE,
    },
    (struct obj1d) /* O_WOOD_BOTTOM */{
        .start = 12,
        .flags = O1_TERMINAL | O1_MIDDLE,
    },
};

const struct obj2d objs2d[256] = {
    (struct obj2d) {
        // PILLAR PLATFORM
        .start = (uint16_t []) { O1(0, 0), O1(1, 8), O1(1, 4) },
        // .start = (uint16_t []) { O1_INDEX(0) | O1_X(0), O1_INDEX(1) | O1_X(8), O1_INDEX(1) | O1_X(4)},
        .flags = O2_VERTICAL | O2_TERMINAL,
    },
    (struct obj2d) {
        // PILLAR PLATFORM
        .start = (uint16_t []) { O1(2, 0), O1(3, 0), O1(3, 0)},
        // .start = (uint16_t []) { O1_INDEX(2) | O1_X(0), O1_INDEX(3) | O1_X(0), O1_INDEX(3) | O1_X(0)},
        .flags = O2_VERTICAL | O2_TERMINAL,
    },
    (struct obj2d) {
        // HWOOD
        .start = (uint16_t []) { O1(5, 0), O1(6, 0) },
        // .start = (uint16_t []) { O1_INDEX(2) | O1_X(0), O1_INDEX(3) | O1_X(0), O1_INDEX(3) | O1_X(0)},
        .flags = O2_VERTICAL | O2_TERMINAL | O2_MIDDLE,
    },
};

#define N_SCREENS 8

uint16_t map_x;
uint8_t screen_x = 0;
const uint32_t *active_map;

uint8_t active_screen;
uint8_t read_offset;
uint8_t write_offset;

uint16_t block_buffer[N_SCREENS][16][16];

void stream_screen(bool reverse) {
    uint32_t o;
    uint8_t target_screen = reverse ? read_offset : write_offset;

    while ((o = *active_map)) {

        const uint8_t y = O2_Y_GET(o);
        const uint8_t x = O2_X_GET(o);
        const uint8_t height = O2_HEIGHT_GET(o); 
        const uint8_t width = O2_WIDTH_GET(o);
        const uint8_t palette = O2_PALETTE_GET(o);

        const struct obj2d obj = objs2d[O2_INDEX_GET(o)];
        const bool terminal = obj.flags & O2_TERMINAL;
        const bool middle = obj.flags & O2_MIDDLE;
        const bool vertical = obj.flags & O2_VERTICAL;

        fprintf(stderr, "o2: %b terminal %032b\n", o, terminal);

        uint16_t o1;
        uint8_t o1_offset = 0;
        for (uint8_t i = 0; i < height; i++) {
            if (terminal && (i == 0)) {
                o1 = obj.start[o1_offset];
                fprintf(stderr, "\tstarting o1 %u: %016b\n", o1_offset, o1);
                o1_offset++;
            } else if (terminal && (i == height - 1)) {
                if (middle && height > 2)
                    o1_offset++;
                o1 = obj.start[o1_offset];
                fprintf(stderr, "\tending o1 %u: %016b\n", o1_offset, o1);
            } else {
                o1 = obj.start[o1_offset];
                fprintf(stderr, "\tmiddle o1 %u: %016b\n", o1_offset, o1);
            }

            // draw o1
            const struct obj1d obj1 = objs1d[O1_INDEX(o1)];
            uint8_t o1_x = (O1_X_GET(o1) * width) >> 4;
            uint8_t o1_index = O1_INDEX_GET(o1);

            bool o1_terminal = obj1.flags & O1_TERMINAL;
            bool o1_middle = obj1.flags & O1_MIDDLE;

            bool cont = true;

            for (uint8_t j = 0; j < width && cont; j++) {
                uint8_t block;
                if (j == 0) {
                    block = obj1.start;
                    fprintf(stderr, "\t\tstart block %u: %u\n", j, block);
                    if (!o1_middle)
                        cont = false;
                } else if (j == width - 1) {
                    block = obj1.start + (o1_terminal ? 1 : 0) + (middle ? 1 : 0);
                    fprintf(stderr, "\t\tend block %u: %u\n", j, block);
                } else {
                    block = obj1.start + (o1_terminal ? 1 : 0);
                    fprintf(stderr, "\t\tmiddle block %u: %u\n", j, block);
                }

                if (vertical) {
                    block_buffer[target_screen][i + y][j + x + o1_x] = block;
                } else {
                    block_buffer[target_screen][j + x + o1_x][i + y] = block;
                }
            }
        }

        active_map++;
        if (o & O2_NEXT_PAGE)
            break;
    }

    if (reverse) {
        read_offset = (read_offset - 1 + N_SCREENS) % N_SCREENS;
    } else {
        write_offset = (write_offset + 1) % N_SCREENS;
    }

}

void stream_map(int16_t x) {
    map_x += x;
    bool loading = true;
    while (loading) {
        active_map++;
        const uint32_t obj = *active_map;
        if (obj & O2_NEXT_PAGE) {
            screen_x++;
            loading = false;
        }
    }
}

void load_map(const uint32_t *m) {
    active_map = m;
    map_x = 0;
    stream_screen(0);
    stream_screen(0);
}

// CAMERA

uint8_t camera_x = 0;
uint8_t camera_y = 0;

//COLLISION FUNCTION
uint16_t solid_at(int16_t draw_x, int16_t draw_y) {
    int32_t raw_x = draw_x + camera_x;
    uint8_t screen;
    uint8_t bx;

    if (raw_x < 0) {
        // left of active_screen's visible window — not currently rendered/cached
        // as active_screen-1; treat as a known gap for now (see caveat below)
        screen = active_screen;
        bx = 0;
    } else if (raw_x < 256) {
        screen = active_screen;
        bx = raw_x / 16;
    } else {
        screen = (active_screen + 1) % N_SCREENS;
        bx = (raw_x - 256) / 16;
    }

    uint8_t by = (draw_y + camera_y) / 16;
    return block_buffer[screen][by][bx];
}


void collide_sprite(struct sprite *sprite, int8_t dy, int8_t dx) {
    uint8_t block_x = (sprite->x + sprite->hitbox.x) / 16;
    uint8_t block_y = (sprite->y + sprite->hitbox.y) / 16;

    uint16_t block_tl = block_buffer[active_screen][sprite->hitbox.x / 16][sprite->hitbox.y / 16];
    uint16_t block_tr = block_buffer[active_screen][(sprite->hitbox.x + sprite->hitbox.width) / 16][sprite->hitbox.y / 16];
    uint16_t block_bl = block_buffer[active_screen][sprite->hitbox.x / 16][(sprite->hitbox.y + sprite->hitbox.height) / 16];
    uint16_t block_br = block_buffer[active_screen][(sprite->hitbox.x + sprite->hitbox.width) / 16][(sprite->hitbox.y + sprite->hitbox.height) / 16];

    switch (block_tl) {
        case 0:
            sprite->falling = -1;
            break;
    }

    switch (block_tr) {
        case 0:
            sprite->falling = -1;
            break;
    }

    switch (block_bl) {
        case 0:
            sprite->falling = 0;
            break;
    }

    switch (block_br) { 
        case 0:
            sprite->falling = 0;
            break;
    }

}

void collide_entity(struct entity *entity, int8_t dy, int8_t dx) {
    struct sprite *sprite = &(entity->sprite);
    entity->falling = true;
    //char buf[64];

    uint16_t block_tl = solid_at(sprite->x + sprite->hitbox.x, sprite->y + sprite->hitbox.y);
    uint16_t block_tr = solid_at(sprite->x + sprite->hitbox.x + sprite->hitbox.width, sprite->y + sprite->hitbox.y);
    uint16_t block_bl = solid_at(sprite->x + sprite->hitbox.x, sprite->y + sprite->hitbox.y + sprite->hitbox.height);
    uint16_t block_br = solid_at(sprite->x + sprite->hitbox.x + sprite->hitbox.width, sprite->y + sprite->hitbox.y + sprite->hitbox.height);

    if (block_tl != 0){
        if (solid_at(sprite->x + sprite->hitbox.x, sprite->y + sprite->hitbox.y - dy) == 0){
            if (entity->y_vel < 0)
                entity->y_vel = 0;
        }
        if  (solid_at(sprite->x + sprite->hitbox.x - dx, sprite->y + sprite->hitbox.y) == 0){
            if (entity->x_vel < 0)
                entity->x_vel = 0;
        }

    }

    //fprintf(stderr, "block_tl %u\n", block_tl);

    if (block_tr != 0){
        if (solid_at(sprite->x + sprite->hitbox.x + sprite->hitbox.width, sprite->y + sprite->hitbox.y - dy) == 0){
            if (entity->y_vel < 0)
                entity->y_vel = 0;
        }
        if  (solid_at(sprite->x + sprite->hitbox.x + sprite->hitbox.width - dx, sprite->y + sprite->hitbox.y) == 0){
            if (entity->x_vel > 0)
                entity->x_vel = 0;
        }

    }
    //fprintf(stderr, "block_tr %u\n", block_tr);

    if (block_br != 0){
        if (solid_at(sprite->x + sprite->hitbox.x + sprite->hitbox.width, sprite->y + sprite->hitbox.y + sprite->hitbox.height - dy) == 0){
            if (entity->y_vel > 0)
                entity->y_vel = 0;
        }

    if (solid_at(sprite->x + sprite->hitbox.x + sprite->hitbox.width - dx, sprite->y + sprite->hitbox.y + sprite->hitbox.height) == 0){
            if (entity->x_vel > 0)
                entity->x_vel = 0;
        }
        // Older buggy code, cuz only checking dx
        //if  (block_buffer[active_screen][(sprite->y + sprite->hitbox.y+ sprite->hitbox.height)/16][(sprite->x + sprite->hitbox.x - dx)/16] == 0){
        //    entity->falling = false;
        //}

    }
    //fprintf(stderr, "block_br %u\n", block_br);

    if (block_bl != 0){
        if (solid_at(sprite->x + sprite->hitbox.x, sprite->y + sprite->hitbox.y + sprite->hitbox.height - dy) == 0){
            if (entity->y_vel > 0)
                entity->y_vel = 0;
        }

    if (solid_at(sprite->x + sprite->hitbox.x - dx, sprite->y + sprite->hitbox.y + sprite->hitbox.height) == 0){
            if (entity->x_vel < 0)
                entity->x_vel = 0;
        }
        // Older buggy code, cuz only checking dx
        //if  (block_buffer[active_screen][(sprite->y + sprite->hitbox.y+ sprite->hitbox.height)/16][(sprite->x + sprite->hitbox.x + sprite->hitbox.width - dx)/16] == 0){
        //    entity->falling = false;
        //}

    }
    //fprintf(stderr, "block_bl %u\n", block_bl);

    uint16_t ground_check_br = block_br;//(sprite->x + sprite->hitbox.x + sprite->hitbox.width, sprite->y + sprite->hitbox.y + sprite->hitbox.height);
    uint16_t ground_check_bl = block_bl; //(sprite->x + sprite->hitbox.x, sprite->y + sprite->hitbox.y + sprite->hitbox.height);
    
    if (ground_check_br != 0 || ground_check_bl != 0) {
        entity->falling = false;
        int16_t feet_y = sprite->y + sprite->hitbox.y + sprite->hitbox.height;
        int16_t snapped_row_top = (feet_y / 16) * 16;
        sprite->y -= (feet_y - snapped_row_top);
    }

    if (block_tl !=0) {
        int16_t body_left_x = sprite->x + sprite->hitbox.x + camera_x;
        int16_t snapped_row_left = (body_left_x / 16) * 16 +16;
        // sprite->x -= (body_left_x - snapped_row_left - camera_x);
        sprite->x = snapped_row_left - sprite->hitbox.x - camera_x;
    }

    if(block_tr !=0){
        int16_t body_right_x = sprite->x + sprite->hitbox.x + sprite->hitbox.width + camera_x;
        int16_t snapped_row_right = (body_right_x / 16) * 16;
        //sprite->x -= (body_right_x - snapped_row_right);
        sprite->x = snapped_row_right - sprite->hitbox.x - sprite->hitbox.width - camera_x;
    }

}


uint16_t guide_counter = 0;
struct camera_guide *guide;

void camera_reset_pos(uint16_t y, uint16_t x) {
    camera_y = y;
    camera_x = x;
}

void camera_set_guides(struct camera_guide *g) { guide = g; }

void camera_increment_guide(int8_t direction) { guide_counter += direction; }

void camera_get_pos() {
    struct camera_guide *g = &guide[guide_counter];
    uint16_t len = g->mode & AXIS ? camera_x : camera_y;
    if (len > g->loc) {
    }
    // switch (g->mode) {
    // case FLIP_X:
    // case FLIP_Y:
    // case AUTOSCROLL:
    //     break;
    // }
}

void camera_move(int16_t y, int16_t x) {
    // uint8_t to_stream_x = x / 16;
    // uint8_t to_move_x = x % 16;
    int32_t next_x = camera_x + x;
    if (next_x < 0) {
        next_x = next_x + 256;
        active_screen = (active_screen - 1 + N_SCREENS) % N_SCREENS;
        fprintf(stderr, "prev screen\n");
        stream_screen(true); 
    } else if (255 < next_x) {
        next_x = next_x - 256;
        active_screen = (active_screen + 1) % N_SCREENS;
        fprintf(stderr, "next screen\n");
        stream_screen(false); 
    }
    camera_x = next_x;
}

void camera_draw_sprite(struct sprite *sprite) {
}

void camera_draw() {
    for (uint8_t y = 0; y < 16; y++) {
        for (uint8_t x = 0; x < 16; x++) {
            uint16_t block = block_buffer[active_screen][y][x];
            draw_tile(blocks[block][0], y * 16 - camera_y, x * 16 - camera_x, 0, 0);
            draw_tile(blocks[block][1], y * 16 - camera_y + 8, x * 16 - camera_x, 0, 0);
            draw_tile(blocks[block][2], y * 16 - camera_y, x * 16 - camera_x + 8, 0, 0);
            draw_tile(blocks[block][3], y * 16 - camera_y + 8, x * 16 - camera_x + 8, 0, 0);

            block = block_buffer[(active_screen + 1) % N_SCREENS][y][x];
            draw_tile(blocks[block][0], y * 16 - camera_y, x * 16 - camera_x + 256, 0, 0);
            draw_tile(blocks[block][1], y * 16 - camera_y + 8, x * 16 - camera_x + 256, 0, 0);
            draw_tile(blocks[block][2], y * 16 - camera_y, x * 16 - camera_x + 256 + 8, 0, 0);
            draw_tile(blocks[block][3], y * 16 - camera_y + 8, x * 16 - camera_x + 256 + 8, 0, 0);
        }
        // fprintf(stderr, "\n");
    }
    // fprintf(stderr, "\n");
}

