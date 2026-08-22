#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

#include "../util.h"
#include "../backend.h"
#include "../graphics.h"

#define SDL_WINDOW_HEIGHT 256
#define SDL_WINDOW_WIDTH 256
// #define SDL_WINDOW_HEIGHT 1080
// #define SDL_WINDOW_WIDTH 1920

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_AudioStream *audio_stream;

struct input input = {0};

void backend_init() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    }
    if (!SDL_CreateWindowAndRenderer("examples/demo/snake", SDL_WINDOW_WIDTH,
                                     SDL_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE,
                                     &window, &renderer)) {
    }
    SDL_SetRenderLogicalPresentation(renderer, SDL_WINDOW_WIDTH,
                                     SDL_WINDOW_HEIGHT,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                SDL_TEXTUREACCESS_STREAMING, X_RESOLUTION,
                                Y_RESOLUTION);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    SDL_AudioSpec spec = {
        .format = SDL_AUDIO_S16,
        .channels = 1,
        .freq = 48000, // 800 samples per frame
    };
    audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                             &spec, NULL, NULL);
    SDL_ResumeAudioStreamDevice(audio_stream);
}

void backend_deinit() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyAudioStream(audio_stream);
}

void backend_render(uint8_t pixelbuf[Y_RESOLUTION][X_RESOLUTION],
                    uint32_t *palette) {
    uint32_t framebuffer[Y_RESOLUTION][X_RESOLUTION];
    for (int y = 0; y < Y_RESOLUTION; y++) {
        for (int x = 0; x < X_RESOLUTION; x++) {
            framebuffer[y][x] = palette[pixelbuf[y][x]];
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_UpdateTexture(texture, NULL, framebuffer, X_RESOLUTION * 4);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void backend_audio(const void *buffer, uint16_t length) {
    SDL_PutAudioStreamData(audio_stream, buffer, length);
}

uint32_t backend_time() { return SDL_GetTicks(); }

struct input backend_input() {
    // struct input input = {0};
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        // case SDL_EVENT_QUIT:
        case SDL_EVENT_KEY_UP:
            switch (event.key.scancode) {
            case SDL_SCANCODE_W:
                if (input.y == -1)
                    input.y = 0;
                break;
            case SDL_SCANCODE_S:
                if (input.y == 1)
                    input.y = 0;
                break;
            case SDL_SCANCODE_R:
                if (input.x == -1)
                    input.x = 0;
                break;
            case SDL_SCANCODE_T:
                if (input.x == 1)
                    input.x = 0;
                break;
            case SDL_SCANCODE_N:
                input.a = false;
                break;
            case SDL_SCANCODE_E:
                input.b = false;
                break;
            case SDL_SCANCODE_I:
                input.start = false;
                break;
            case SDL_SCANCODE_O:
                input.select = false;
                break;
            default:
                break;
            }
            break;
        case SDL_EVENT_KEY_DOWN:
            switch (event.key.scancode) {
            case SDL_SCANCODE_W:
                input.y = -1;
                break;
            case SDL_SCANCODE_S:
                input.y = 1;
                break;
            case SDL_SCANCODE_R:
                input.x = -1;
                break;
            case SDL_SCANCODE_T:
                input.x = 1;
                break;
            case SDL_SCANCODE_N:
                input.a = true;
                break;
            case SDL_SCANCODE_E:
                input.b = true;
                break;
            case SDL_SCANCODE_I:
                input.start = true;
                break;
            case SDL_SCANCODE_O:
                input.select = true;
                break;
            default:
                break;
            }
            break;
        }
    }
    return input;
}

// uint8_t backend_load_roms(char paths[8][64]) {
//     DIR *dir = opendir("build/roms");

//     struct dirent *ent;
//     uint8_t i = 0;
//     while ((ent = readdir(dir))) {
//         if (ent->d_type == DT_REG) {
//             snprintf(paths[i], 64, "build/roms/%s", ent->d_name);
//             i++;
//         }
//     }
//     closedir(dir);
//     return i;
// }


// uint8_t backend_load_roms(char paths[8][64]) { // orginal version didnt work on ubuntu
//     DIR *dir = opendir("build/roms");
//     if (!dir) return 0;

//     struct dirent *ent;
//     uint8_t i = 0;
//     while ((ent = readdir(dir)) && i < 8) {
//         char fullpath[128];
//         snprintf(fullpath, sizeof(fullpath), "build/roms/%s", ent->d_name);

//         struct stat st; // holds metadata about file, like timestamp, size, type
//         if (stat(fullpath, &st) == 0 && S_ISREG(st.st_mode)) {// stat fills st, S_ISREG checks if it's a regular file
//             snprintf(paths[i], 64, "%s", fullpath); 
//             printf("%s",paths[i]);
//             i++;
//         }
//     }

//     closedir(dir);
//     return i;
// }
uint8_t backend_load_roms(char paths[8][64]) {
    // Look inside build/roms relative to the project root
    DIR *dir = opendir("build/roms");
    if (!dir) {
        LOG("Failed to open build/roms directory");
        return 0;
    }

    struct dirent *ent;
    uint8_t i = 0;
    while ((ent = readdir(dir)) && i < 8) {
        // Skip hidden entries like '.' and '..'
        if (ent->d_name[0] == '.') continue;

        char fullpath[128];
        // Store the correct path including the 'build/' prefix for dlopen
        snprintf(fullpath, sizeof(fullpath), "build/roms/%s", ent->d_name);

        struct stat st;
        if (stat(fullpath, &st) == 0 && S_ISREG(st.st_mode)) {
            snprintf(paths[i], 64, "%s", fullpath);
            LOG("Found ROM: %s", paths[i]);
            i++;
        }
    }

    closedir(dir);
    return i;
}


void backend_sleep(uint32_t ms) {
    SDL_Delay(ms);
}
 
