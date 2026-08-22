#pragma once

#include <stdint.h>
#include "defs.h"

/* ── Enemy behavior vtable ─────────────────────────────────────────── */
struct enemy_type {
    void (*on_init)(struct entity *self);
    void (*on_update)(struct entity *self, float dt);
    void (*on_stomped)(struct entity *self, struct entity *player);
    void (*on_side_hit)(struct entity *self, struct entity *player);
    void (*on_shell_hit)(struct entity *self);
    const struct texture *texture;
    int8_t  default_x_vel;
    uint8_t hitbox_w;
    uint8_t hitbox_h;
};

/* ── Enemy subtypes ────────────────────────────────────────────────── */
enum enemy_subtype : uint8_t {
    ENEMY_GOOMBA,
    ENEMY_TURTLE,
    ENEMY_DUCK,
    ENEMY_FIREBALL_SHOOTER,
    ENEMY_SUBTYPE_COUNT,
};

/* ── Per-entity state (stored in entity.enemy_state) ───────────────── */
enum enemy_state : uint8_t {
    ENEM_WALKING,
    ENEM_SHELL_IDLE,
    ENEM_SHELL_SLIDING,
    ENEM_JUMPING,
    ENEM_CHARGING,
    ENEM_DEAD,
};

extern const struct enemy_type enemy_types[ENEMY_SUBTYPE_COUNT];

/* Find a free entity slot (ENTITY_KILLED or ENTITY_NONE), return index or -1 */
int8_t find_free_entity_slot(void);
