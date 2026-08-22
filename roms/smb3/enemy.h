#pragma once

#include "defs.h"
#include <stdint.h>

/* Initialize an enemy at a world position with a given subtype */
void enemy_init_at(struct entity *enemy, uint8_t screen, uint8_t x,
                   uint8_t y, uint8_t subtype);

/* Per-frame update — handles sim zone entry/exit and dispatches to type */
void enemy_update(struct entity *enemy, float deltatime);

/* Draw enemy if within render zone */
void enemy_draw(struct entity *enemy);

/* World-distance helper (ring-buffered screens) */
int16_t real_world_distance(uint8_t screen1, uint8_t x1,
                            uint8_t screen2, uint8_t x2);

/* World coordinate helpers */
int16_t get_world_x(struct entity *e);
void set_world_x(struct entity *e, int16_t absolute_x);
void entity_world_to_sprite(struct entity *e, uint8_t cam_screen, uint8_t cam_x);
void entity_sprite_to_world(struct entity *e, uint8_t cam_screen, uint8_t cam_x);
