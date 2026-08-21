#include <stdint.h>
#include "defs.h"
#include "../../src/input.h"
#include "../../src/graphics.h"
#include "camera.h"


static const struct texture tex_enemy = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(6, 5, 3), TILE(6, 5, 4), TILE(6, 5, 5),
                          TILE(6, 5, 6)},
    .num_frames = 1,
};


void enemy_init(struct entity *enemy){

    *enemy = (struct entity){
        .type = ENTITY_ENEMY,
        .sprite =
            {
                .texture = &tex_enemy,
                .palette = 0,
                .attributes = 0,
                .x = 192,
                .y = 128,
                .hitbox =
                    {
                        .x = 0,
                        .y = 0,
                        .height = 16,
                        .width = 16,
                    },
                .screen =0,
            },

        .falling = false,
    };

 
};

extern const float FALL_ACCEL;
extern float MAX_VEL_Y;
const uint8_t SCREEN_POSITION_MAX = 192; // Enemy will move between 64 and 192 on the screen index defined in enemy entity 
const uint8_t SCREEN_POSITION_MIN = 64;
uint8_t screen_position = 0;
int8_t box_x_vel = 1;
int16_t box_position_x;
extern uint8_t camera_x;

void enemy_update(struct entity *enemy, float deltatime){
    if (enemy->type == ENTITY_KILLED) {
        return;
    }
    if (enemy->falling && !enemy->holding) {
        enemy->y_vel += FALL_ACCEL * deltatime;
    } else {
        enemy->y_vel = enemy->y_vel > 0 ? 0 : enemy->y_vel;
    }

    enemy->y_vel = CLAMP(enemy->y_vel, -MAX_VEL_Y, MAX_VEL_Y);

    if (!enemy->holding)
        move_entity(enemy);


    if (screen_position+box_x_vel >= SCREEN_POSITION_MAX) {
        screen_position = SCREEN_POSITION_MAX;
        box_x_vel = -box_x_vel;
    } else if (screen_position+box_x_vel <= SCREEN_POSITION_MIN) {
        screen_position = SCREEN_POSITION_MIN;
        box_x_vel = -box_x_vel;
    } else {
        screen_position += box_x_vel;
    }

    box_position_x = screen_position - camera_x;

};

extern uint8_t active_screen;

void enemy_draw(struct entity *enemy){
    if (enemy->type == ENTITY_KILLED) {
    return;
    }
    if (enemy->sprite.screen == active_screen && box_position_x > 0) {
        enemy->sprite.x= screen_position-camera_x;
        draw_sprite(&enemy->sprite);
    }
    else if(enemy->sprite.screen == (active_screen + 1) % N_SCREENS && box_position_x < 0){
        enemy->sprite.x= screen_position+256-camera_x;
        draw_sprite(&enemy->sprite);
    }
};