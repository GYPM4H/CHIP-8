#pragma once
#include <cstdint>

#define RAM_SIZE 4*1024

class CPU
{
private:
	uint8_t ram[RAM_SIZE];
	uint8_t V[16];
	uint8_t stack[48];

	uint8_t delay_timer;
	uint8_t sound_timer;

	uint16_t I = 0x0000;
	uint16_t pc = 0x0000;
	uint16_t sp = 0x0000;
	uint16_t opcode = 0x0000;
public:
	CPU();
	~CPU();

	uint8_t graphics[64 * 32];
	uint8_t keypad[16];
	bool draw;

	void clock();
	bool load_rom(const char* path);
};

