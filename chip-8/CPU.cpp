#include "CPU.h"
#include <iostream>
#include <assert.h>
#include <string>

#define CHIP8_CONFIG 1 // 1 == original ; 2 == superchip

CPU::CPU() {
	uint8_t font[80] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	pc = 0x200;

	for (int i = 0; i < 80; i++) {
		ram[i] = font[i];
	}

	//clear things
	for (int i = 0; i < 16; i++) {
		stack[i] = 0;
		keypad[i] = 0;
		V[i] = 0;
	}

	for (int i = 0; i < 2048; i++) {
		graphics[i] = 0;
	}

	for (int i = 0; i < 4096; i++) {
		ram[i] = 0;
	}
}

CPU::~CPU() {

}

bool CPU::load_rom(const char* path) {
	FILE* rom = fopen(path, "r");
	assert(std::ferror(rom) != 0, "Failed to load rom");
	uint8_t buffer[3584];

	std::fread(buffer, sizeof(uint8_t), 3584, rom);

	assert(std::feof(rom) == 0, "Failed to load rom");

	std::rewind(rom);
	std::fseek(rom, 0, SEEK_END);
	long rom_size = std::ftell(rom);
	std::rewind(rom);


	for (int i = 0; i < rom_size; i++) {
		ram[i + 512] = buffer[i];
	}

	std::fclose(rom);
}

void CPU::clock() {
	opcode = (ram[pc]) << 8 | (ram[pc + 1]); // pc is 8 bits. opcode is 16 bits. So we bitshift to the left and add another 8 bits;
	uint16_t opcode_number = opcode & 0xF000;
	 
	// implemet call and return from subroutine
	switch (opcode_number) {
	case 0x0000:
		switch(opcode & 0x000F){
		case 0x0000:
			for (int i = 0; i < 2048; i++) {
				graphics[i] = 0;
			}
			draw = true;
			pc += 2;
			break;

		case 0x000E:
			pc = stack[sp];
			pc += 2;
			break;
		}
		break;

	case 0x1000:
		pc = opcode & 0x0FFF;
		break;
	
	case 0x2000:
		stack[sp] = pc;
		pc = opcode & 0x0FFF;
		sp++;
		break;

	case 0x3000:
		V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF) ? pc += 4 : pc += 2;
		break;

	case 0x4000:
		V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF) ? pc += 4 : pc += 2;
		break;

	case 0x5000:
		V[(opcode & 0x0F00) >> 8] == V[opcode & 0x00F0] ? pc += 4 : pc += 2;
		break;

	case 0x6000:
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		pc += 2;
		break;

	case 0x7000:
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;
		break;

	case 0x8000:
		switch (opcode & 0x000F) {
		case 0x0000:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0001:
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0002:
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0003:
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0004:
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			((V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4]) > 0xFF) ? V[0xF] = 1 : V[0xF] = 0;
			pc += 2;
			break;

		case 0x0005:
			V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8] ? V[0xF] = 0 : V[0xF] = 1;
			pc += 2;
			break;

		case 0x0006:

#if CHIP8_CONFIG == 1
			V[(opcode & 0x0F00) >> 8] = V[opcode & 0x00F0];
#endif
			V[0xF] = (V[(opcode & 0x0F00) >> 8] & 0x1);
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;

		case 0x0007:
			V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) ? V[0xF] = 0 : V[0xF] = 1;
			pc += 2;
			break;

		case 0x000E:
			V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
			break;
		}
		break;

	case 0x9000:
		V[(opcode & 0x0F00) >> 8] != V[opcode & 0x00F0] ? pc += 4 : pc += 2;
		break;

	case 0xA000:
		I = opcode & 0x0FFF;
		pc += 2;
		break;

	case 0xB000:
#if CHIP8_CONFIG == 1
		pc = (opcode & 0x0FFF) + V[0x0];
		break;
#endif
#if CHIP8_CONFIG == 2
		pc = (opcode & 0x0FFF) + V[(opcode & 0x0F00) >> 8];
		break;
#endif

	case 0xC000:
		V[(opcode & 0x0F00) >> 8] = (0 + rand() % 0xFF) & (opcode & 0x00FF);
		pc += 2;
		break;

	case 0xD000: // 0xDXYN where (X,Y) - coordinates and N represents height in pixels, width = 8 pixels;
		uint8_t X = V[(opcode & 0x0F00) >> 8] % 64;
		uint8_t Y = V[(opcode & 0x00F0) >> 4] % 32;
		uint8_t H = opcode & 0x000F;
		uint8_t sprite_data;
		uint8_t dx = std::min(64 - X, 8);
		uint8_t dy = std::min(32 - Y, static_cast<int>(H));
		V[0xF] = 0;

		for (int i = 0; i < dy; i++) {
			sprite_data = ram[I + i];
			for (int j = 0; j < dx; j++) {

				if ((sprite_data >> j) & 1) {
					if (graphics[(X + j) + (Y + i) * 8]) {
						graphics[(X + j) + (Y + i) * 8] = 0;
						V[0xF] = 0x01;
					}
					else {
						graphics[(X + j) + (Y + i) * 8] = 1;
					}
				}
			}
		}
		break;

	case 0xE000:
		switch (opcode & 0x00FF) {
		case 0x009E:
			if (keypad[V[(opcode & 0x0F00) >> 8]] != 0) {
				pc += 4;
			}
			else pc += 2;

			break;
		case 0x00A1:
			if (keypad[V[(opcode & 0x0F00) >> 8]] == 0) {
				pc += 4;
			}
			else pc += 2;

			break;
		}
		break;

	case 0xF000:
		switch (opcode & 0x00FF) {
		case 0x0007:
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		case 0x000A:
			key_pressed = false;
			for (auto key : keypad) {
				if (keypad[key] != 0) {
					V[(opcode & 0x0F00) >> 8] = key;
					key_pressed = true;
					pc += 2;
					break;
				}
				else if(key_pressed = false){
					pc -= 2;
					break;
				}
			}

		case 0x0015:
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0018:
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x001E:
			(V[(opcode & 0x0F00) >> 8] + I > 0xFFFF) ? V[0x0F] = 1 : V[0x0F] = 0;
			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0029:
			I = V[(opcode & 0x0F00) >> 8] * 5; // font is 4 by 5. Look at the 9-th line of the file 
			pc += 2;
			break;

		case 0x0033: 
			// Vx stores 1 byte number(0 - 255). 
			//This instruction takes this number and splits it into three separate digits wich is stored in ram[I]
			//e.g. 153 splits into 1 which is stored in ram[I+0], 5 in ram[I+1] and 3 in ram[I+2]

			ram[I + 0] = V[(opcode & 0x0F00) >> 8] / 100; // get the first num
			ram[I + 1] = V[(opcode & 0x0F00) >> 8] % 100 / 10; // get the second num
			ram[I + 2] = V[(opcode & 0x0F00) >> 8] % 10; // get the third num
			
			pc += 2;

			break;
		
		case 0x0055:
			for (int i = 0; i <= (opcode & 0x0F00 >> 8); i++) {
				ram[I + i] = V[i];

			}

			pc += 2;

			break;

		case 0x0065:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
				V[i] = ram[I + i];
			}
			
			pc += 2;

			break;
		}
		break;
	}
		
		

}

