#pragma once

#include <stdint.h>

typedef uint32_t o2;
#define O2_NEXT_PAGE 1
#define O2_PALETTE(o) ((o & 0x0F) << 4)
#define O2_HEIGHT(o)  ((o & 0x0F) << 8)
#define O2_WIDTH(o)   ((o & 0x0F) << 12)
#define O2_Y(o)       ((o & 0x0F) << 16)
#define O2_X(o)       ((o & 0x0F) << 20)
#define O2_INDEX(o)   ((o & 0xFF) << 24)
#define O2(index, x, y, width, height, palette) ( \
          ((index & 0xFF) << 24) \
        | ((x & 0x0F) << 20) \
        | ((y & 0x0F) << 16) \
        | ((width & 0x0F) << 12) \
        | ((height & 0x0F) << 8) \
        | ((palette & 0x0F) << 4))

#define O2_PALETTE_GET(o) (o >> 4 & 0x0F)
#define O2_HEIGHT_GET(o)  (o >> 8 & 0x0F)
#define O2_WIDTH_GET(o)   (o >> 12 & 0x0F)
#define O2_Y_GET(o)       (o >> 16 & 0x0F)
#define O2_X_GET(o)       (o >> 20 & 0x0F)
#define O2_INDEX_GET(o)   (o >> 24)

typedef uint16_t o1;
#define O1_X(o) ((o & 0x0F) << 8)
#define O1_INDEX(o) (o & 0xFF)
#define O1(index, x) ((index & 0xFF) | ((x & 0x0F) << 8))

#define O1_X_GET(o) ((o >> 8) & 0x0F)
#define O1_INDEX_GET(o) (o & 0xFF)

#define TILE_X_MIRROR ((uint16_t)(1U << 14))
#define TILE_Y_MIRROR ((uint16_t)(1U << 15))


typedef uint16_t block[];

enum objflags {
    terminal_double = 1,
    middle_double = 1 << 1,
    axis = 1 << 2,
    _2d = 1 << 3,
};


enum o1flags {
    O1_TERMINAL = 1,
    O1_MIDDLE = 1 << 1,
};

enum o2flags {
    O2_TERMINAL = 1,
    O2_MIDDLE   = 1 << 2,
    O2_VERTICAL = 1 << 3,
};

struct obj1d {
    uint8_t start;
    uint8_t flags; // 2:start_len 1:middle
    // what if I need single block, so 1 start len and 0 end len
};


// typedef uint16_t o1; // 8:index 4:len 4:palette

struct obj2d {
    const uint16_t *start;
    uint8_t flags; // 1:height 1:width 1:axis 1:  
};



