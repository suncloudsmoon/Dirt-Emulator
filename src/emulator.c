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

static void movl(long *reg, long value, long *errReg);

static void addl(long *reg, long value, long *errReg);
static void subl(long *reg, long value, long *errReg);
static void imul(long *reg, long value, long *errReg);

static void andl(long *reg, long value, long *errReg);
static void orl(long *reg, long value, long *errReg);
static void xorl(long *reg, long value, long *errReg);
static void shrw(long *reg, long value, long *errReg);
static void shlw(long *reg, long value, long *errReg);

static void cmpl(long *reg, long value, long *x_special_reg, long *errReg);
static void intl(long value, long *errReg, bool *isRunning);

static long* get_reg_ptr(long reg, emulator_t *emu);
static long get_value_on_type(long type, long val, emulator_t *emu);

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

	// Let the operating system create the heap!
	bool isRunning = true;
	while (isRunning) {
		unsigned int opcode, reg, type, val;
		fscanf(emu->rom, "%x %x %x %x", &opcode, &reg, &type, &val); // TODO: replace with sscanf() later

		long *regPtr = get_reg_ptr(reg, emu);
		long value = get_value_on_type(type, val, emu);
		long *errReg = &emu->err_reg;

		switch (opcode) {
		case MOVL_INSTR:
			movl(regPtr, value, errReg);
			break;
		case ADDL_INSTR:
			addl(regPtr, value, errReg);
			break;
		case SUBL_INSTR:
			subl(regPtr, value, errReg);
			break;
		case IMUL_INSTR:
			imul(regPtr, value, errReg);
			break;
		case ANDL_INSTR:
			andl(regPtr, value, errReg);
			break;
		case ORL_INSTR:
			orl(regPtr, value, errReg);
			break;
		case XORL_INSTR:
			xorl(regPtr, value, errReg);
			break;
		case SHRW_INSTR:
			shrw(regPtr, value, errReg);
			break;
		case SHLW_INSTR:
			shlw(regPtr, value, errReg);
			break;
		case CMPL_INSTR:
			cmpl(regPtr, value, &emu->x_special_reg, errReg);
			break;
		case INTL_INSTR:
			intl(value, errReg, &isRunning);
			break;
		default:
			emu->err_reg = -999; // A very bad error code!
			break;
		}
		// Debug
		printf("Instruction Line: %d %d %d %d\n", opcode, reg, type, val);
		printf("--------------\n");
		printf("General Purpose Registers: %ld %ld %ld %ld\n", emu->a_reg, emu->b_reg,
				emu->c_reg, emu->d_reg);
		printf("Other Registers: %ld %ld %ld\n", emu->err_reg, emu->stack_reg, emu->base_reg);
		printf("Special Registers: %ld\n", emu->x_special_reg);
		printf("--------------\n");
	}
	return 0;
}

static void movl(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = -MOVL_INSTR;
		return;
	}
	*reg = value;
}

static void addl(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = -ADDL_INSTR;
		return;
	}
	*reg += value;
}

static void subl(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = -SUBL_INSTR;
		return;
	}
	*reg -= value;
}

static void imul(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = -IMUL_INSTR;
		return;
	}
	*reg *= value;
}

static void andl(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = -ANDL_INSTR;
		return;
	}
	*reg = *reg & value;
}

static void orl(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = -ORL_INSTR;
		return;
	}
	*reg = *reg | value;
}

static void xorl(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = -XORL_INSTR;
		return;
	}
	*reg = *reg ^ value;
}

static void shrw(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = -SHRW_INSTR;
		return;
	}
	*reg = *reg >> value;
}

static void shlw(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = -SHLW_INSTR;
		return;
	}
	*reg = *reg << value;
}

static void cmpl(long *reg, long value, long *x_special_reg, long *errReg) {
	if (*reg == -1) {
		*errReg = -CMPL_INSTR;
		return;
	}
	movl(x_special_reg, *reg, errReg);
	subl(x_special_reg, value, errReg);
}

// Interrupt
static void intl(long value, long *errReg, bool *isRunning) {
	switch (value) {
	case INT_SYS_EXIT_CODE:
		*isRunning = false;
		break;
	default:
		*errReg = -6;
		break;
	}
}

static long* get_reg_ptr(long reg, emulator_t *emu) {
	switch (reg) {
	case A_REG_HEX:
		return &emu->a_reg;
	case B_REG_HEX:
		return &emu->b_reg;
	case C_REG_HEX:
		return &emu->c_reg;
	case D_REG_HEX:
		return &emu->d_reg;
	case ERR_REG_HEX:
		return &emu->err_reg;
	case STACK_PTR_REG_HEX:
		return &emu->stack_reg;
	case BASE_PTR_REG_HEX:
		return &emu->stack_reg;
	default:
		emu->err_reg = -100;
		return &emu->err_reg;
	}
}

/*
 * If type is POINTER_TYPE then
 * get the register that is in val and do stack[val] to get the value of it
 */
static long get_value_on_type(long type, long val, emulator_t *emu) {
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
		emu->err_reg = -1;
		return emu->err_reg;
	}
}
