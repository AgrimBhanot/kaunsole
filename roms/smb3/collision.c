#include "../../src/graphics.h"
#include <stdbool.h>
#include <stdio.h>
#include "camera.h"
#include "defs.h"
#include "enemy_types.h"

/* ── Sprite-vs-sprite overlap tests ────────────────────────────────── */

int8_t colliding_x(struct sprite *s1, struct sprite *s2) {
    int16_t x_dist = (s2->x + s2->hitbox.x) - (s1->x + s1->hitbox.x);
    if (x_dist > 0) {
        if (x_dist < s1->hitbox.width)
            return 1;   /* s2 is to the right, overlapping */
        else
            return 0;
    } else {
        if (x_dist < -(int16_t)s2->hitbox.width)
            return 0;
        else
            return -1;  /* s2 is to the left, overlapping */
    }
}

int8_t colliding_y(struct sprite *s1, struct sprite *s2) {
    int16_t y_dist = (s2->y + s2->hitbox.y) - (s1->y + s1->hitbox.y);
    if (y_dist > 0) {
        if (y_dist < s1->hitbox.height)
            return 1;   /* s2 is below, overlapping (player above) */
        else
            return 0;
    } else {
        if (y_dist < -(int16_t)s2->hitbox.height)
            return 0;
        else
            return -1;  /* s2 is above, overlapping (player below) */
    }
}

/* ── Tile collision sampling ───────────────────────────────────────── */

extern uint16_t block_buffer[N_SCREENS][16][16];
extern uint8_t camera_y;

uint16_t solid_at_world(int16_t world_x, int16_t world_y) {
    if (world_y < 0 || world_y >= 256) return 0;
    
    while (world_x < 0) world_x += 2048;
    while (world_x >= 2048) world_x -= 2048;

    uint8_t screen = world_x / 256;
    uint8_t bx = (world_x % 256) / 16;
    uint8_t by = world_y / 16;

    return block_buffer[screen][by][bx];
}

/* ── Two-pass axis-separated tile collision ─────────────────────────
 *
 *  Pass 1 (X): move sprite by dx, then check the leading X edge
 *              against tile grid. Snap and zero x_vel if solid.
 *  Pass 2 (Y): move sprite by dy, then check the leading Y edge.
 *              Snap and zero y_vel. Detect ground (falling flag).
 *
 *  This prevents the diagonal-push glitch where landing on a
 *  platform corner ejects the entity sideways.
 * ────────────────────────────────────────────────────────────────── */

int16_t collide_entity_x(struct entity *entity, int16_t world_x, int32_t dx) {
    if (dx == 0) return world_x;

    struct sprite *s = &entity->sprite;
    int16_t top = s->y + s->hitbox.y;
    int16_t bot = s->y + s->hitbox.y + s->hitbox.height - 1;

    if (dx > 0) {
        /* Moving right — check right edge */
        int16_t right = world_x + s->hitbox.x + s->hitbox.width - 1;
        if (solid_at_world(right, top) || solid_at_world(right, bot)) {
            int16_t tile_left = SNAP_TILE(right);
            world_x = tile_left - s->hitbox.x - s->hitbox.width;
            entity->x_vel = 0;
        }
    } else {
        /* Moving left — check left edge */
        int16_t left = world_x + s->hitbox.x;
        if (solid_at_world(left, top) || solid_at_world(left, bot)) {
            int16_t tile_right = SNAP_TILE(left) + 16;
            world_x = tile_right - s->hitbox.x;
            entity->x_vel = 0;
        }
    }
    return world_x;
}

void collide_entity_y(struct entity *entity, int16_t world_x, int32_t dy) {
    struct sprite *s = &entity->sprite;
    entity->falling = true;

    int16_t left  = world_x + s->hitbox.x;
    int16_t right = world_x + s->hitbox.x + s->hitbox.width - 1;

    /* ── Downward / ground check ────────────────────────────────── */
    {
        int16_t bottom = s->y + s->hitbox.y + s->hitbox.height;
        if (solid_at_world(left, bottom) || solid_at_world(right, bottom)) {
            int16_t tile_top = SNAP_TILE(bottom);
            s->y = tile_top - s->hitbox.y - s->hitbox.height;
            if (entity->y_vel >= 0) {
                entity->y_vel = 0;
            }
            entity->falling = false;
        }
    }

    /* ── Upward / head check ────────────────────────────────────── */
    if (dy < 0) {
        int16_t top = s->y + s->hitbox.y;
        if (solid_at_world(left, top) || solid_at_world(right, top)) {
            int16_t tile_bot = SNAP_TILE(top) + 16;
            s->y = tile_bot - s->hitbox.y;
            entity->y_vel = 0;
        }
    }
}

/* ── Entity-vs-entity collision ────────────────────────────────────── */

static const struct texture tex_mario_die = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(10, 12, 12), TILE(10, 12, 12) | 1 << 15,
                          TILE(10, 12, 13), TILE(10, 12, 13) | 1 << 15},
    .num_frames = 1,
};

static void kill_player(struct entity *player) {
    player->sprite.texture = &tex_mario_die;
    player->sprite.hitbox.height = 0;
    player->sprite.hitbox.width = 0;
    player->falling = true;
    player->y_vel = -3.0f;  /* small upward bounce on death */
}

void handle_entity_collisions(struct entity *player, struct entity ents[], uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        struct entity *other = &ents[i];

        if (other == player) continue;
        if (other->type != ENTITY_ENEMY && other->type != ENTITY_PROJECTILE) continue;
        if (!other->sim_active) continue;
        if (other->sprite.hitbox.height == 0 || other->sprite.hitbox.width == 0) continue;

        int8_t cx = colliding_x(&player->sprite, &other->sprite);
        int8_t cy = colliding_y(&player->sprite, &other->sprite);

        if (cx && cy) {
            if (other->type == ENTITY_PROJECTILE) {
                /* Projectiles always hurt the player */
                kill_player(player);
                other->type = ENTITY_KILLED;
                continue;
            }

            const struct enemy_type *etype = &enemy_types[other->enemy_subtype];

            /* Player landing on top (cy == 1 means other is below player) */
            if (cy == 1 && player->y_vel >= 0) {
                if (etype->on_stomped) {
                    etype->on_stomped(other, player);
                    player->y_vel = -3.0f; /* bounce after stomp */
                }
            } else {
                /* Side / bottom collision */
                if (etype->on_side_hit) {
                    etype->on_side_hit(other, player);
                } else {
                    kill_player(player);
                }
            }
        }
    }
}
