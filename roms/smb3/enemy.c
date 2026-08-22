#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "defs.h"
#include "enemy_types.h"
#include "collision.h"
#include "../../src/input.h"
#include "../../src/graphics.h"
#include "camera.h"

/* ── Externs ───────────────────────────────────────────────────────── */
extern const float FALL_ACCEL;
extern float MAX_VEL_Y;
extern uint8_t camera_x;
extern uint8_t camera_y;
extern uint8_t active_screen;
extern int16_t center;

/* ── Distance calculation (ring-buffered screens) ──────────────────
 *  Returns signed pixel distance from (screen1,x1) to (screen2,x2).
 *  Positive = screen2 is to the right of screen1.
 *  Returns DISTANCE_TOO_FAR if screens are too far apart.
 *  WORKS ONLY FOR N_SCREENS > 4
 * ────────────────────────────────────────────────────────────────── */
enum Distance : int16_t {
    DISTANCE_TOO_FAR = 1 << 14,
};

int16_t real_world_distance(uint8_t screen1, uint8_t x1,
                            uint8_t screen2, uint8_t x2) {
    int16_t abs1 = screen1 * 256 + x1;
    int16_t abs2 = screen2 * 256 + x2;
    int16_t diff = abs1 - abs2;

    if (diff > 1024) {
        diff -= 2048;
    } else if (diff < -1024) {
        diff += 2048;
    }

    return diff;
}

/* ── World Coordinate Helpers ─────────────────────────────────────── */

int16_t get_world_x(struct entity *e) {
    return e->world_screen * 256 + e->world_x;
}

void set_world_x(struct entity *e, int16_t absolute_x) {
    while (absolute_x < 0) absolute_x += 2048;
    while (absolute_x >= 2048) absolute_x -= 2048;
    
    e->world_screen = absolute_x / 256;
    e->world_x = absolute_x % 256;
}

void entity_world_to_sprite(struct entity *e, uint8_t cam_screen, uint8_t cam_x) {
    int16_t dist = real_world_distance(e->world_screen, e->world_x, cam_screen, cam_x);
    e->sprite.x = dist;
}

void entity_sprite_to_world(struct entity *e, uint8_t cam_screen, uint8_t cam_x) {
    int32_t world_offset = (int32_t)e->sprite.x + cam_x;
    uint8_t screen = cam_screen;
    while (world_offset < 0) {
        screen = (screen - 1 + N_SCREENS) % N_SCREENS;
        world_offset += 256;
    }
    while (world_offset >= 256) {
        screen = (screen + 1) % N_SCREENS;
        world_offset -= 256;
    }
    e->world_screen = screen;
    e->world_x = (uint8_t)world_offset;
}

/* ── Init enemy at world position ──────────────────────────────────── */
void enemy_init_at(struct entity *enemy, uint8_t screen, uint8_t x,
                   uint8_t y, uint8_t subtype) {
    const struct enemy_type *etype = &enemy_types[subtype];

    *enemy = (struct entity){
        .type         = ENTITY_ENEMY,
        .enemy_subtype = subtype,
        .enemy_state  = ENEM_WALKING,
        .state_timer  = 0,
        .world_screen = screen,
        .world_x      = x,
        .sim_active   = false,
        .falling      = false,
        .sprite = {
            .texture  = etype->texture,
            .palette  = 0,
            .attributes = 0,
            .x        = x,       /* will be overwritten on activation */
            .y        = y,
            .hitbox   = {
                .x      = 0,
                .y      = 0,
                .height = etype->hitbox_h,
                .width  = etype->hitbox_w,
            },
            .screen   = screen,
        },
        .x_vel = etype->default_x_vel,
    };

    if (etype->on_init)
        etype->on_init(enemy);
}

/* ── Per-frame enemy update ────────────────────────────────────────
 *
 *  Simulation zone: 512px from camera_x (2 screens worth)
 *    - Entities within this zone get physics + AI updates
 *    - sprite.x holds camera-relative offset
 *
 *  Outside sim zone:
 *    - Entity is dormant, world_screen/world_x hold world coords
 *    - No physics applied
 *
 *  Render zone: first 256px (camera viewport) — drawing only
 * ────────────────────────────────────────────────────────────────── */
void enemy_update(struct entity *enemy, float deltatime) {
    if (enemy->type == ENTITY_KILLED) return;

    int16_t dist = real_world_distance(
        enemy->world_screen, enemy->world_x,
        active_screen, camera_x);

    if (dist >= -128 && dist < 384) {
        if (!enemy->sim_active) {
            enemy->sim_active = true;
        }

        const struct enemy_type *etype = &enemy_types[enemy->enemy_subtype];
        if (etype->on_update)
            etype->on_update(enemy, deltatime);

        if (!enemy->holding) {
            if (enemy->falling) {
                enemy->y_vel += FALL_ACCEL * deltatime;
            } else {
                enemy->y_vel = enemy->y_vel > 0 ? 0 : enemy->y_vel;
            }
            enemy->y_vel = CLAMP(enemy->y_vel, -MAX_VEL_Y, MAX_VEL_Y);

            move_entity(enemy);
        }
    } else {
        if (enemy->sim_active) {
            enemy->sim_active = false;
        }
    }
}

/* ── Draw (only within 256px render zone) ──────────────────────────── */
void enemy_draw(struct entity *enemy) {
    if (enemy->type == ENTITY_KILLED) return;
    if (!enemy->sim_active) return;

    /* sprite.x is camera-relative; draw only if within viewport */
    if (enemy->sprite.x >= 0 && enemy->sprite.x < CAMERA_WIDTH) {
        draw_sprite(&enemy->sprite);
    }
}

/* ── Free entity slot finder ───────────────────────────────────────── */
int8_t find_free_entity_slot(void) {
    for (uint8_t i = 2; i < NUM_ENTITIES; i++) {
        if (entities[i].type == ENTITY_KILLED || entities[i].type == ENTITY_NONE) {
            return (int8_t)i;
        }
    }
    return -1;
}
