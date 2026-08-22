#include "camera.h"
#include "tile.h"
#include "obj.h"
#include <stdbool.h>
#include <stdint.h>
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

    { TILE(7, 24, 12), TILE(7, 25, 12), TILE(7, 24, 13), TILE(7, 25, 13) }, /* pipe_tl */
    { TILE(7, 24, 14), TILE(7, 25, 14), TILE(7, 24, 15), TILE(7, 25, 15) }, /* pipe_tr */
    { TILE(7, 26, 12), TILE(7, 26, 12), TILE(7, 26, 13), TILE(7, 26, 13) }, /* pipe_bl */
    { TILE(7, 26, 14), TILE(7, 26, 14), TILE(7, 26, 15), TILE(7, 26, 15) }, /* pipe_br */

    { TILE(6, 2, 0), TILE(6, 3, 0), TILE(6, 2, 1), TILE(6, 3, 1) }, /* cloud */

    { TILE(7, 29, 12), TILE(7, 29, 13), TILE(7, 29, 14), TILE(7, 29, 15) }, /* brick */

    { TILE(1, 7, 12), TILE(1, 7, 13), TILE(1, 7, 14), TILE(1, 7, 15) }, /* wood */

    { TILE(3, 22, 0), TILE(3, 22, 1), TILE(3, 22, 2), TILE(3, 22, 3) }, /* pin_box */

    { TILE(7, 27, 8), TILE(7, 27, 9), TILE(7, 27, 10), TILE(7, 27, 11) }, /* note_block */

    { TILE(7, 25, 8), TILE(7, 25, 9), TILE(7, 25, 10), TILE(7, 25, 11) }, /* qmark_block */

    { TILE(3, 0, 10), TILE(3, 0, 11), TILE(3, 0, 12), TILE(3, 0, 13) }, /* shiny_block */

    { TILE(2, 9, 0), TILE(2, 9, 1), TILE(2, 9, 2), TILE(2, 9, 3) }, /* u_block */

    { TILE(6, 16, 10), TILE(6, 17, 10), TILE(6, 16, 11), TILE(6, 17, 11) }, /* vwood_tl */
    { TILE(6, 16, 12), TILE(6, 17, 12), TILE(6, 16, 13), TILE(6, 17, 13) }, /* vwood_tr */
    { TILE(6, 18, 10), TILE(6, 19, 10), TILE(6, 18, 11), TILE(6, 19, 11) }, /* vwood_bl */
    { TILE(6, 18, 12), TILE(6, 19, 12), TILE(6, 18, 13), TILE(6, 19, 13) }, /* vwood_br */

    { TILE(6, 1, 0), TILE(6, 1, 2), TILE(6, 0, 14), TILE(7, 31, 13) }, /* bpin_tl */
    { TILE(6, 0, 14), TILE(7, 31, 13), TILE(6, 0, 14), TILE(7, 31, 13) }, /* bpin_tm */
    { TILE(6, 0, 14), TILE(7, 31, 13), TILE(6, 1, 1), TILE(6, 1, 5) }, /* bpin_tr */
    { TILE(6, 1, 2), TILE(6, 1, 2), TILE(7, 31, 13), TILE(7, 31, 13) }, /* bpin_ml */
    { TILE(7, 31, 13), TILE(7, 31, 13), TILE(7, 31, 13), TILE(7, 31, 13) }, /* bpin_mm */
    { TILE(7, 31, 13), TILE(7, 31, 13), TILE(6, 1, 5), TILE(6, 1, 5) }, /* bpin_mr */
    { TILE(6, 1, 2), TILE(6, 1, 6), TILE(7, 31, 13), TILE(6, 0, 12) }, /* bpin_bl */
    { TILE(7, 31, 13), TILE(6, 0, 12), TILE(7, 31, 13), TILE(6, 0, 12) }, /* bpin_bm */
    { TILE(7, 31, 13), TILE(6, 0, 12), TILE(6, 1, 5), TILE(6, 1, 7) }, /* bpin_br */

    { TILE(11, 7, 4), TILE(11, 7, 5), TILE(11, 7, 6), TILE(11, 7, 7) }, /* o_block */

    { TILE(13, 21, 0), TILE(13, 21, 1), TILE(13, 21, 2), TILE(13, 21, 3) }, /* hbush_l */
    { TILE(13, 21, 4), TILE(13, 21, 5), TILE(13, 21, 2), TILE(13, 21, 3) }, /* hbush_m */
    { TILE(13, 21, 4), TILE(13, 21, 5), TILE(13, 21, 6), TILE(13, 21, 7) }, /* hbush_r */

    { TILE(13, 11, 4), TILE(13, 11, 7), TILE(13, 11, 5), TILE(13, 0, 5) }, /* rbridge_l */
    { TILE(13, 11, 4), TILE(13, 11, 7), TILE(13, 11, 5), TILE(13, 0, 5) }, /* rbridge_m */
    { TILE(13, 11, 6), TILE(13, 11, 7), TILE(13, 0, 5), TILE(13, 0, 5) }, /* rbridge_r */

    { TILE(6, 6, 11), TILE(6, 6, 15), TILE(6, 6, 12), TILE(6, 7, 0) }, /* tree_tl */
    { TILE(6, 6, 13), TILE(6, 7, 1), TILE(6, 6, 14), TILE(6, 7, 2) }, /* tree_tr */
    { TILE(6, 0, 5), TILE(6, 0, 5), TILE(6, 7, 3), TILE(6, 7, 3) }, /* tree_bl */
    { TILE(6, 7, 4), TILE(6, 7, 4), TILE(6, 0, 5), TILE(6, 0, 5) }, /* tree_br */
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
    (struct obj1d)/* PIPE_CAGE */{
        .start = 15,
        .flags = O1_MIDDLE
    },
    (struct obj1d)/* PIPE_SHAFT */{
        .start = 17,
        .flags = O1_MIDDLE
    },
    (struct obj1d) /* CLOUD */{
        .start = 19,
        .flags = O1_MIDDLE
    },
    (struct obj1d) /* BRICK */{
        .start = 20,
        .flags = O1_MIDDLE
    },
    (struct obj1d) /* WOOD */{
        .start = 21,
        .flags = O1_MIDDLE
    },
    (struct obj1d) /* PIN_BOX */{
        .start = 22,
        .flags = O1_MIDDLE
    },
    (struct obj1d) /* NOTE_BLOCK */{
        .start = 23,
        .flags = O1_MIDDLE
    },
    (struct obj1d) /* QMARK_BLOCK */{
        .start = 24,
        .flags = O1_MIDDLE
    },
    (struct obj1d) /* SHINY_BLOCK */{
        .start = 25,
        .flags = O1_MIDDLE
    },
    (struct obj1d) /* U_BLOCK */{
        .start = 26,
        .flags = O1_MIDDLE
    },
    (struct obj1d)/* VWOOD_TOP */{
        .start = 27,
        .flags = O1_MIDDLE
    },
    (struct obj1d)/* VWOOD_BOTTOM */{
        .start = 29,
        .flags = O1_MIDDLE
    },
    (struct obj1d)/* BPIN_TOP */{
        .start = 31,
        .flags = O1_TERMINAL | O1_MIDDLE
    },
    (struct obj1d)/* BPIN_MIDDLE */{
        .start = 34,
        .flags = O1_TERMINAL | O1_MIDDLE
    },
    (struct obj1d)/* BPIN_BOTTOM */{
        .start = 37,
        .flags = O1_TERMINAL | O1_MIDDLE
    },
    (struct obj1d) /* O_BLOCK */{
        .start = 40,
        .flags = O1_MIDDLE
    },
    (struct obj1d)/* HBUSH */{
        .start = 41,
        .flags = O1_TERMINAL | O1_MIDDLE
    },
    (struct obj1d)/* RBRIDGE */{
        .start = 44,
        .flags = O1_TERMINAL | O1_MIDDLE
    },
    (struct obj1d)/* TREE_TOP */{
        .start = 47,
        .flags = O1_MIDDLE
    },
    (struct obj1d)/* TREE_SHAFT */{
        .start = 49,
        .flags = O1_MIDDLE
    },
};

const struct obj2d objs2d[256] = {
    (struct obj2d) {
        // PILLAR PLATFORM
        .start = (uint16_t []) { O1(0, 0), O1(1, 8) },
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
    (struct obj2d){
        // PIPE
        .start = (uint16_t []) { O1(7, 0), O1(8, 0), O1(8, 0)},
        .flags =  O2_VERTICAL | O2_TERMINAL | O2_MIDDLE,
    },
    (struct obj2d){
        // CLOUD
        .start = (uint16_t []) { O1(9, 0), O1(9, 0), O1(9, 0)},
        .flags = O2_VERTICAL,
    },
    (struct obj2d){
        // BRICK
        .start = (uint16_t []) { O1(10, 0), O1(10, 0), O1(10, 0)},
        .flags = O2_VERTICAL,
    },
    (struct obj2d){
        // WOOD
        .start = (uint16_t []) { O1(11, 0), O1(11, 0), O1(11, 0)},
        .flags = O2_VERTICAL,
    },
    (struct obj2d){
        // PIN_BOX
        .start = (uint16_t []) { O1(12, 0), O1(12, 0), O1(12, 0)},
        .flags = O2_VERTICAL,
    },
    (struct obj2d){
        // NOTE_BLOCK
        .start = (uint16_t []) { O1(13, 0), O1(13, 0), O1(13, 0)},
        .flags = O2_VERTICAL,
    },
    (struct obj2d){
        // QMARK_BLOCK
        .start = (uint16_t []) { O1(14, 0), O1(14, 0), O1(14, 0)},
        .flags = O2_VERTICAL,
    },
    (struct obj2d){
        // SHINY_BLOCK
        .start = (uint16_t []) { O1(15, 0), O1(15, 0), O1(15, 0)},
        .flags = O2_VERTICAL,
    },
    (struct obj2d){
        // U_BLOCK
        .start = (uint16_t []) { O1(16, 0), O1(16, 0), O1(16, 0)},
        .flags = O2_VERTICAL,
    },
    (struct obj2d){
        // VWOOD
        .start = (uint16_t []) { O1(17, 0), O1(18, 0), O1(18, 0)},
        .flags =  O2_VERTICAL | O2_TERMINAL | O2_MIDDLE,
    },
    (struct obj2d){
        // BPIN
        .start = (uint16_t []) { O1(19, 0), O1(20, 0), O1(21, 0)},
        .flags =  O2_VERTICAL | O2_TERMINAL | O2_MIDDLE,
    },
    (struct obj2d){
        // U_BLOCK
        .start = (uint16_t []) { O1(22, 0), O1(22, 0), O1(22, 0)},
        .flags = O2_VERTICAL,
    },
    (struct obj2d){
        // HBUSH
        .start = (uint16_t []) { O1(23, 0), O1(23, 0), O1(23, 0)},
        .flags = O2_VERTICAL | O2_TERMINAL | O2_MIDDLE,
    },
    (struct obj2d){
        // RBRIDGE
        .start = (uint16_t []) { O1(24, 0), O1(24, 0), O1(24, 0)},
        .flags = O2_VERTICAL | O2_TERMINAL | O2_MIDDLE,
    },
    (struct obj2d){
        // TREE
        .start = (uint16_t []) { O1(25, 0), O1(26, 0), O1(26, 0)},
        .flags =  O2_VERTICAL | O2_TERMINAL | O2_MIDDLE,
    },
};


uint16_t map_x;
uint8_t screen_x = 0;
const uint32_t *active_map;

uint8_t active_screen;
uint8_t read_offset;
uint8_t write_offset;
struct block_entry {
    uint8_t id;
    uint8_t palette;
};

struct block_entry block_buffer[N_SCREENS][16][16];



void stream_screen(bool reverse) {
    uint32_t o;
    uint8_t target_screen = reverse ? read_offset : write_offset;

    while ((o = *active_map)) {

        const uint8_t y = O2_Y_GET(o);
        const uint8_t x = O2_X_GET(o);
        const uint8_t height = O2_HEIGHT_GET(o); 
        const uint8_t width = O2_WIDTH_GET(o) + 1;
        const uint8_t palette = O2_PALETTE_GET(o);

        const struct obj2d obj = objs2d[O2_INDEX_GET(o)];
        const bool terminal = obj.flags & O2_TERMINAL;
        const bool middle = obj.flags & O2_MIDDLE;
        const bool vertical = obj.flags & O2_VERTICAL;

        //fprintf(stderr, "o2: %b terminal %032b\n", o, terminal);

        uint16_t o1;
        uint8_t o1_offset = 0;
        for (uint8_t i = 0; i < height; i++) {
            if (terminal && (i == 0)) {
                o1 = obj.start[o1_offset];
                //fprintf(stderr, "\tstarting o1 %u: %016b\n", o1_offset, o1);
                o1_offset++;
            } else if (terminal && (i == height - 1)) {
                if (middle && height > 2)
                    o1_offset++;
                o1 = obj.start[o1_offset];
                //fprintf(stderr, "\tending o1 %u: %016b\n", o1_offset, o1);
            } else {
                o1 = obj.start[o1_offset];
                //fprintf(stderr, "\tmiddle o1 %u: %016b\n", o1_offset, o1);
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
                    //fprintf(stderr, "\t\tstart block %u: %u\n", j, block);
                    if (!o1_middle)
                        cont = false;
                } else if (j == width - 1) {
                    block = obj1.start + (o1_terminal ? 1 : 0) + (middle ? 1 : 0);
                    //fprintf(stderr, "\t\tend block %u: %u\n", j, block);
                } else {
                    block = obj1.start + (o1_terminal ? 1 : 0);
                    //fprintf(stderr, "\t\tmiddle block %u: %u\n", j, block);
                }

                if (vertical) {
                    block_buffer[target_screen][i + y][j + x + o1_x].id = block;
                    block_buffer[target_screen][i + y][j + x + o1_x].palette = palette;
                } else {
                    block_buffer[target_screen][j + x + o1_x][i + y].id = block;
                    block_buffer[target_screen][j + x + o1_x][i + y].palette = block;
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

uint16_t guide_counter = 0;
struct camera_guide *guide;
// CAMERA

uint8_t camera_x = 0;
uint8_t camera_y = 0;

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
            uint16_t block = block_buffer[active_screen][y][x].id;
            uint8_t palette = block_buffer[active_screen][y][x].palette;
            draw_tile(blocks[block][0], y * 16 - camera_y, x * 16 - camera_x, palette, 0);
            draw_tile(blocks[block][1], y * 16 - camera_y + 8, x * 16 - camera_x, palette, 0);
            draw_tile(blocks[block][2], y * 16 - camera_y, x * 16 - camera_x + 8, palette, 0);
            draw_tile(blocks[block][3], y * 16 - camera_y + 8, x * 16 - camera_x + 8, palette, 0);

            block = block_buffer[(active_screen + 1) % N_SCREENS][y][x].id;
            palette = block_buffer[(active_screen + 1) % N_SCREENS][y][x].palette;
            draw_tile(blocks[block][0], y * 16 - camera_y, x * 16 - camera_x + 256, palette, 0);
            draw_tile(blocks[block][1], y * 16 - camera_y + 8, x * 16 - camera_x + 256, palette, 0);
            draw_tile(blocks[block][2], y * 16 - camera_y, x * 16 - camera_x + 256 + 8, palette, 0);
            draw_tile(blocks[block][3], y * 16 - camera_y + 8, x * 16 - camera_x + 256 + 8, palette, 0);
        }
        // fprintf(stderr, "\n");
    }
    // fprintf(stderr, "\n");
}

