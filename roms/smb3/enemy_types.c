#include "enemy_types.h"
#include "defs.h"
#include "collision.h"
#include "enemy.h"
#include "../../src/graphics.h"
#include <stdio.h>

/* ══════════════════════════════════════════════════════════════════════
 *  Dummy textures — replace with real tileset references later
 * ══════════════════════════════════════════════════════════════════ */

static const struct texture tex_goomba = {
    .height = 2, .width = 2, .num_frames = 1,
    .tiles = (uint16_t[]){TILE(6, 5, 3), TILE(6, 5, 4),
                          TILE(6, 5, 5), TILE(6, 5, 6)},
};
static const struct texture tex_goomba_flat = {
    .height = 1, .width = 2, .num_frames = 1,
    .tiles = (uint16_t[]){TILE(6, 5, 5), TILE(6, 5, 6)},
};

static const struct texture tex_turtle = {
    .height = 2, .width = 2, .num_frames = 1,
    .tiles = (uint16_t[]){TILE(6, 5, 3), TILE(6, 5, 4),
                          TILE(6, 5, 5), TILE(6, 5, 6)},
};
static const struct texture tex_shell = {
    .height = 2, .width = 2, .num_frames = 1,
    .tiles = (uint16_t[]){TILE(6, 5, 5), TILE(6, 5, 6),
                          TILE(6, 5, 5), TILE(6, 5, 6)},
};

static const struct texture tex_duck = {
    .height = 2, .width = 2, .num_frames = 1,
    .tiles = (uint16_t[]){TILE(6, 5, 3), TILE(6, 5, 4),
                          TILE(6, 5, 5), TILE(6, 5, 6)},
};

static const struct texture tex_shooter = {
    .height = 2, .width = 2, .num_frames = 1,
    .tiles = (uint16_t[]){TILE(6, 5, 3), TILE(6, 5, 4),
                          TILE(6, 5, 5), TILE(6, 5, 6)},
};
static const struct texture tex_fireball = {
    .height = 1, .width = 1, .num_frames = 1,
    .tiles = (uint16_t[]){TILE(6, 5, 3)},
};

/* ── Shared helpers ────────────────────────────────────────────────── */

extern uint8_t active_screen;
extern uint8_t camera_x;

static void kill_enemy(struct entity *self) {
    self->type = ENTITY_KILLED;
    self->sprite.hitbox.height = 0;
    self->sprite.hitbox.width = 0;
    self->x_vel = 0;
    self->y_vel = 0;
}

static void reverse_on_wall(struct entity *self) {
    /* Check if the leading edge hit a wall (x_vel was zeroed by collision) */
    if (self->x_vel == 0) {
        self->x_dir = -self->x_dir;
        self->x_vel = self->x_dir;
    }
}

/* ══════════════════════════════════════════════════════════════════════
 *  1. GOOMBA — walks, reverses on walls, squished when stomped
 * ══════════════════════════════════════════════════════════════════ */

static void goomba_init(struct entity *self) {
    self->x_dir = -1;
    self->x_vel = -1;
}

static void goomba_update(struct entity *self, float dt) {
    (void)dt;
    if (self->enemy_state == ENEM_DEAD) {
        if (self->state_timer > 0) {
            self->state_timer--;
        } else {
            kill_enemy(self);
        }
        return;
    }
    reverse_on_wall(self);
    self->x_vel = self->x_dir;
}

static void goomba_stomped(struct entity *self, struct entity *player) {
    (void)player;
    self->enemy_state = ENEM_DEAD;
    self->state_timer = 30;
    self->sprite.texture = &tex_goomba_flat;
    self->sprite.hitbox.height = 8;
    self->x_vel = 0;
    self->y_vel = 0;
}

static void goomba_side_hit(struct entity *self, struct entity *player) {
    (void)self;
    /* Player dies */
    player->sprite.hitbox.height = 0;
    player->sprite.hitbox.width = 0;
    player->falling = true;
    player->y_vel = -3.0f;
}

static void goomba_shell_hit(struct entity *self) {
    /* Launched upward and killed */
    self->y_vel = -4.0f;
    self->falling = true;
    kill_enemy(self);
}

/* ══════════════════════════════════════════════════════════════════════
 *  2. TURTLE — walks, enters shell when stomped, shell slides on 2nd hit
 * ══════════════════════════════════════════════════════════════════ */

#define SHELL_SLIDE_SPEED 4

static void turtle_init(struct entity *self) {
    self->x_dir = -1;
    self->x_vel = -1;
}

static void turtle_update(struct entity *self, float dt) {
    (void)dt;
    switch (self->enemy_state) {
    case ENEM_WALKING:
        reverse_on_wall(self);
        self->x_vel = self->x_dir;
        break;

    case ENEM_SHELL_IDLE:
        /* Just sit there */
        self->x_vel = 0;
        break;

    case ENEM_SHELL_SLIDING:
        /* Reverse direction when hitting a wall (collision zeroed x_vel) */
        if (self->x_vel == 0) {
            self->x_dir = -self->x_dir;
            self->x_vel = self->x_dir * SHELL_SLIDE_SPEED;
        }
        break;

    default:
        break;
    }
}

static void turtle_stomped(struct entity *self, struct entity *player) {
    (void)player;
    switch (self->enemy_state) {
    case ENEM_WALKING:
        /* Enter shell */
        self->enemy_state = ENEM_SHELL_IDLE;
        self->sprite.texture = &tex_shell;
        self->x_vel = 0;
        break;

    case ENEM_SHELL_IDLE:
        /* Kick shell in direction player is facing */
        self->enemy_state = ENEM_SHELL_SLIDING;
        if (get_world_x(player) < get_world_x(self))
            self->x_dir = 1;
        else
            self->x_dir = -1;
        self->x_vel = self->x_dir * SHELL_SLIDE_SPEED;
        break;

    case ENEM_SHELL_SLIDING:
        /* Stop the shell */
        self->enemy_state = ENEM_SHELL_IDLE;
        self->x_vel = 0;
        break;

    default:
        break;
    }
}

static void turtle_side_hit(struct entity *self, struct entity *player) {
    if (self->enemy_state == ENEM_SHELL_IDLE) {
        /* Kick shell away from player */
        self->enemy_state = ENEM_SHELL_SLIDING;
        if (get_world_x(player) < get_world_x(self))
            self->x_dir = 1;
        else
            self->x_dir = -1;
        self->x_vel = self->x_dir * SHELL_SLIDE_SPEED;
    } else {
        /* Walking or sliding shell hurts player */
        player->sprite.hitbox.height = 0;
        player->sprite.hitbox.width = 0;
        player->falling = true;
        player->y_vel = -3.0f;
    }
}

static void turtle_shell_hit(struct entity *self) {
    self->y_vel = -4.0f;
    self->falling = true;
    kill_enemy(self);
}

/* ══════════════════════════════════════════════════════════════════════
 *  3. DUCK — walks and periodically jumps
 * ══════════════════════════════════════════════════════════════════ */

#define DUCK_JUMP_INTERVAL 90   /* frames between jumps */
#define DUCK_JUMP_VEL      4.0f

static void duck_init(struct entity *self) {
    self->x_dir = -1;
    self->x_vel = -1;
    self->state_timer = DUCK_JUMP_INTERVAL;
}

static void duck_update(struct entity *self, float dt) {
    (void)dt;
    if (self->enemy_state == ENEM_DEAD) {
        if (self->state_timer > 0)
            self->state_timer--;
        else
            kill_enemy(self);
        return;
    }

    reverse_on_wall(self);
    self->x_vel = self->x_dir;

    if (!self->falling) {
        if (self->state_timer > 0) {
            self->state_timer--;
        } else {
            /* Jump! */
            self->y_vel = -DUCK_JUMP_VEL;
            self->state_timer = DUCK_JUMP_INTERVAL;
            self->enemy_state = ENEM_JUMPING;
        }
    } else {
        self->enemy_state = ENEM_WALKING;
    }
}

static void duck_stomped(struct entity *self, struct entity *player) {
    (void)player;
    self->enemy_state = ENEM_DEAD;
    self->state_timer = 30;
    self->x_vel = 0;
    self->y_vel = 0;
}

static void duck_side_hit(struct entity *self, struct entity *player) {
    (void)self;
    player->sprite.hitbox.height = 0;
    player->sprite.hitbox.width = 0;
    player->falling = true;
    player->y_vel = -3.0f;
}

static void duck_shell_hit(struct entity *self) {
    self->y_vel = -4.0f;
    self->falling = true;
    kill_enemy(self);
}

/* ══════════════════════════════════════════════════════════════════════
 *  4. FIREBALL SHOOTER — stationary, fires projectiles at intervals
 * ══════════════════════════════════════════════════════════════════ */

#define SHOOT_INTERVAL    120   /* frames between shots */
#define FIREBALL_SPEED    3
#define FIREBALL_LIFETIME 180   /* auto-kill after N frames */

static void shooter_init(struct entity *self) {
    self->x_vel = 0;     /* stationary */
    self->state_timer = SHOOT_INTERVAL;
}

static void spawn_fireball(struct entity *shooter, int8_t direction) {
    int8_t slot = find_free_entity_slot();
    if (slot < 0) return;   /* no free slots */

    struct entity *fb = &entities[slot];
    *fb = (struct entity){
        .type         = ENTITY_PROJECTILE,
        .sprite = {
            .texture  = &tex_fireball,
            .palette  = 0,
            .attributes = 0,
            .x        = 0, /* will be projected */
            .y        = shooter->sprite.y + 4,
            .hitbox   = { .x = 0, .y = 0, .height = 8, .width = 8 },
        },
        .x_vel        = direction * FIREBALL_SPEED,
        .falling      = false,
        .state_timer  = FIREBALL_LIFETIME,
        .sim_active   = true,
    };
    
    int16_t spawn_x = get_world_x(shooter) + (direction > 0 ? 16 : -8);
    set_world_x(fb, spawn_x);
}

static void shooter_update(struct entity *self, float dt) {
    (void)dt;

    if (self->state_timer > 0) {
        self->state_timer--;
    } else {
        /* Fire toward player (entities[0]) */
        int8_t dir = (get_world_x(&entities[0]) < get_world_x(self)) ? -1 : 1;
        spawn_fireball(self, dir);
        self->state_timer = SHOOT_INTERVAL;
    }
}

static void shooter_stomped(struct entity *self, struct entity *player) {
    /* Cannot be stomped — player takes damage */
    (void)self;
    player->sprite.hitbox.height = 0;
    player->sprite.hitbox.width = 0;
    player->falling = true;
    player->y_vel = -3.0f;
}

static void shooter_side_hit(struct entity *self, struct entity *player) {
    (void)self;
    player->sprite.hitbox.height = 0;
    player->sprite.hitbox.width = 0;
    player->falling = true;
    player->y_vel = -3.0f;
}

static void shooter_shell_hit(struct entity *self) {
    kill_enemy(self);
}

/* ══════════════════════════════════════════════════════════════════════
 *  Vtable array — indexed by enum enemy_subtype
 * ══════════════════════════════════════════════════════════════════ */

const struct enemy_type enemy_types[ENEMY_SUBTYPE_COUNT] = {
    [ENEMY_GOOMBA] = {
        .on_init     = goomba_init,
        .on_update   = goomba_update,
        .on_stomped  = goomba_stomped,
        .on_side_hit = goomba_side_hit,
        .on_shell_hit = goomba_shell_hit,
        .texture     = &tex_goomba,
        .default_x_vel = -1,
        .hitbox_w    = 16,
        .hitbox_h    = 16,
    },
    [ENEMY_TURTLE] = {
        .on_init     = turtle_init,
        .on_update   = turtle_update,
        .on_stomped  = turtle_stomped,
        .on_side_hit = turtle_side_hit,
        .on_shell_hit = turtle_shell_hit,
        .texture     = &tex_turtle,
        .default_x_vel = -1,
        .hitbox_w    = 16,
        .hitbox_h    = 16,
    },
    [ENEMY_DUCK] = {
        .on_init     = duck_init,
        .on_update   = duck_update,
        .on_stomped  = duck_stomped,
        .on_side_hit = duck_side_hit,
        .on_shell_hit = duck_shell_hit,
        .texture     = &tex_duck,
        .default_x_vel = -1,
        .hitbox_w    = 16,
        .hitbox_h    = 16,
    },
    [ENEMY_FIREBALL_SHOOTER] = {
        .on_init     = shooter_init,
        .on_update   = shooter_update,
        .on_stomped  = shooter_stomped,
        .on_side_hit = shooter_side_hit,
        .on_shell_hit = shooter_shell_hit,
        .texture     = &tex_shooter,
        .default_x_vel = 0,
        .hitbox_w    = 16,
        .hitbox_h    = 16,
    },
};

