#pragma once


#include <stdbool.h>
#include <stdint.h>
#include "defs.h"

struct sprite;
struct entity;

bool colliding_x(struct sprite *s1, struct sprite *s2);
bool colliding_y(struct sprite *s1, struct sprite *s2);
void collide_entity(struct entity *entity, int8_t dy, int8_t dx);
void handle_entity_collisions(struct entity *player, struct entity entities[], uint8_t count);


