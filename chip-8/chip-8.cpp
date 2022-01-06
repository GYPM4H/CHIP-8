#define SDL_MAIN_HANDLED
#include <iostream>
#include "CPU.h"
#include <assert.h>
#include <SDL.h>
#include <stdlib.h>
const char* rom_path = "C:\\Users\\justpassion\\source\\repos\\chip-8\\test_opcode.ch8";

int main()
{
    uint8_t sdl_keypad[16] =
    {
        SDLK_KP_7,
        SDLK_KP_8,
        SDLK_KP_9,
        SDLK_c,
        SDLK_KP_4,
        SDLK_KP_5,
        SDLK_KP_6,
        SDLK_d,
        SDLK_KP_1,
        SDLK_KP_2,
        SDLK_KP_3,
        SDLK_e,
        SDLK_a,
        SDLK_KP_0,
        SDLK_b,
        SDLK_f,
    };

    CPU chip8 = CPU();
    chip8.load_rom(rom_path);
    uint8_t pixel_buffer[64 * 32 * 4];
    
    SDL_Window* window = nullptr;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "failed to init video subsystem" << SDL_GetError() <<'\n';
    }
    else {
        window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
        if (window == NULL) {
            std::cout << "Failed to create a window" << SDL_GetError() << '\n';
        }
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    SDL_Event event;
    bool quit = false;

    while (true) {
        if (quit) {
            break;
        }
        chip8.clock();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
                break;
            }

            if (event.type == SDL_KEYDOWN) {
                if(event.key.keysym.sym == SDLK_ESCAPE){
                    quit = true;
                    break;
                }

                for (int i = 0; i < 16; i++) {
                    if (event.key.keysym.sym == sdl_keypad[i]) {
                        chip8.keypad[i] = 1;
                    }
                }

            }

            if (event.type == SDL_KEYUP) {
                for(int i = 0; i < 16; i++){
                    if (event.key.keysym.sym == sdl_keypad[i]) {
                        chip8.keypad[i] = 0;
                    }
                }
                
            }
        }
        //system("CLS");
        if (chip8.draw) {
            chip8.draw = false;
            for (int i = 0; i < 2048; i++) {
                pixel_buffer[i * 4 + 0] = chip8.graphics[i];
                pixel_buffer[i * 4 + 1] = chip8.graphics[i];
                pixel_buffer[i * 4 + 2] = chip8.graphics[i];
                pixel_buffer[i * 4 + 3] = chip8.graphics[i];
            }

            //for (int i = 0; i < 32; i++) {
            //    for (int j = 0; j < 64; j++) {
            //        if (chip8.graphics[i * 64 + j] == 0) {
            //            std::cout << " ";
            //        }
            //        else {
            //            std::cout << "#";
            //        }
            //    }
            //    std::cout << "\n";
            //}
        }
        
        SDL_UpdateTexture(texture, NULL, pixel_buffer, 64 * 4);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
}


