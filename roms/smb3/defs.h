#pragma once

#include "../../src/graphics.h"

#define TILESET_ROW_OFFSET 16
#define TILESET_PAGE_OFFSET (TILESET_ROW_OFFSET * 32)

#define CLAMP(a, b, c) (a < b ? b : (a < c ? a : c))

#define CLAMP_SET(a, b, c) { \
    if (a > b) { \
        b = a; \
    } else if (b > c) { \
        b = c; \
    } \
}

#define ABS(a) (a < 0 ? -a : a)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define GIF(num, dem) \
    (((num < 0 && dem > 0) || (num > 0 && dem < 0)) && ((num) % (dem) != 0) ? \
     (num) / (dem) - 1 : (num) / (dem))

#define TILE(page, row, column)                                                \
    (page * TILESET_PAGE_OFFSET + row * TILESET_ROW_OFFSET + column)

/* Floor-align to 16px tile boundary (works for negative values) */
#define SNAP_TILE(x) (((x) >> 4) << 4)

#define CAMERA_WIDTH 256
#define SIM_ZONE_WIDTH 512

enum entity_type : unsigned char {
    ENTITY_NONE,
    ENTITY_PLAYER,
    ENTITY_BOX,
    ENTITY_ENEMY,
    ENTITY_PROJECTILE,
    ENTITY_KILLED,
};

struct entity {
    struct sprite sprite;
    enum entity_type type;
    int8_t x_dir;
    float y_vel;
    float x_vel;
    float x_accel;
    float y_accel;
    float x_accumulator;
    float y_accumulator;
    bool falling;
    uint8_t holding;

    /* Enemy-specific fields */
    uint8_t enemy_subtype;    /* enum enemy_subtype */
    uint8_t enemy_state;      /* enum enemy_state */
    uint8_t state_timer;      /* generic timer for state transitions */
    /* World Position (Authoritative for simulation & physics horizontally) */
    uint8_t world_screen;     /* canonical absolute world screen index */
    uint8_t world_x;          /* canonical absolute offset within screen */
    
    /* sim_active simply dictates whether AI/physics updates are run.
       sprite.x is strictly a projected camera-relative offset. */
    bool    sim_active;       
};

void move_entity(struct entity *entity);
int16_t entity_screen_x(const struct entity *entity);
bool entity_in_sim_zone(const struct entity *entity);
bool entity_in_draw_zone(const struct entity *entity);

#define NUM_ENTITIES 16
extern struct entity entities[NUM_ENTITIES];
