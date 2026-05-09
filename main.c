#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define TRANSFORM_SIZE 2048
#define NUM_BARS 160 // 800px width / (4px width + 1px gap)

float sample_buffer[TRANSFORM_SIZE];
int buffer_index = 0;

// Track smoothing per visual bar, not per frequency bin
float smoothed_heights[NUM_BARS] = {0}; 
const int SAMPLE_RATE = 44100;

void capture_audio(int chan, void *stream, int len, void *udata) {
    int16_t *samples = (int16_t *)stream;
    int num_samples = len / sizeof(int16_t);

    for (int i = 0; i < num_samples; i += 2) {
        float left = samples[i] / 32768.0f;
        sample_buffer[buffer_index] = left;
        buffer_index = (buffer_index + 1) % TRANSFORM_SIZE;
    }
}

void draw_dft(SDL_Renderer* renderer) {
    float windowed[TRANSFORM_SIZE];
    
    // Unwrap the circular buffer
    for (int n = 0; n < TRANSFORM_SIZE; n++) {
        float window = 0.5f * (1.0f - cosf(2.0f * M_PI * n / (TRANSFORM_SIZE - 1)));
        int idx = (buffer_index + n) % TRANSFORM_SIZE;
        windowed[n] = sample_buffer[idx] * window;
    }

    int bar_width = 4;
    int gap = 1;

    // Iterate over visual bars, NOT frequency bins
    for (int b = 0; b < NUM_BARS; b++) {
        
        // Logarithmic Mapping: Translate uniform visual bars backwards into frequency bins (k)
        float t = (float)b / (NUM_BARS - 1);
        float log_min = log10f(1.0f);
        float log_max = log10f((float)(TRANSFORM_SIZE / 2 - 1));
        
        float k_float = powf(10.0f, log_min + t * (log_max - log_min));
        int k_int = (int)roundf(k_float);

        if (k_int < 1) k_int = 1;
        if (k_int >= TRANSFORM_SIZE / 2) k_int = TRANSFORM_SIZE / 2 - 1;

        // Calculate DFT only for the specific bin we mapped to
        float sum_real = 0.0f;
        float sum_imag = 0.0f;
        for (int n = 0; n < TRANSFORM_SIZE; n++) {
            float theta = (2.0f * M_PI * k_int * n) / TRANSFORM_SIZE;
            sum_real += windowed[n] * cosf(theta);
            sum_imag += windowed[n] * sinf(theta);
        }

        float magnitude = sqrtf(sum_real * sum_real + sum_imag * sum_imag);

        // Your original EQ boost math
        float eq_boost = log10f((float)k_int * 10.0f); 
        float target_h = sqrtf(magnitude) * 22.5f * eq_boost;

        // Snappy attack, smooth decay
        if (target_h > smoothed_heights[b]) {
            smoothed_heights[b] += (target_h - smoothed_heights[b]) * 0.6f;
        } else {
            smoothed_heights[b] -= (smoothed_heights[b] - target_h) * 0.15f;
        }

        int h = (int)smoothed_heights[b];
        if (h > 600) h = 600;
        if (h < 2) h = 2; // Keep a 2px nub so silent bars are still visible

        int x = b * (bar_width + gap);

        SDL_Rect bar = { x, 600 - h, bar_width, h };

        // Restored your original colors
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        SDL_RenderFillRect(renderer, &bar);
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return 1;

    if (Mix_OpenAudio(SAMPLE_RATE, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return 1;
    
    Mix_RegisterEffect(MIX_CHANNEL_POST, capture_audio, NULL, NULL);

    Mix_Music* music = Mix_LoadMUS("assets/music.mp3");
    if (!music) {
        printf("Failed to load music!\n");
        return 1;
    }
    Mix_PlayMusic(music, -1);

    SDL_Window* window = SDL_CreateWindow("DFT Visualizer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        // Restored your original background color
        SDL_SetRenderDrawColor(renderer, 33, 33, 33, 255); 
        SDL_RenderClear(renderer);

        draw_dft(renderer);

        SDL_RenderPresent(renderer);
    }

    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}