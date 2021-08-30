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
static void addl(long int reg, long int value, emulator_t *emu);
static void subl(long int reg, long int value, emulator_t *emu);
static void imul(long int reg, long int value, emulator_t *emu);

void emulator_init(FILE *rom, emulator_t *emu) {
	// ROM
	emu->rom = rom;
}

void emulator_free(emulator_t *emu) {
	free(emu);
}

// TODO: make a error code documentation / table
// Format: [opcode] [register] [type (indicates if it is a register, etc.)] [value (pure numbers here)]
// So if you find a registry, say A_REG_TYPE in type, then it will multiply the A_REG
// BASE_REG = ba in assembly
int emulator_start(emulator_t *emu) {
	// CPU registers set to zero
	emu->a_reg = 0;
	emu->b_reg = 0;
	emu->c_reg = 0;
	emu->d_reg = 0;

	emu->err_reg = 0;
	emu->stack_reg = 0;
	emu->base_reg = 0;
	emu->x_special_reg = 0;

	while (true) {
		unsigned int opcode, reg, type, val;
		fscanf(emu->rom, "%x %x %x %x", &opcode, &reg, &type, &val); // TODO: replace with sscanf() later

		// Let the operating system create the heap!
		switch (opcode) {
		case ADDL_INSTR:
			addl(reg, get_value_on_type(type, val, emu), emu);
			break;
		case SUBL_INSTR:
			subl(reg, get_value_on_type(type, val, emu), emu);
			break;
		case IMUL_INSTR:
			imul(reg, get_value_on_type(type, val, emu), emu);
			break;
		case CMPL_INSTR:
			break;
		case INT_INSTR:
			break;
		default:
			emu->err_reg = -1;
			break;
		}
		// Debug
		printf("Instruction Line: %d %d %d %d\n", opcode, reg, type, val);
		printf("Registers: %d %d %d %d\n", emu->a_reg, emu->b_reg, emu->c_reg, emu->d_reg);

		break; // TODO: remove this
	}
	return 0;
}

static void addl(long int reg, long int value, emulator_t *emu) {
	switch (reg) {
	case A_REG_HEX:
		emu->a_reg += value;
		break;
	case B_REG_HEX:
		emu->b_reg += value;
		break;
	case C_REG_HEX:
		emu->c_reg += value;
		break;
	case D_REG_HEX:
		emu->d_reg += value;
		break;
	case ERR_REG_HEX:
		emu->err_reg += value;
		break;
	case STACK_PTR_REG_HEX:
		emu->stack_reg += value;
		break;
	case BASE_PTR_REG_HEX:
		emu->base_reg += value;
		break;
	default:
		emu->err_reg = -2;
		break;
	}
}

static void subl(long int reg, long int value, emulator_t *emu) {
	switch (reg) {
	case A_REG_HEX:
		emu->a_reg -= value;
		break;
	case B_REG_HEX:
		emu->b_reg -= value;
		break;
	case C_REG_HEX:
		emu->c_reg -= value;
		break;
	case D_REG_HEX:
		emu->d_reg -= value;
		break;
	case ERR_REG_HEX:
		emu->err_reg -= value;
		break;
	case STACK_PTR_REG_HEX:
		emu->stack_reg -= value;
		break;
	case BASE_PTR_REG_HEX:
		emu->base_reg -= value;
		break;
	default:
		emu->err_reg = -3;
		break;
	}
}

static void imul(long int reg, long int value, emulator_t *emu) {
	switch (reg) {
	case A_REG_HEX:
		emu->a_reg *= value;
		break;
	case B_REG_HEX:
		emu->b_reg *= value;
		break;
	case C_REG_HEX:
		emu->c_reg *= value;
		break;
	case D_REG_HEX:
		emu->d_reg *= value;
		break;
	case ERR_REG_HEX:
		emu->err_reg *= value;
		break;
	case STACK_PTR_REG_HEX:
		emu->stack_reg *= value;
		break;
	case BASE_PTR_REG_HEX:
		emu->base_reg *= value;
		break;
	default:
		emu->err_reg = -4;
		break;
	}
}

/*
 * If type is POINTER_TYPE then
 * get the register that is in val and do stack[val] to get the value of it
 */
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
	case ERR_REG_TYPE:
		return emu->err_reg * val;
	case STACK_REG_TYPE:
		return emu->stack_reg * val;
	case BASE_REG_TYPE:
		return emu->base_reg * val;
	case A_REG_POINTER_TYPE:
		return *(emu->stack + emu->a_reg) * val;
	case B_REG_POINTER_TYPE:
		return emu->stack[emu->b_reg] * val;
	case C_REG_POINTER_TYPE:
		return emu->stack[emu->c_reg] * val;
	case D_REG_POINTER_TYPE:
		return emu->stack[emu->d_reg] * val;
	case ERR_REG_POINTER_TYPE:
		return emu->stack[emu->err_reg] * val;
	case STACK_REG_POINTER_TYPE:
		return emu->stack[emu->stack_reg] * val;
	case BASE_REG_POINTER_TYPE:
		return emu->stack[emu->base_reg] * val;
	default:
		return -1;
	}
}
