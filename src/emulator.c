/*
 * Copyright (c) 2021, suncloudsmoon and the Dirt OS Emulator contributors.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * emulator.c
 *
 *  Created on: Aug 29, 2021
 *      Author: suncloudsmoon
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "emulator.h"

static long int get_value_on_type(long int type, long int val, emulator_t *emu);

emulator_t* emulator_init(size_t ram_size, FILE *rom) {
	emulator_t *emu = malloc(sizeof(emulator_t));

	// ROM
	emu->rom = rom;
	return emu;
}

void emulator_free(emulator_t *emu) {
	free(emu);
}

// Format: [opcode] [register] [type (indicates if it is a register, etc.)] [value (pure numbers here)]
// So if you find a registry, say A_REG_TYPE in type, then it will multiply the A_REG
// BASE_REG = ba in assembly
int emulator_start(emulator_t *emu) {
	// CPU registers set to zero
	emu->a_reg = 0;
	emu->b_reg = 0;
	emu->c_reg = 0;
	emu->d_reg = 0;
	emu->stack_reg = 0;
	emu->base_reg = 0;
	emu->x_special_reg = 0;

	while (true) {
		unsigned int opcode, reg, type, val;
		fscanf(emu->rom, "%x %x %x %x", &opcode, &reg, &type, &val); // TODO: replace with sscanf() later

		// Let the operating system create the heap!
		switch (opcode) {
		case ADDL_INSTR:
			// If type is POINTER_TYPE then
			// get the register that is in val and do stack[val] to get the value of it
			break;
		case SUBL_INSTR:
			break;
		case IMUL_INSTR:
			break;
		case CMPL_INSTR:
			break;
		case INT_INSTR:
			break;
		default:
			return -1;
		}

		printf("Debug: %d %d %d %d", opcode, reg, type, val);
		break;
	}
	return 0;
}

static long int get_value_on_type(long int type, long int val, emulator_t *emu) {
	switch (type) {
	case INTEGER_TYPE:
		return val;
	case A_REG_TYPE:
		return emu->a_reg * val;
	case B_REG_TYPE:
		return emu->b_reg * val;
	case C_REG_TYPE:
		return emu->c_reg * val;
	case D_REG_TYPE:
		return emu->d_reg * val;
	case A_REG_POINTER_TYPE:
		return *(emu->a_reg) * val;
	case B_REG_POINTER_TYPE:
		return *(emu->b_reg) * val;
	case C_REG_POINTER_TYPE:
		return *(emu->c_reg) * val;
	case D_REG_POINTER_TYPE:
		return *(emu->d_reg) * val;
	case STACK_POINTER_TYPE:
		return *(emu->stack_reg) * val;
	case BASE_POINTER_TYPE:
		return *(emu->base_reg) * val;
	default:
		return -1;
	}
}
