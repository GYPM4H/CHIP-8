#include "CPU.h"
#include <iostream>
#include <assert.h>

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
		for (int i = 0; i < 2048; i++) {
			graphics[i] = 0;
		}
		pc += 2;
		break;

	case 0x1000:
		pc = opcode & 0x0FFF;
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
		uint8_t X = V[(opcode & 0x0F00) >> 8] & 63;
		uint8_t Y = V[(opcode & 0x00F0) >> 4] & 31;
		uint8_t H = opcode & 0x000F;
		uint8_t sprite_data;
		
		V[0xF] = 0;

		for (int i = 0; i < H; i++) {
			sprite_data = ram[I + i];
			for (int j; j < 8; j++) {

			}

		}
	}
		

}

