#include "../../src/graphics.h"
#include <stdbool.h>
#include "camera.h"


int8_t colliding_x(struct sprite *s1, struct sprite *s2) {
    int16_t x_dist = s2->x + s2->hitbox.x - s1->x - s1->hitbox.x;
    if (x_dist > 0) {
        if (x_dist < s1->hitbox.width)
            return 1; // s1 is colliding with (s2 on the right)
        else
            return 0;
    } else {
        if (x_dist < -s2->hitbox.width)
            return 0;
        else
            return -1; // s1 is colliding with (s2 on the left)
    }
}

int8_t colliding_y(struct sprite *s1, struct sprite *s2) {
    int16_t y_dist = s2->y + s2->hitbox.y - s1->y - s1->hitbox.y;
    if (y_dist > 0) {
        if (y_dist < s1->hitbox.height)
            return 1; // s2 is colliding with (s1 on the top)
        else
            return 0;
    } else {
        if (y_dist < -s2->hitbox.height)
            return 0;
        else
            return -1; // s2 is colliding with (s1 on the bottom)
    }
}

bool collidingp(struct sprite *sprite, uint8_t y, uint8_t x) {
    bool x_bound = sprite->x + sprite->hitbox.x < x &&
                   x < (sprite->x + sprite->hitbox.width);
    bool y_bound = sprite->y + sprite->hitbox.y < y &&
                   y < (sprite->y + sprite->hitbox.height);
    return x_bound && y_bound;
}

// bool collide_tile(struct sprite *sprite, )
extern uint8_t camera_x;
extern uint8_t camera_y;
extern uint8_t active_screen;
extern uint16_t block_buffer[N_SCREENS][16][16];


//COLLISION HELPER FUNCTION
uint16_t solid_at(int16_t draw_x, int16_t draw_y) {
    int32_t raw_x = draw_x + camera_x; // real position relative to left edge of active screen
    uint8_t screen;
    uint8_t bx;

    if (raw_x < 0) {
        screen = active_screen;
        bx = 0;
    } else if (raw_x < 256) {
        screen = active_screen;
        bx = raw_x / 16;
    } else {
        screen = (active_screen + 1) % N_SCREENS;
        bx = (raw_x - 256) / 16;
    }

    uint8_t by = (draw_y + camera_y) / 16;
    return block_buffer[screen][by][bx];
}


void collide_entity(struct entity *entity, int8_t dy, int8_t dx) {
    struct sprite *sprite = &(entity->sprite);
    entity->falling = true;
    //char buf[64];

    uint16_t block_tl = solid_at(sprite->x + sprite->hitbox.x, sprite->y + sprite->hitbox.y);
    uint16_t block_tr = solid_at(sprite->x + sprite->hitbox.x + sprite->hitbox.width, sprite->y + sprite->hitbox.y);
    uint16_t block_bl = solid_at(sprite->x + sprite->hitbox.x, sprite->y + sprite->hitbox.y + sprite->hitbox.height);
    uint16_t block_br = solid_at(sprite->x + sprite->hitbox.x + sprite->hitbox.width, sprite->y + sprite->hitbox.y + sprite->hitbox.height);


    // Check TOP LEFT
    if (block_tl != 0){
        int16_t body_left_x = sprite->x + sprite->hitbox.x + camera_x;
        int16_t snapped_row_left = (body_left_x / 16) * 16 +16;
        sprite->x = snapped_row_left - sprite->hitbox.x - camera_x;

        if (solid_at(sprite->x + sprite->hitbox.x, sprite->y + sprite->hitbox.y - dy) == 0){
            if (entity->y_vel < 0)
                entity->y_vel = 0;
        }
        if  (solid_at(sprite->x + sprite->hitbox.x - dx, sprite->y + sprite->hitbox.y) == 0){
            if (entity->x_vel < 0)
                entity->x_vel = 0;
        }

    }

    //fprintf(stderr, "block_tl %u\n", block_tl);

    // Check TOP RIGHT
    if (block_tr != 0){
        int16_t body_right_x = sprite->x + sprite->hitbox.x + sprite->hitbox.width + camera_x;
        int16_t snapped_row_right = (body_right_x / 16) * 16;
        sprite->x = snapped_row_right - sprite->hitbox.x - sprite->hitbox.width - camera_x;

        if (solid_at(sprite->x + sprite->hitbox.x + sprite->hitbox.width, sprite->y + sprite->hitbox.y - dy) == 0){
            if (entity->y_vel < 0)
                entity->y_vel = 0;
        }
        if  (solid_at(sprite->x + sprite->hitbox.x + sprite->hitbox.width - dx, sprite->y + sprite->hitbox.y) == 0){
            if (entity->x_vel > 0)
                entity->x_vel = 0;
        }

    }
    //fprintf(stderr, "block_tr %u\n", block_tr);

    // Check BOTTOM RIGHT
    if (block_br != 0){
        if (solid_at(sprite->x + sprite->hitbox.x + sprite->hitbox.width, sprite->y + sprite->hitbox.y + sprite->hitbox.height - dy) == 0){
            if (entity->y_vel > 0)
                entity->y_vel = 0;
        }

    if (solid_at(sprite->x + sprite->hitbox.x + sprite->hitbox.width - dx, sprite->y + sprite->hitbox.y + sprite->hitbox.height) == 0){
            if (entity->x_vel > 0)
                entity->x_vel = 0;
        }

    }
    //fprintf(stderr, "block_br %u\n", block_br);

    // Check BOTTOM LEFT
    if (block_bl != 0){
        if (solid_at(sprite->x + sprite->hitbox.x, sprite->y + sprite->hitbox.y + sprite->hitbox.height - dy) == 0){
            if (entity->y_vel > 0)
                entity->y_vel = 0;
        }

    if (solid_at(sprite->x + sprite->hitbox.x - dx, sprite->y + sprite->hitbox.y + sprite->hitbox.height) == 0){
            if (entity->x_vel < 0)
                entity->x_vel = 0;
        }

    }
    //fprintf(stderr, "block_bl %u\n", block_bl);

    // GROUND COLLISION CHECK
    if (block_br != 0 || block_bl != 0) {
        entity->falling = false;
        int16_t feet_y = sprite->y + sprite->hitbox.y + sprite->hitbox.height;
        int16_t snapped_row_top = (feet_y / 16) * 16;
        sprite->y -= (feet_y - snapped_row_top);
    }

}


enum entity_collision_type : unsigned char {
    PLAYER_KILL,
    ENEMY_KILL,
};

bool entity_is_enemy(struct entity *entity) {
    return entity->type == ENTITY_ENEMY;
}


static const struct texture tex_mario_die = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(10, 12, 12), TILE(10, 12, 12) | 1 << 15,
                          TILE(10, 12, 13), TILE(10, 12, 13) | 1 << 15},
    .num_frames = 1,
};
static const struct texture tex_box = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(1, 17, 6), TILE(1, 17, 7), TILE(1, 18, 6),
                          TILE(1, 18, 7)},
    .num_frames = 1,
};

static void handle_player_enemy_collision(struct entity *player, struct entity *enemy, enum entity_collision_type col_type) {
    if (col_type == PLAYER_KILL) {
        player->sprite.texture = &tex_mario_die;
        player->sprite.hitbox.height = 0;
        player->sprite.hitbox.width = 0;
        player->falling = true;
    } else if (col_type == ENEMY_KILL) {
        enemy->sprite.texture = &tex_box;
        enemy->sprite.hitbox.height = 0;
        enemy->sprite.hitbox.width = 0;
        enemy->falling = true;
    }
}


#define COUNT 8 // should be >= number of enemies in the level
uint8_t last_y_collided[COUNT] = {1}; //for enemy kill logic

void handle_entity_collisions(struct entity *player, struct entity entities[], uint8_t count) {
    
    
    for (uint8_t i = 0; i < count; i++) {
        struct entity *other = &entities[i];

        if (other == player)
            continue;

        if (!entity_is_enemy(other))
            continue;

        if (colliding_x(&player->sprite, &other->sprite) && colliding_y(&player->sprite, &other->sprite)) {
            if (colliding_y(&player->sprite, &other->sprite)==1 && !last_y_collided[i]) {
                handle_player_enemy_collision(player, other, ENEMY_KILL);
            } else {
                handle_player_enemy_collision(player, other, PLAYER_KILL);
            }
            last_y_collided[i] = (colliding_y(&player->sprite, &other->sprite) != 0);
        }
    }
}

