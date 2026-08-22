#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "collision.h"
#include "enemy.h"
#include "enemy_types.h"
#include "../../src/engine.h"
#include "../../src/rom.h"
#include "../../src/sprite.h"

#include "camera.h"
#include "defs.h"
#include "obj.h"

#define NUM_X_BLOCKS 8
#define NUM_Y_BLOCKS 4
#define BLOCK_WIDTH 40
#define BLOCK_HEIGHT 8

#define BALL_LEN 8
#define PADDLE_LEN 8

#define X_VEL 2
#define Y_VEL 2
#define PADDLE_Y 232

const uint32_t map[] = {
    O2(15, 0, 11, 1, 1, 1),
    O2(9, 4, 8, 0, 1, 2),
    O2(5, 8, 8, 0, 1, 3),
    O2(9, 9, 8, 0, 1, 2),
    O2(5, 10, 8, 0, 1, 3),
    O2(5, 10, 5, 0, 1, 3),
    O2(9, 11, 8, 0, 1, 2),
    O2(5, 12, 8, 0, 1, 3),
    O2(15, 11, 11, 3, 1, 1),
    O2(5, 0, 12, 15, 4, 3) | O2_NEXT_PAGE, //SCREEN 0

    O2(3, 0, 9, 1, 3, 1),
    O2(3, 6, 8, 1, 4, 1),
    O2(15, 9, 11, 2, 1, 1),
    O2(3, 13, 6, 1, 6, 1),
    O2(5, 0, 12, 15, 4, 3) | O2_NEXT_PAGE, //SCREEN 1

    O2_NEXT_PAGE, // I have no idea why this must be added for it to work.

    O2(3, 3, 6, 1, 6, 1),
    O2(15, 6, 11, 3, 1, 1),
    O2(5, 0, 12, 12, 4, 3) | O2_NEXT_PAGE, //SCREEN 2

    O2(15, 1, 11, 1, 1, 1),
    O2(5, 6, 8, 2, 1, 3),
    O2(5, 9, 4, 6, 1, 3),
    O2(5, 0, 12, 15, 4, 3) | O2_NEXT_PAGE, //SCREEN 3

    O2(5, 0, 4, 2, 1, 3),
    O2(5, 6, 4, 2, 1, 3),
    O2(9, 9, 4, 0, 1, 2),
    O2(15, 3, 11, 2, 1, 1),
    O2(9, 12, 8, 0, 1, 2),
    O2(9, 14, 8, 0, 1, 2),
    O2(9, 14, 5, 0, 1, 2),
    O2(5, 2, 12, 13, 4, 3) | O2_NEXT_PAGE, //SCREEN 4

    O2(9, 0, 8, 0, 1, 2),
    O2(15, 0, 11, 4, 1, 1),
    O2(5, 0, 12, 13, 4, 3) | O2_NEXT_PAGE, //SCREEN 5

    O2(3, 0, 10, 1, 2, 1),
    O2(3, 5, 10, 1, 2, 1),
    O2(15, 8, 11, 3, 1, 1),
    O2(12, 13, 5, 1, 7, 4),
    O2(5, 0, 12, 15, 4, 3) | O2_NEXT_PAGE, //SCREEN 6
    // O2(0, 0, 8, 3, 5, 0),
    // O2(2, 0, 12, 15, 2, 0) | O2_NEXT_PAGE,
    // O2_INDEX(0) | O2_Y(3) | O2_X(7) | O2_WIDTH(7) | O2_HEIGHT(5) |
    // O2_PALETTE(0), O2_INDEX(1) | O2_Y(8) | O2_X(8) | O2_WIDTH(4) |
    // O2_HEIGHT(5) | O2_PALETTE(0) | O2_NEXT_PAGE,
    0,

};

static const struct texture tex_none = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){ TILE(0, 3, 0), TILE(0, 3, 0), TILE(0, 3, 0), TILE(0, 3, 0) },
    .num_frames = 1,
};

static const struct texture tex_koopa_run = {
    .height = 4,
    .width = 2,
    .tiles = (uint16_t []){ TILE(9,28,1), TILE(9,28,0),
                            TILE(9,28,2), TILE(9,28,4),
                            TILE(9,28,3), TILE(9,28,5),
                            TILE(9,28,6), TILE(9,28,8),

                            TILE(9,28,1), TILE(9,28,0),
                            TILE(9,28,10), TILE(9,28,4),
                            TILE(9,28,11), TILE(9,28,5),
                            TILE(9,31,8), TILE(9,31,10),
    },
    .num_frames = 2,
};

static const struct texture tex_koopa_shell = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t []){ TILE(9,29,0), TILE(9,29,0) | TILE_Y_MIRROR,
                            TILE(9,29,1), TILE(9,29,1) | TILE_Y_MIRROR,

                            TILE(9,29,4) | TILE_Y_MIRROR, TILE(9,29,2) | TILE_Y_MIRROR,
                            TILE(9,29,5) | TILE_Y_MIRROR, TILE(9,29,3) | TILE_Y_MIRROR,

                            TILE(9,31,12), TILE(9,31,12) | TILE_Y_MIRROR,
                            TILE(9,31,13), TILE(9,31,13) | TILE_Y_MIRROR,

                            TILE(9,29,2), TILE(9,29,4),
                            TILE(9,29,3), TILE(9,29,5),
    },
    .num_frames = 4,
};

static const struct texture tex_coin_anim = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(7, 30, 8),TILE(7, 30, 10),
                          TILE(7, 30, 9),TILE(7, 30, 11),

    },
    .num_frames = 4,
};

static const struct texture tex_mushroom_p = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(0, 17, 0),TILE(0, 17, 0) | TILE_Y_MIRROR,
                          TILE(0, 17, 1),TILE(0, 17, 1) | TILE_Y_MIRROR,
                        },
    .num_frames = 1,
};

static const struct texture tex_flower_p = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(0, 17, 2),TILE(0, 17, 2) | TILE_Y_MIRROR,
                          TILE(0, 17, 3),TILE(0, 17, 3) | TILE_Y_MIRROR,
                        },
    .num_frames = 1,
};

static const struct texture tex_star_p = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(0, 17, 4),TILE(0, 17, 4) | TILE_Y_MIRROR,
                          TILE(0, 17, 5),TILE(0, 17, 5) | TILE_Y_MIRROR,
                        },
    .num_frames = 1,
};

static const struct texture tex_goomba_stand = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(9, 29, 8), TILE(9, 29, 8) | TILE_Y_MIRROR,
                          TILE(9, 29, 9),TILE(9, 29, 9) | TILE_Y_MIRROR,
                        },
    .num_frames = 1,
};

static const struct texture tex_goomba_run = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(9, 29, 8), TILE(9, 29, 8) | TILE_Y_MIRROR, 
                          TILE(9, 29, 9),TILE(9, 29, 11) | TILE_Y_MIRROR,

                          TILE(9, 29, 8), TILE(9, 29, 8) | TILE_Y_MIRROR,
                          TILE(9, 29, 11), TILE(9, 29, 9) | TILE_Y_MIRROR
                        },
    .num_frames = 2,
};

static const struct texture tex_goomba_die = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(9, 31, 11), TILE(9, 31, 11), 
                          TILE(9, 30, 11),TILE(9, 30, 11) | TILE_Y_MIRROR,
                        },
    .num_frames = 1,
};

static const struct texture tex_bullet_b = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(9, 29, 12), TILE(9, 29, 14), 
                          TILE(9, 29, 13),TILE(9, 29, 15),
                        },
    .num_frames = 1,
};

static const struct texture tex_none = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){ TILE(0, 3, 0), TILE(0, 3, 0), TILE(0, 3, 0), TILE(0, 3, 0) },
    .num_frames = 1,
};

static const struct texture tex_mario_stand = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){60, 62, 61, 63},
    .num_frames = 1,
};  

static const struct texture tex_mario_run = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(10, 12, 4), TILE(10, 12, 6), TILE(10, 12, 5),
                          TILE(10, 12, 7), TILE(10, 12, 0), TILE(10, 12, 2),
                          TILE(10, 12, 1), TILE(10, 12, 3)},
    .num_frames = 2,
};

static const struct texture tex_mario_turn = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(10, 14, 0), TILE(10, 14, 2), TILE(10, 14, 1),
                          TILE(10, 14, 3)},
    .num_frames = 1,
};

static const struct texture tex_mario_jump = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(10, 13, 8), TILE(10, 13, 10), TILE(10, 13, 9),
                          TILE(10, 13, 11)},
    .num_frames = 1,
};


static const struct texture tex_mario_hold = {
    .height = 2,
    .width = 2,
    .tiles =
        (uint16_t[]){
            TILE(10, 11, 4),
            TILE(10, 11, 6),
            TILE(10, 11, 5),
            TILE(10, 11, 7),
            TILE(10, 11, 0),
            TILE(10, 11, 2),
            TILE(10, 11, 1),
            TILE(10, 11, 3),
        },
    .num_frames = 2,
};

static const struct texture tex_box = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(1, 17, 6), TILE(1, 17, 7), TILE(1, 18, 6),
                          TILE(1, 18, 7)},
    .num_frames = 1,
};

uint16_t font[256] = {0};


#define box entities[1]
#define enemy1 entities[2]

#define spr_mario (mario.sprite)
#define ent_mario (mario.entity)

const float MAX_VEL_X = 3;
const float MAX_VEL_Y = 5;
const float MIN_VEL_X = 0.01;
const float MARIO_FRICTION = 4;
const float MARIO_FRICTION_MULT = -0.3;
const float MARIO_RUN_VEL = 5;
const float MARIO_JUMP_RUN_VEL = 2.5;
const float FALL_ACCEL = 15;
const float JUMP_VEL = 5;
const float JUMP_ACCEL_MULT_UP = 0.5;
const float JUMP_ACCEL_MULT_DOWN = 0.8;
const float THROW_UP_VEL = 3;
const float THROW_X_VEL = 3;
const float PUT_X_VEL = 0.5;


struct entity entities[NUM_ENTITIES] = {
    [0]={
    .sprite =
        {
            .texture = &tex_mario_run,
            .palette = 0,
            .attributes = FLIP_X,
            .hitbox =
                {
                    .x = 2, //.x = 140,
                    .y = 0,
                    .height = 16,
                    .width = 12 ,
                },
        },

    .falling = false,
},0

};
#define mario entities[0]
// struct sprite* spr_mario = &mario.entity.sprite;

uint32_t palette[256] = {
    0xFFFDBB90, 0xFF0000E0, 0xFF95D3FF, 0xFF000000, //mario
    0xFFFFFFFF, 0xFF000000, 0xFF46DD4B, 0xFF00A900, //pipe
    0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF3599FC, //qmark
    0xFFFFFFFF, 0xFF000000, 0xFFB1BDFC, 0xFF004E99, //brick
    0xFFFDBB90, 0xFF000000, 0xFF3599FC, 0xFF044bc9, //vwood
    0xFFFDBB90, 0xFF000000, 0xFFFFFFFF, 0xFF46DD4B, //powerup
};

extern const uint8_t _binary_smb3_chr_start[];
extern const uint8_t _binary_smb3_chr_end[];

uint32_t gametime = 0;

void init();
void update(struct input, uint32_t);
void draw();

struct rom rom = {
    .init = init,
    .update = update,
    .draw = draw,
    .running = true,
};

void jump() {
    // spr_mario.texture = &tex_mario_jump;
    mario.y_vel = -JUMP_VEL;
}

void run() {
    // spr_mario.texture = &tex_mario_run;
    if (!mario.holding && colliding_x(&mario.sprite, &box.sprite) &&
        colliding_y(&mario.sprite, &box.sprite)) {
        spr_mario.texture = &tex_mario_hold;
        mario.holding = 1;
        box.holding = 1;
    }
}

extern uint8_t camera_x;
extern uint8_t camera_y;
extern uint8_t active_screen;

uint8_t tile_attr = 0;

static uint16_t camera_world_x(void) {
    return (uint16_t)active_screen * CAMERA_WIDTH + camera_x;
}

static uint16_t wrap_world_x(int32_t x) {
    const int32_t world_width = N_SCREENS * CAMERA_WIDTH;
    x %= world_width;
    if (x < 0)
        x += world_width;
    return (uint16_t)x;
}

int16_t entity_screen_x(const struct entity *entity) {
    const int16_t world_width = N_SCREENS * CAMERA_WIDTH;
    int16_t entity_x = (int16_t)entity->world_screen * CAMERA_WIDTH + entity->world_x;
    int16_t dx = entity_x - (int16_t)camera_world_x();

    if (dx > world_width / 2)
        dx -= world_width;
    else if (dx < -(world_width / 2))
        dx += world_width;

    return dx;
}

bool entity_in_sim_zone(const struct entity *entity) {
    if (entity->type == ENTITY_NONE || entity->type == ENTITY_KILLED)
        return false;

    int16_t x = entity_screen_x(entity);
    return x >= 0 && x < SIM_ZONE_WIDTH;
}

bool entity_in_draw_zone(const struct entity *entity) {
    if (entity->type == ENTITY_NONE || entity->type == ENTITY_KILLED)
        return false;

    int16_t x = entity_screen_x(entity);
    return x > -16 && x < CAMERA_WIDTH;
}



void move_entity(struct entity *entity) {
    entity->x_accumulator += entity->x_vel;
    entity->y_accumulator += entity->y_vel;

    int32_t x_pix = (int32_t)entity->x_accumulator;
    int32_t y_pix = (int32_t)entity->y_accumulator;

    entity->x_accumulator -= x_pix;
    entity->y_accumulator -= y_pix;

    int16_t absolute_x = get_world_x(entity);
    absolute_x += x_pix;

    if (entity->sprite.hitbox.height > 0 && entity->sprite.hitbox.width > 0){
        absolute_x = collide_entity_x(entity, absolute_x, x_pix);
    }
    
    set_world_x(entity, absolute_x);

    entity->sprite.y += y_pix;
    if (entity->sprite.hitbox.height > 0 && entity->sprite.hitbox.width > 0){
        collide_entity_y(entity, absolute_x, y_pix);
    }
    else{
        entity->falling = true;
    }
}

void init() {
    engine.tileset = (struct tileset){
        .data = _binary_smb3_chr_start,
    };

    engine.palette = palette;
    mario.type = ENTITY_PLAYER;
    mario.world_screen = active_screen;
    mario.world_x = camera_x + mario.sprite.x;

    entities[1] = (struct entity){
        .type = ENTITY_BOX,
        .world_screen = 0,
        .world_x = 4,
        .sprite =
            {
                .texture = &tex_box,
                .palette = 0,
                .attributes = 0,
                .x = 4,
                .y = 0,
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
    
    enemy_init_at(&entities[2], 0, 192, 128, ENEMY_GOOMBA);
    enemy_init_at(&entities[3], 1, 220, 128, ENEMY_TURTLE);
    enemy_init_at(&entities[4], 2, 50, 128, ENEMY_DUCK);
    enemy_init_at(&entities[5], 3, 100, 128, ENEMY_FIREBALL_SHOOTER);
    
    for (int i = 6; i < NUM_ENTITIES; i++) {
        entities[i].type = ENTITY_NONE;
    }

    font['0'] = TILE(11, 31, 0);
    font['1'] = TILE(11, 31, 1);
    font['2'] = TILE(11, 31, 2);
    font['3'] = TILE(11, 31, 3);
    font['4'] = TILE(11, 31, 4);
    font['5'] = TILE(11, 31, 5);
    font['6'] = TILE(11, 31, 6);
    font['7'] = TILE(11, 31, 7);
    font['8'] = TILE(11, 31, 8);
    font['9'] = TILE(11, 31, 9);
    font['/'] = TILE(11, 31, 10);
    font['*'] = TILE(11, 31, 11);

    font['.'] = TILE(11, 30, 9);
    font['!'] = TILE(11, 30, 10);
    font['?'] = TILE(11, 30, 11);
    font['$'] = TILE(11, 30, 12);

    font['A'] = TILE(14, 30, 0);
    font['B'] = TILE(14, 30, 1);
    font['C'] = TILE(14, 30, 2);
    font['D'] = TILE(14, 30, 3);
    font['E'] = TILE(14, 30, 4);
    font['F'] = TILE(14, 30, 5);
    font['G'] = TILE(14, 30, 8);
    font['H'] = TILE(14, 30, 9);
    font['I'] = TILE(14, 30, 10);
    font['J'] = TILE(14, 30, 11);

    font['K'] = TILE(14, 31, 0);
    font['L'] = TILE(14, 31, 1);
    font['M'] = TILE(14, 31, 2);
    font['N'] = TILE(14, 31, 3);
    font['O'] = TILE(14, 31, 4);
    font['P'] = TILE(14, 31, 5);

    font['Q'] = TILE(14, 30, 14);
    font['R'] = TILE(14, 30, 15);

    font['S'] = TILE(14, 31, 8);
    font['T'] = TILE(14, 31, 9);
    font['U'] = TILE(14, 31, 10);
    font['V'] = TILE(14, 31, 11);

    font['W'] = TILE(14, 29, 14);

    font['X'] = TILE(14, 31, 13);
    font['Y'] = TILE(14, 31, 14);
    font['Z'] = TILE(14, 31, 15);
    font[' '] = TILE(14, 23, 15);

    font['-'] = TILE(2, 28, 14);

    load_map(map);
}

void entity_fall_kill(struct entity* entity){
    for(int8_t i = 0; i < NUM_ENTITIES; i++){
        if((entity+i)->type != ENTITY_NONE && (entity+i)->type != ENTITY_KILLED){
            if (i == 0 || (entity+i)->sim_active) {
                if ((entity+i)->sprite.y > 240) {
                    (entity+i)->sprite.texture = &tex_none;
                    (entity+i)->type = ENTITY_KILLED;
                    fprintf(stderr, "ENTITY KILLED by fall%d\n", i);
                }
            }
        }
    }
}
bool had_y = false;
uint32_t cnt_update = 0;
int16_t center;

void update(struct input input, uint32_t time) {
    float deltatime = (float)(time - gametime) / 1000;
    gametime = time;

    // if (!(time % 500)) {
    //     next_frame(&spr_mario);
    // }
    if (input.start) {
        rom.running = false;
    }
    if (mario.type == ENTITY_KILLED) {
        return;
    }
    if (input.b && !mario.falling) {
        jump();
    }

    if (input.a) {
        run();
    } else {
        if (mario.holding) {
            if (input.y < 0) {
                entities[mario.holding].y_vel = -THROW_UP_VEL;
            } else if (input.y > 0) {
                entities[mario.holding].x_vel = PUT_X_VEL * input.x;
            } else {
                entities[mario.holding].x_vel = THROW_X_VEL * input.x;
            }
            entities[mario.holding].holding = 0;
            mario.holding = 0;
        }
    }


    // box.falling = box.sprite.y < 200;
    // mario.falling = spr_mario.y < 11 * 16;

    float x_accel;

    // camera_move(input.y, input.x);

    if (mario.falling) {
        x_accel = input.x * MARIO_JUMP_RUN_VEL * deltatime;
        if (input.b) {
            if (mario.y_vel > 0)
                mario.y_vel += FALL_ACCEL * JUMP_ACCEL_MULT_DOWN * deltatime;
            else
                mario.y_vel += FALL_ACCEL * JUMP_ACCEL_MULT_UP * deltatime;
        } else
            mario.y_vel += FALL_ACCEL * deltatime;
    } else {
        x_accel = input.x * MARIO_RUN_VEL * deltatime;

        mario.y_vel = mario.y_vel > 0 ? 0 : mario.y_vel;
        // mario.x_vel -= (1 - MARIO_FRICTION_MULT) * deltatime * mario.x_vel;
        // float friction = mario.x_vel * MARIO_FRICTION_MULT * deltatime;
        // mario.x_vel += friction;

        if (mario.x_vel > 0) {
            mario.x_vel = MAX(mario.x_vel - MARIO_FRICTION * deltatime, 0);
        } else if (mario.x_vel < 0) {
            mario.x_vel = MIN(mario.x_vel + MARIO_FRICTION * deltatime, 0);
        }

        // if (ABS(mario.x_vel) < MIN_VEL_X) {
        //     mario.x_vel = 0;
        // }
    }

    mario.x_vel = CLAMP(mario.x_vel + x_accel, -MAX_VEL_X, MAX_VEL_X);
    mario.y_vel = CLAMP(mario.y_vel, -MAX_VEL_Y, MAX_VEL_Y);


    if (input.x > 0) {
        spr_mario.attributes |= FLIP_X;
        if (mario.x_vel < 0) {
            spr_mario.texture = &tex_mario_turn;
        } else {
            spr_mario.texture = &tex_mario_run;
        }
    } else if (input.x < 0) {
        spr_mario.attributes &= ~FLIP_X;
        if (mario.x_vel < 0) {
            spr_mario.texture = &tex_mario_run;
        } else {
            spr_mario.texture = &tex_mario_turn;
        }
    } else {
        // spr_mario.texture = &tex_mario_stand;
    }

    if (mario.holding) {
        spr_mario.texture = &tex_mario_hold;
    } else {
        if (mario.x_vel * input.x < 0) {
            spr_mario.texture = &tex_mario_turn;
        } else {
            spr_mario.texture = &tex_mario_run;
        }

        if (mario.y_vel > 0) {
            spr_mario.texture = &tex_mario_jump;
        } else if (mario.y_vel < 0) {
            spr_mario.texture = &tex_mario_jump;
        }
    }

    if (mario.falling) {
        spr_mario.frame = 0;
        next_frame(&mario.sprite);
    } else if (mario.x_vel != 0) {
        if (!(cnt_update % 10))
            next_frame(&mario.sprite);
    } else {
        spr_mario.frame = 0;
    }

    move_entity(&mario);
    
    entity_world_to_sprite(&mario, active_screen, camera_x);
    center = mario.sprite.x - 128;
    camera_move(0, center);

    for (int i = 2; i < NUM_ENTITIES; i++) {
        if (entities[i].type == ENTITY_ENEMY) {
            enemy_update(&entities[i], deltatime);
        } else if (entities[i].type == ENTITY_PROJECTILE) {
            if (entities[i].state_timer > 0) {
                entities[i].state_timer--;
                
                int16_t dist = real_world_distance(entities[i].world_screen, entities[i].world_x, active_screen, camera_x);
                entities[i].sim_active = (dist >= -128 && dist < 384);
                
                if (entities[i].sim_active) {
                    move_entity(&entities[i]);
                }
                
                if (entities[i].state_timer == 0) {
                    entities[i].type = ENTITY_KILLED;
                }
            }
        }
    }
    
    for (int i = 0; i < NUM_ENTITIES; i++) {
        if (entities[i].type != ENTITY_NONE && entities[i].type != ENTITY_KILLED) {
            entity_world_to_sprite(&entities[i], active_screen, camera_x);
        }
    }
    
    handle_entity_collisions(&mario, entities, NUM_ENTITIES);
    entity_fall_kill(entities);

    if (mario.holding) {
        if (mario.sprite.attributes & FLIP_X)
            entities[mario.holding].sprite.x = mario.sprite.x +
                                               mario.sprite.hitbox.x +
                                               mario.sprite.hitbox.width;
        else
            entities[mario.holding].sprite.x =
                mario.sprite.x + mario.sprite.hitbox.x -
                entities[mario.holding].sprite.hitbox.width;

        entities[mario.holding].sprite.y = mario.sprite.y;
    }

    // if (mario.x_vel > 1) {
    //     mario.x_vel -= MARIO_FRICTION * deltatime;
    // } else if (mario.x_vel < -1) {
    //     mario.x_vel += MARIO_FRICTION * deltatime;
    // } else {
    //     mario.x_vel = 0;
    // }

    // if (input.x && !(cnt_update % 20)) {
    //     next_frame(&mario.sprite);
    // }
    // fprintf(stderr, "MARIO X %d Y %d VEL X %f VEL Y %f\n", mario.sprite.x,
    //         mario.sprite.y, mario.x_vel, mario.y_vel);
    cnt_update++;
}

void draw() {
    // for (int y = 0; y < 32; y++) {
    //     for (int x = 0; x < 16; x++) {
    //         draw_tile(y * 16 + x, y * 8, x * 8, 0, 0);
    //         // draw_tile(TILE(page, y, x), y * 8, x * 8, 0, 0);
    //     }
    // }
    clear_pixelbuf();

    camera_draw();

    char buf[64];
    // snprintf(buf, 64, "X VEL %f", mario.x_vel);
    // draw_chars(buf, font, 0, 0, 0);
    // snprintf(buf, 64, "Y VEL %f", mario.y_vel);
    // draw_chars(buf, font, 1, 0, 0);

    // snprintf(buf, 64, "COLLIDING X %d",
    //          colliding_x(&mario.sprite, &box.sprite));
    // draw_chars(buf, font, 2, 0, 0);
    // snprintf(buf, 64, "COLLIDING Y %d",
    //          colliding_y(&mario.sprite, &box.sprite));
    // draw_chars(buf, font, 3, 0, 0);

    // snprintf(buf, 64, "X VEL %f", box.x_vel);
    // draw_chars(buf, font, 4, 0, 0);
    // snprintf(buf, 64, "Y VEL %f", box.y_vel);
    // draw_chars(buf, font, 5, 0, 0);

    snprintf(buf, 64, "CAMERA X %u", camera_x);
    draw_chars(buf, font, 0, 0, 0);
    snprintf(buf, 64, "CAMERA Y %u", camera_y);
    draw_chars(buf, font, 1, 0, 0);
    snprintf(buf, 64, "ACTIVE SCREEN %u", active_screen);
    draw_chars(buf, font, 2, 0, 0);

    // draw_sprite(&box.sprite);
    if (mario.type != ENTITY_KILLED) {
        draw_sprite(&spr_mario);
    }

    for (int i = 2; i < NUM_ENTITIES; i++) {
        if (entities[i].type == ENTITY_ENEMY) {
            enemy_draw(&entities[i]);
        } else if (entities[i].type == ENTITY_PROJECTILE) {
            if (entities[i].sim_active && entities[i].sprite.x >= 0 && entities[i].sprite.x < CAMERA_WIDTH) {
                draw_sprite(&entities[i].sprite);
            }
        }
    }

    // next_frame(&mario.entity.sprite);
}
