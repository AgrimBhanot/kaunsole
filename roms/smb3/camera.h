#pragma once

#include <stdint.h>

#include "defs.h"

#define N_SCREENS 8


void map_draw(uint32_t *map);

enum camera_mode {
    AXIS = 1,
    DIRECTION = 1 << 1,
    AUTOSCROLL = 1 << 2,
};

struct camera_guide {
    uint16_t loc;
    enum camera_mode mode;
};

void load_map(const uint32_t *map);

void camera_reset_pos(uint16_t y, uint16_t x);
void camera_move(int16_t y, int16_t x);
void camera_set_guides(struct camera_guide *guide);
void camera_draw();
