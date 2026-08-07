#include "engine.h"
#include "audio.h"
#include "backend.h"
#include "graphics.h"
#include "rom.h"
#include "util.h"
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#define FRAME_MS (1000 / 60)

struct engine engine = {0};

// void compute_audio(uint16_t num_samples) {
//     int16_t samples[num_samples];
//     memset(samples, 0, num_samples);

//     for (int i = 0; i < AUDIO_QUEUE_LEN; i++) {
//         struct audio_event *event = &engine.audio_queue[i];
//         if (event->priority != 0) {
//             uint16_t to_read;
//             uint16_t left =
//                 event->sampler.sample->length - event->sampler.position;
//             if (left < num_samples) {
//                 event->priority = 0;
//                 to_read = left;
//             } else
//                 to_read = num_samples;

//             for (int i = 0; i < to_read; i++) {
//                 samples[i] =
//                     MIN(samples[i] + event->sampler.sample->data[i], INT16_MAX);
//             }
//         }
//     }

//     // push samples to ringbuffer
// }

void mainloop(struct rom *rom) {
    uint32_t gametime = 0;
    rom->init();
    LOG("inited");

    while (rom->running) {
        struct input i = backend_input();
        uint32_t time = backend_time();

        rom->update(i, time);

        // uint32_t update_time = backend_time() - time;

        // if (update_time > FRAME_MS) {
        // } else {
        rom->draw();
        // }

        // time = backend_time();
        // gametime = time;

        uint16_t audio_samples = 800; // calculate samples from deltatime
        // compute_audio(audio_samples);

        backend_render(engine.pixelbuf, engine.palette);

        uint32_t deltatime = backend_time() - time;
        if (deltatime < FRAME_MS) {
            backend_sleep(FRAME_MS - deltatime);
        }
    }
}

struct input prev_input = {0};
uint8_t rom_index = 0;
uint8_t num_roms = 0;
char rom_names[8][64];
struct rom *roms[8] = {0};

void ui_init() {
    num_roms = backend_load_roms(rom_names);
    LOG("roms: %d", num_roms);
    // init_palettes();
    // load_tileset(font);
}

struct rom ui_rom;

void ui_update(struct input input, uint32_t time) {
    if (prev_input.y == 0 && input.y != 0) {
        rom_index = (rom_index + input.y + num_roms) % num_roms;
        LOG("input: x:%d, y:%d, a:%b, b:%b, index:%d", input.x, input.y,
            input.a, input.b, rom_index);
    }

    if (input.a) {
        LOG("opening: %s", rom_names[rom_index]);
        void *so = dlopen(rom_names[rom_index], RTLD_LAZY);
        if (!so)
            LOG("dlopen failed");
        struct rom *rom = dlsym(so, "rom");
        if (!rom)
            LOG("dlsym failed");
        mainloop(rom);
    }

    if (input.b)
        ui_rom.running = false;

    prev_input = input;
}

void ui_draw() {
    for (int i = 0; i < num_roms; i++) {
        bool selected = rom_index == i;
        // draw_text_palette(rom_names[i], selected ? 1 : 0, i * 8, 0);
    }
}

struct rom ui_rom = {
    .init = ui_init,
    .update = ui_update,
    .draw = ui_draw,
    .running = true,
};

int main(int argc, const char **argv) {
    const char *rom_name = argv[1];
    LOG("opening: %s", rom_name);
    backend_init();

    void *so = dlopen(rom_name, RTLD_LAZY);
    if (!so) {
        LOG("dlopen failed");
        const char *e = dlerror();
        fprintf(stderr, "dlopen failed: %s\n", e ? e : "unknown");
    }
    struct rom *rom = dlsym(so, "rom");
    if (!rom)
        LOG("dlsym failed");

    mainloop(rom);

    // mainloop(&ui_rom);
}
