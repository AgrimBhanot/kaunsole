#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "../../src/collision.h"
#include "../../src/engine.h"
#include "../../src/rom.h"
#include "../../src/sprite.h"

#include "defs.h"
#include "obj.h"
#include "camera.h"

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
    O2(0, 0, 8, 5, 4, 0),
    O2(17, 6, 7, 2, 5, 0),
    O2(10, 0, 2, 2, 2, 0),
    O2(14, 0, 4, 2, 2, 0),
    O2(11, 0, 6, 2, 2, 0),
    O2(15, 0, 13, 15, 1, 0),
    O2(16, 0, 12, 15, 1, 0) | O2_NEXT_PAGE,
    O2(0, 0, 8, 5, 5, 0),
    O2(2, 0, 12, 15, 2, 0) | O2_NEXT_PAGE,
    // O2(0, 0, 8, 3, 5, 0),
    // O2(2, 0, 12, 15, 2, 0) | O2_NEXT_PAGE,
    // O2_INDEX(0) | O2_Y(3) | O2_X(7) | O2_WIDTH(7) | O2_HEIGHT(5) | O2_PALETTE(0), 
    // O2_INDEX(1) | O2_Y(8) | O2_X(8) | O2_WIDTH(4) | O2_HEIGHT(5) | O2_PALETTE(0) | O2_NEXT_PAGE, 
    0,
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

static const struct texture tex_mario_die = {
    .height = 2,
    .width = 2,
    .tiles = (uint16_t[]){TILE(10, 12, 12), TILE(10, 12, 12) | 1 << 15,
                          TILE(10, 12, 13), TILE(10, 12, 13) | 1 << 15},
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
    .tiles = (uint16_t[]){TILE(1, 17, 12), TILE(1, 18, 12), TILE(1, 17, 13),
                          TILE(1, 18, 13)},
    .num_frames = 1,
};

uint16_t font[256] = {0};

struct entity mario = {
    .sprite =
        {
            .texture = &tex_mario_run,
            .palette = 0,
            .attributes = FLIP_X,
            .hitbox =
                {
                    .x = 140,
                    .y = 0,
                    .height = 16,
                    .width = 10,
                },
        },

    .falling = false,
};

#define box entities[1]

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

#define NUM_ENTITIES 8

struct entity entities[NUM_ENTITIES] = {0};

// struct sprite* spr_mario = &mario.entity.sprite;

uint32_t palette[256] = {
    0xFFFFFFFF, 0xFF0000E0, 0xFF95D3FF, 0xFF000000,
    0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000,
    0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000,
    0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000,
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

uint8_t tile_attr = 0;

void move_entity(struct entity *entity) {
    entity->x_accumulator += entity->x_vel;
    entity->y_accumulator += entity->y_vel;

    int32_t x_pix = (int32_t)entity->x_accumulator;
    int32_t y_pix = (int32_t)entity->y_accumulator;

    entity->x_accumulator -= x_pix;
    entity->y_accumulator -= y_pix;

    // collide_entity(&mario, y_pix, x_pix);

    entity->sprite.x += x_pix;
    entity->sprite.y += y_pix;

}

void init() {
    engine.tileset = (struct tileset){
        .data = _binary_smb3_chr_start,
    };

    engine.palette = palette;

    entities[1] = (struct entity){
        .sprite =
            {
                .texture = &tex_box,
                .palette = 0,
                .attributes = 0,
                .x = 40,
                .hitbox =
                    {
                        .x = 0,
                        .y = 0,
                        .height = 16,
                        .width = 16,
                    },
            },

        .falling = false,
    };

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

bool had_y = false;
uint32_t cnt_update = 0;

void update(struct input input, uint32_t time) {
    float deltatime = (float)(time - gametime) / 1000;
    gametime = time;

    // if (!(time % 500)) {
    //     next_frame(&spr_mario);
    // }

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

    if (input.start) {
        rom.running = false;
    }

    // box.falling = box.sprite.y < 200;
    mario.falling = spr_mario.y < 11 * 16;

    if (box.falling && !box.holding) {
        box.y_vel += FALL_ACCEL * deltatime;
    } else {
        box.y_vel = box.y_vel > 0 ? 0 : box.y_vel;
    }

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
    box.y_vel = CLAMP(box.y_vel, -MAX_VEL_Y, MAX_VEL_Y);

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
            // spr_mario.texture = &tex_mario_die;
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

    if (!box.holding)
        move_entity(&box);


    move_entity(&mario);
    int16_t center = mario.sprite.x - 128;
    mario.sprite.x -= center;
    camera_move(0, center);

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

    cnt_update++;
}
extern uint8_t camera_x;
extern uint8_t camera_y;
extern uint8_t active_screen;

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
    draw_sprite(&spr_mario);
    // next_frame(&mario.entity.sprite);
}
