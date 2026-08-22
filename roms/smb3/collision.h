#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "defs.h"

struct sprite;
struct entity;

int8_t colliding_x(struct sprite *s1, struct sprite *s2);
int8_t colliding_y(struct sprite *s1, struct sprite *s2);
/* ── Map block checking ──────────────────────────────────────────────── */
uint16_t solid_at_world(int16_t world_x, int16_t world_y);

/* ── Entity vs Map collisions (Axis-separated) ───────────────────────── */
int16_t collide_entity_x(struct entity *entity, int16_t world_x, int32_t dx);
void collide_entity_y(struct entity *entity, int16_t world_x, int32_t dy);
void handle_entity_collisions(struct entity *player, struct entity entities[], uint8_t count);
