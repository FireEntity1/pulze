#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <complex.h>

#define TRANSFORM_SIZE 4096
#define NUM_BARS 416

float sample_buffer[TRANSFORM_SIZE];
int buffer_index = 0;

float smoothed_heights[NUM_BARS] = {0}; 
const int SAMPLE_RATE = 44100;

const int SMOOTHEN = 2;

float elapsed = 0.0f;

float bass = 0.0f;
float bass_smoothed = 150.0f;

void increment_time() {
    elapsed = SDL_GetTicks64() / 1000.0f;
}

void bit_reverse(float complex* X, int N) {
    int target = 0;
    for (int position = 0; position < N; position++) {
        if (target > position) {
            float complex temp = X[target];
            X[target] = X[position];
            X[position] = temp;
        }
        int mask = N >> 1;
        while (mask > 0 && (target & mask)) {
            target &= ~mask;
            mask >>= 1;
        }
        target |= mask;
    }
}

void fft(float complex* X, int N) {
    bit_reverse(X, N);
    for (int len=2; len <= N; len <<= 1) {
        float angle = -2.0f * M_PI / len;
        float complex wlen = cosf(angle) + sinf(angle) * I;
        for (int i=0; i < N; i += len) {
            float complex w = 1.0f + 0.0f * I;
            for (int j=0; j < len/2; j++) {
                float complex u = X[i+j];
                float complex v = X[i+j+len/2] * w;
                X[i+j] = u + v;
                X[i+j+len/2] = u - v;
                w *= wlen;
            }
        }
    }
}

float dft(float* windowed, int k) {
    if (k < 0 || k >= TRANSFORM_SIZE / 2) return 0.0f;
    float sum_real = 0.0f;
    float sum_imag = 0.0f;
    for (int n = 0; n < TRANSFORM_SIZE; n++) {
        float theta = (2.0f * M_PI * (k+5) * n) / TRANSFORM_SIZE;
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

float clamp(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

void draw_bars(SDL_Renderer* renderer) {
    
    float complex fft_buffer[TRANSFORM_SIZE];
    for (int n = 0; n < TRANSFORM_SIZE; n++) {
        int target_index = (buffer_index + n) % TRANSFORM_SIZE;
        float window = 0.5f * (1.0f - cosf(2.0f * M_PI * n / (TRANSFORM_SIZE - 1)));
        fft_buffer[n] = sample_buffer[target_index] * window + 0.0f * I;
    }

    fft(fft_buffer, TRANSFORM_SIZE);

    int bar_width = 2;
    int gap = 0;

    for (int b = 0; b < NUM_BARS; b++) {
        float t = (float)b / (NUM_BARS - 1);
        float log_min = log10f(1.5f);
        float log_max = log10f((float)(TRANSFORM_SIZE / 2 - 1));
        
        float k_exact = powf(10.0f, log_min + t * (log_max - log_min));
        int k0 = (int)floorf(k_exact);
        int k1 = k0 + 1;
        float fraction = k_exact - k0;

        float mag0 = cabsf(fft_buffer[k0 + 5]);
        float mag1 = cabsf(fft_buffer[k1 + 5]);
        float magnitude = mag0 + fraction * (mag1 - mag0);

        float eq_boost = log10f(k_exact * 10.0f); 
        float target_h = sqrtf(magnitude) * 12.5f * eq_boost;
        
        if (target_h > smoothed_heights[b]) {
            smoothed_heights[b] += (target_h - smoothed_heights[b]) * 0.5f;
        } else {
            smoothed_heights[b] -= (smoothed_heights[b] - target_h) * 0.4f;
        }

        int h = 0;
        for (int i = 1; i<SMOOTHEN;i++) {
            h += smoothed_heights[b];
        }
        h /= SMOOTHEN;
        if (h > 600) h = 600;
        
        int x = b * (bar_width + gap);

        SDL_Rect bar = {
            x,
            300 - h,
            bar_width,
            h
        };
        SDL_Rect bar2 = {
            x,
            300,
            bar_width,
            h
        };
        float scroll_speed = 120.0f;
        // float i = elapsed * 1.8f + b * 0.045f;

        // float r_wave = sinf(i) * 0.5f + 0.5f;
        // float b_wave = sinf(i + 2.1f) * 0.5f + 0.5f;

        float i = elapsed * 1.2f + b * 0.02f;

        Uint8 r = (Uint8)clamp((bass_smoothed-100)*3.0f, 100, 200);
        Uint8 g = 50;
        Uint8 bl = (Uint8)clamp((-bass_smoothed+200)*2.0f, 0, 255);

        SDL_SetRenderDrawColor(renderer, r, g, bl, 255);
        SDL_RenderFillRect(renderer, &bar);
        SDL_RenderFillRect(renderer, &bar2);
    }
}

float lerp(float a, float b, float t, bool down_slow) {
    if (down_slow && b < a) {
        t *= 0.2f;
    }
    return a + t * (b - a);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return 1;
    if (Mix_OpenAudio(SAMPLE_RATE, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return 1;
    
    
    Mix_RegisterEffect(MIX_CHANNEL_POST, capture_audio, NULL, NULL);
    
    Mix_Music* music = Mix_LoadMUS("assets/ghoul.mp3");
    if (!music) return 1;
    Mix_PlayMusic(music, -1);
    Mix_SetMusicPosition(10.0);
    SDL_Window* window = SDL_CreateWindow("pulze", 100, 100, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    bool running = true;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }
        float loudness = 0.0f;
    
        for (int i = 0; i < 50; i++) {
            loudness += smoothed_heights[(i+30)*20];
        }
        loudness /= 50.0f;
        // printf("Initial loudness: %f\n", loudness);
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        float new_bass = 0.0f;
        for (int i = 0; i < 35; i++) {
            new_bass += smoothed_heights[i+25];
        }
        new_bass /= 35.0f;
        bass = bass * 0.9f + new_bass * 0.1f;
        bass_smoothed = lerp(bass_smoothed, bass, 0.015f, true);
        printf("Bass: %f\n", bass_smoothed);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, bass/20.0f);
        SDL_RenderFillRect(renderer, &(SDL_Rect){0, 0, 800, 800});
        draw_bars(renderer);
        
        SDL_RenderPresent(renderer);
        elapsed = SDL_GetTicks64() / 1000.0f;
    }

    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}
