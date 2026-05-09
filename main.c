#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define TRANSFORM_SIZE 4096
#define NUM_BARS 160 

float sample_buffer[TRANSFORM_SIZE];
int buffer_index = 0;

float smoothed_heights[NUM_BARS] = {0}; 
const int SAMPLE_RATE = 44100;

float dft(float* windowed, int k) {
    if (k < 0 || k >= TRANSFORM_SIZE / 2) return 0.0f;
    float sum_real = 0.0f;
    float sum_imag = 0.0f;
    for (int n = 0; n < TRANSFORM_SIZE; n++) {
        float theta = (2.0f * M_PI * k * n) / TRANSFORM_SIZE;
        sum_real += windowed[n] * cosf(theta);
        sum_imag += windowed[n] * sinf(theta);
    }
    return sqrtf(sum_real * sum_real + sum_imag * sum_imag);
}

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
        for (int n = 0; n < TRANSFORM_SIZE; n++) {
            float window = 0.5f * (1.0f - cosf(2.0f * M_PI * n / (TRANSFORM_SIZE - 1)));
            windowed[n] = sample_buffer[(buffer_index + n) % TRANSFORM_SIZE] * window;
        }

    int bar_width = 4;
    int gap = 0;

    for (int b = 0; b < NUM_BARS; b++) {
        float t = (float)b / (NUM_BARS - 1);
        float log_min = log10f(1.0f);
        float log_max = log10f((float)(TRANSFORM_SIZE / 2 - 1));
        
        float k_exact = powf(10.0f, log_min + t * (log_max - log_min));
        int k0 = (int)floorf(k_exact);
        int k1 = k0 + 1;
        float fraction = k_exact - k0;

        float mag0 = dft(windowed, k0);
        float mag1 = dft(windowed, k1);
        float magnitude = mag0 + fraction * (mag1 - mag0);

        float eq_boost = log10f(k_exact * 10.0f); 
        float target_h = sqrtf(magnitude) * 22.5f * eq_boost;
        
        if (target_h > smoothed_heights[b]) {
            smoothed_heights[b] += (target_h - smoothed_heights[b]) * 0.5f;
        } else {
            smoothed_heights[b] -= (smoothed_heights[b] - target_h) * 0.4f;
        }

        int h = (int)smoothed_heights[b];
        if (h > 600) h = 600;
        
        int x = b * (bar_width + gap);

        SDL_Rect bar = {
            x,
            600 - h,
            bar_width,
            h
        };

        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); 
        SDL_RenderFillRect(renderer, &bar);
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return 1;

    if (Mix_OpenAudio(SAMPLE_RATE, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return 1;
    
    Mix_RegisterEffect(MIX_CHANNEL_POST, capture_audio, NULL, NULL);

    Mix_Music* music = Mix_LoadMUS("assets/music.mp3");
    if (!music) return 1;
    Mix_PlayMusic(music, -1);

    SDL_Window* window = SDL_CreateWindow("DFT Visualizer", 100, 100, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_dft(renderer);

        SDL_RenderPresent(renderer);
    }

    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}