//#include "graphics.h"
#include <stdbool.h>
#include "camera.h"


int8_t colliding_x(struct sprite *s1, struct sprite *s2) {
    int16_t x_dist = s2->x + s2->hitbox.x - s1->x - s1->hitbox.x;
    if (x_dist > 0) {
        if (x_dist < s1->hitbox.width)
            return 1;
        else
            return 0;
    } else {
        if (x_dist < -s2->hitbox.width)
            return 0;
        else
            return -1;
    }
}

int8_t colliding_y(struct sprite *s1, struct sprite *s2) {
    int16_t y_dist = s2->y + s2->hitbox.y - s1->y - s1->hitbox.y;
    if (y_dist > 0) {
        if (y_dist < s1->hitbox.height)
            return 1;
        else
            return 0;
    } else {
        if (y_dist < -s2->hitbox.height)
            return 0;
        else
            return -1;
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