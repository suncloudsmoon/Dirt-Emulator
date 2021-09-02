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

static int programToMem(emulator_t *emu);

static void movl(long *reg, long value, long *errReg);
static void stmovl(long *reg, long value, long *stack, long *errReg);

static void addl(long *reg, long value, long *errReg);
static void subl(long *reg, long value, long *errReg);
static void imul(long *reg, long value, long *errReg);

static void andl(long *reg, long value, long *errReg);
static void orl(long *reg, long value, long *errReg);
static void xorl(long *reg, long value, long *errReg);
static void shrw(long *reg, long value, long *errReg);
static void shlw(long *reg, long value, long *errReg);

static void cmpl(long *reg, long value, long *x_special_reg, long *errReg);

static void intl(long value, long *errReg, bool *isRunning, emulator_t *emu);
static void pushl(long value, long *specialMemArr, long *specialMemCounter,
		long ramSize, long *errReg);
static void popl(long *regPtr, long *errReg, long *specialMemArr,
		long *specialMemCounter);

static long* get_reg_ptr(long reg, emulator_t *emu);
static long get_value_on_type(long type, long val, emulator_t *emu);

// TODO: make a error code documentation / table
// Format: [opcode] [register] [type (indicates if it is a register, etc.)] [value (pure numbers here)]
// So if you find a registry, say A_REG_TYPE in type, then it will multiply the A_REG * value
// BASE_REG = ba in assembly

int emulator_init(long ramSize, FILE *rom, emulator_t *emu) {
	// RAM
	emu->stack = malloc(ramSize * sizeof(long));
	emu->specialMem = malloc(ramSize / 2 * sizeof(long));
	if (emu->stack == NULL || emu->specialMem == NULL) {
		return -1;
	}
	emu->specialMemCounter = -1;
	emu->ramSize = ramSize;
	emu->rom = rom;

	return 0;
}

void emulator_free(emulator_t *emu) {
	free(emu->stack);
	free(emu->specialMem);
}

int emulator_start(emulator_t *emu) {
	emu->instructionCounter = 0;
	programToMem(emu);

	bool isRunning = true;
	while (isRunning) {
		long opcode, reg, type, val;
		opcode = emu->stack[emu->instructionCounter];
		reg =  emu->stack[emu->instructionCounter + 1];
		type = emu->stack[emu->instructionCounter + 2];
		val = emu->stack[emu->instructionCounter + 3];

		long *regPtr = get_reg_ptr(reg, emu);
		long value = get_value_on_type(type, val, emu);
		long *errReg = &emu->err_reg;

		switch (opcode) {
		case MOVL_INSTR:
			movl(regPtr, value, errReg);
			break;
		case STMOVL_INSTR:
			stmovl(regPtr, value, emu->stack, errReg);
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
			intl(value, errReg, &isRunning, emu);
			break;
		case PUSHL_INSTR:
			pushl(value, emu->specialMem, &emu->specialMemCounter, emu->ramSize,
					errReg);
			break;
		case POPL_INSTR:
			popl(regPtr, errReg, &emu->specialMem[0], &emu->specialMemCounter);
			break;
		default:
			emu->err_reg = INSTR_NOT_FOUND_ERR; // A very bad error code!
			break;
		}
		emu->instructionCounter++;

		// A nice view of what is going on behind the scenes
		printf("Instruction Line: %d %d %d %d\n", opcode, reg, type, val);
		printf("--------------\n");
		printf("A, B, C, D: %ld %ld %ld %ld\n", emu->a_reg, emu->b_reg,
				emu->c_reg, emu->d_reg);
		printf("Error, Stack, Base: %ld %ld %ld\n", emu->err_reg,
				emu->stack_reg, emu->base_reg);
		printf("X Special Reg: %ld\n", emu->x_special_reg);

		printf("Memory: ");
		for (int i = 0; i <= emu->stack_reg; i++) {
			printf("%ld ", emu->stack[i]);
		}
		printf("\n");

		printf("Special Memory: ");
		for (int i = 0; i <= emu->specialMemCounter; i++) {
			printf("%ld ", emu->specialMem[i]);
		}
		printf("\n");
		printf("--------------\n");
		break;
	}
	return 0;
}

static int programToMem(emulator_t *emu) {
	movl(&emu->stack_reg, 3, &emu->err_reg);
	while (!feof(emu->rom)) {
		char opcodeStr[50], regStr[50], typeStr[50], valStr[50];
		fscanf(emu->rom, "%s %s %s %s", &opcodeStr, &regStr, &typeStr, &valStr);

		long opcode, reg, type, val;
		opcode = strtol(opcodeStr, NULL, 16);
		reg = strtol(regStr, NULL, 16);
		type = strtol(typeStr, NULL, 16);
		val = strtol(valStr, NULL, 16);

		movl(&emu->a_reg, opcode, &emu->err_reg);
		movl(&emu->b_reg, reg, &emu->err_reg);
		movl(&emu->c_reg, type, &emu->err_reg);
		movl(&emu->d_reg, val, &emu->err_reg);

		stmovl(&emu->a_reg, emu->stack_reg - 3, emu->stack, &emu->err_reg);
		stmovl(&emu->b_reg, emu->stack_reg - 2, emu->stack, &emu->err_reg);
		stmovl(&emu->c_reg, emu->stack_reg - 1, emu->stack, &emu->err_reg);
		stmovl(&emu->d_reg, emu->stack_reg, emu->stack, &emu->err_reg);

		addl(&emu->stack_reg, 3, &emu->err_reg);
	}
	return emu->err_reg;
}

static void movl(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = MOVL_INSTR;
		return;
	}
	*reg = value;
}

static void stmovl(long *reg, long value, long *stack, long *errReg) {
	if (*reg == -1) {
		*errReg = MOVL_INSTR;
		return;
	}
	*(stack + value) = *reg;
}

static void addl(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = ADDL_INSTR;
		return;
	}
	*reg += value;
}

static void subl(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = SUBL_INSTR;
		return;
	}
	*reg -= value;
}

static void imul(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = IMUL_INSTR;
		return;
	}
	*reg *= value;
}

static void andl(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = ANDL_INSTR;
		return;
	}
	*reg = *reg & value;
}

static void orl(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = ORL_INSTR;
		return;
	}
	*reg = *reg | value;
}

static void xorl(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = XORL_INSTR;
		return;
	}
	*reg = *reg ^ value;
}

static void shrw(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = SHRW_INSTR;
		return;
	}
	*reg = *reg >> value;
}

static void shlw(long *reg, long value, long *errReg) {
	if (*reg == -1) {
		*errReg = SHLW_INSTR;
		return;
	}
	*reg = *reg << value;
}

static void cmpl(long *reg, long value, long *x_special_reg, long *errReg) {
	if (*reg == -1) {
		*errReg = CMPL_INSTR;
		return;
	}
	movl(x_special_reg, *reg, errReg);
	subl(x_special_reg, value, errReg);
}

static void je(long *x_special_reg) {

}

// Interrupt
static void intl(long value, long *errReg, bool *isRunning, emulator_t *emu) {
	switch (value) {
	case INT_STDOUT_CODE:
		// a_reg is the pointer to the location in stack, b_reg is the string length
		for (int i = 0; i < emu->b_reg; i++) {
			fprintf(stdout, "%c", (char) emu->stack[emu->a_reg + i]);
		}
		break;
	case INT_SYS_EXIT_CODE:
		*isRunning = false;
		break;
	default:
		*errReg = INTL_INSTR;
		break;
	}
}

static void pushl(long value, long *specialMemArr, long *specialMemCounter,
		long ramSize, long *errReg) {
	if (*specialMemCounter > ramSize / 2) {
		*errReg = PUSHL_INSTR;
		return;
	}
	*specialMemCounter = *specialMemCounter + 1;
	*(specialMemArr + *specialMemCounter) = value;
}

static void popl(long *regPtr, long *errReg, long *specialMemArr,
		long *specialMemCounter) {
	if (*specialMemCounter < 0) {
		*errReg = POPL_INSTR;
		return;
	}
	*regPtr = *(specialMemArr + *specialMemCounter);
	*specialMemCounter = *specialMemCounter - 1;
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
		return &emu->base_reg;
	default:
		emu->err_reg = REG_NOT_FOUND_ERR;
		return &emu->err_reg; // TODO: replace this with an alternative method because this yields funny results
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
		return emu->a_reg + val;
	case B_REG_TYPE:
		return emu->b_reg + val;
	case C_REG_TYPE:
		return emu->c_reg + val;
	case D_REG_TYPE:
		return emu->d_reg + val;
	case ERR_REG_TYPE:
		return emu->err_reg + val;
	case STACK_REG_TYPE:
		return emu->stack_reg + val;
	case BASE_REG_TYPE:
		return emu->base_reg + val;
	case A_REG_POINTER_TYPE:
		return emu->stack[emu->a_reg] + val;
	case B_REG_POINTER_TYPE:
		return emu->stack[emu->b_reg] + val;
	case C_REG_POINTER_TYPE:
		return emu->stack[emu->c_reg] + val;
	case D_REG_POINTER_TYPE:
		return emu->stack[emu->d_reg] + val;
	case ERR_REG_POINTER_TYPE:
		return emu->stack[emu->err_reg] + val;
	case STACK_REG_POINTER_TYPE:
		return emu->stack[emu->stack_reg] + val;
	case BASE_REG_POINTER_TYPE:
		return emu->stack[emu->base_reg] + val;
	default:
		emu->err_reg = TYPE_NOT_FOUND_ERR;
		return emu->err_reg;
	}
}
