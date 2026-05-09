#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define FFT_SIZE 1024
float sample_buffer[FFT_SIZE];
int buffer_index = 0;

const int SAMPLE_RATE = 44100;

void capture_audio(int chan, void *stream, int len, void *udata) {
    int16_t *samples = (int16_t *)stream;
    int num_samples = len / sizeof(int16_t);

    for (int i = 0; i < num_samples; i += 2) {
        float left = samples[i] / 32768.0f;
        sample_buffer[buffer_index] = left;
        buffer_index = (buffer_index + 1) % FFT_SIZE;
    }
}

void draw_dft(SDL_Renderer* renderer) {
    for (int k = 0; k < FFT_SIZE / 2; k += 2) {
        float sum_real = 0.0f;
        float sum_imag = 0.0f;

        for (int n = 0; n < FFT_SIZE; n++) {
            float theta = (2.0f * M_PI * k * n) / FFT_SIZE;

            sum_real += sample_buffer[n] * cosf(theta);
            sum_imag += sample_buffer[n] * sinf(theta);
        }

        float magnitude = sqrtf(sum_real * sum_real + sum_imag * sum_imag);

float normalized = (float)k / (FFT_SIZE / 2);

magnitude = sqrtf(magnitude);
        
        
        int bar_width = 3;
        int scale = 15; 
        int h = (int)(magnitude * scale);
        
        SDL_Rect bar = {
            (k / 2) * (bar_width + 1),
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

        SDL_SetRenderDrawColor(renderer, 33, 33, 33, 255);
        SDL_RenderClear(renderer);

        draw_dft(renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}