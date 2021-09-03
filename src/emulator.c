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

static void movl(long *reg, long value);
static void stmovl(long *reg, long value, long *stack, long stackSize,
		long *errReg);

static void addl(long *reg, long value);
static void subl(long *reg, long value);
static void imul(long *reg, long value);
static void idivl(long *reg, long value);

static void andl(long *reg, long value);
static void orl(long *reg, long value);
static void xorl(long *reg, long value);
static void shrw(long *reg, long value);
static void shlw(long *reg, long value);

static void cmpl(long *reg, long value, long *x_special_reg);
static int je(long lineNum, long x_special_reg, long *instructionCounter);
static int jl(long lineNum, long x_special_reg, long *instructionCounter);
static int jg(long lineNum, long x_special_reg, long *instructionCounter);
static int jle(long lineNum, long x_special_reg, long *instructionCounter);
static int jge(long lineNum, long x_special_reg, long *instructionCounter);
static void jmp(long lineNum, long *instructionCounter);

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

int emulator_init(long stackSize, FILE *hdd, emulator_t *emu) {
	// RAM
	emu->stack = malloc(stackSize * sizeof(long));
	emu->specialMem = malloc(stackSize / 2 * sizeof(long));
	if (emu->stack == NULL || emu->specialMem == NULL) {
		return -1;
	}
	emu->specialMemCounter = -1;
	emu->stackSize = stackSize;
	emu->hdd = hdd;

	return 0;
}

void emulator_free(emulator_t *emu) {
	free(emu->stack);
	free(emu->specialMem);
}

int emulator_create_hdd(long hddSize, FILE *hdd) {
	// 8 zeros and a space (long int is 8 bit data type)
	for (long i = 0; i < hddSize; i++) {
		if (fprintf(hdd, "%08x ", 0x0) < 0) {
			return -1;
		}
	}
	return 0;
}

// Like for loading operating systems, etc.
int emulator_flash_pgrm_to_hdd(long location, FILE *pgrm, FILE *hdd) {
	// hdd with r+
	fseek(hdd, location * HDD_BIT_OFFSET, SEEK_SET);
	while (!feof(pgrm)) {
		unsigned long opcode, reg, type, val;
		if (fscanf(pgrm, "%lx %lx %lx %lx", &opcode, &reg, &type, &val) == EOF) {
			return -1;
		}
		if (fprintf(hdd, "%08lx %08lx %08lx %08lx ", opcode, reg, type, val)
				< 0) {
			return -1;
		}
	}
	return 0;
}

int emulator_start(emulator_t *emu) {
	movl(&emu->nop_reg, 0);
	emu->instructionCounter = 0;
	programToMem(emu);

	bool isRunning = true;
	while (isRunning) {
		long opcode, reg, type, val;
		opcode = emu->stack[emu->instructionCounter];
		reg = emu->stack[emu->instructionCounter + 1];
		type = emu->stack[emu->instructionCounter + 2];
		val = emu->stack[emu->instructionCounter + 3];

		long *regPtr = get_reg_ptr(reg, emu);
		long value = get_value_on_type(type, val, emu);
		long *errReg = &emu->err_reg;

		switch (opcode) {
		case NOP_INSTR:
			break;
		case MOVL_INSTR:
			movl(regPtr, value);
			break;
		case STMOVL_INSTR:
			stmovl(regPtr, value, emu->stack, emu->stackSize, errReg);
			break;
		case ADDL_INSTR:
			addl(regPtr, value);
			break;
		case SUBL_INSTR:
			subl(regPtr, value);
			break;
		case IMUL_INSTR:
			imul(regPtr, value);
			break;
		case IDIVL_INSTR:
			idivl(regPtr, value);
			break;
		case ANDL_INSTR:
			andl(regPtr, value);
			break;
		case ORL_INSTR:
			orl(regPtr, value);
			break;
		case XORL_INSTR:
			xorl(regPtr, value);
			break;
		case SHRW_INSTR:
			shrw(regPtr, value);
			break;
		case SHLW_INSTR:
			shlw(regPtr, value);
			break;
		case CMPL_INSTR:
			cmpl(regPtr, value, &emu->x_special_reg);
			break;
		case JE_INSTR:
			if (je(value - 1, emu->x_special_reg, &emu->instructionCounter))
				continue;
			break;
		case JL_INSTR:
			if (jl(value - 1, emu->x_special_reg, &emu->instructionCounter))
				continue;
			break;
		case JG_INSTR:
			if (jg(value - 1, emu->x_special_reg, &emu->instructionCounter))
				continue;
			break;
		case JLE_INSTR:
			if (jle(value - 1, emu->x_special_reg, &emu->instructionCounter))
				continue;
			break;
		case JGE_INSTR:
			if (jge(value - 1, emu->x_special_reg, &emu->instructionCounter))
				continue;
			break;
		case JMP_INSTR:
			jmp(value - 1, &emu->instructionCounter);
			continue;
		case INTL_INSTR:
			intl(value, errReg, &isRunning, emu);
			break;
		case PUSHL_INSTR:
			pushl(value, emu->specialMem, &emu->specialMemCounter,
					emu->stackSize, errReg);
			break;
		case POPL_INSTR:
			popl(regPtr, errReg, &emu->specialMem[0], &emu->specialMemCounter);
			break;
		default:
			fprintf(stderr, "[Debug] CPU FAULT: 0x%x on emulator_start!\n",
			SEGMENTATION_FAULT);
			emu->err_reg = SEGMENTATION_FAULT;
			break;
		}
		emu->instructionCounter += 4;

		// A nice view of what is going on behind the scenes
		printf("Instruction Line: %ld %ld %ld %ld\n", opcode, reg, type, val);
		printf("--------------\n");
		printf("A, B, C, D: %ld %ld %ld %ld\n", emu->a_reg, emu->b_reg,
				emu->c_reg, emu->d_reg);
		printf("Error, Stack, Base, X Special Reg: %ld %ld %ld %ld\n",
				emu->err_reg, emu->stack_reg, emu->base_reg,
				emu->x_special_reg);
		printf("Instruction Counter: %ld\n", emu->instructionCounter);
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
		printf("emu->specialMemCounter: %ld\n", emu->specialMemCounter);
		printf("--------------\n");
	}
	return 0;
}

static int programToMem(emulator_t *emu) {
	// Put the size of the code first (similar to ELF binary)
	unsigned long operation, numLines, operand1, operand2;
	if (fscanf(emu->hdd, "%lx %lx %lx %lx", &operation, &numLines, &operand1,
			&operand2) == EOF) {
		return -1;
	}

	long lineCounter = 1;
	while (lineCounter <= numLines && !feof(emu->hdd)) {
		unsigned long opcode, reg, type, val;
		if (fscanf(emu->hdd, "%lx %lx %lx %lx", &opcode, &reg, &type,
				&val) == EOF) {
			return -1;
		}

		movl(&emu->a_reg, opcode);
		movl(&emu->b_reg, reg);
		movl(&emu->c_reg, type);
		movl(&emu->d_reg, val);

		addl(&emu->stack_reg, 4);
		stmovl(&emu->a_reg, emu->stack_reg - 4, emu->stack, emu->stackSize,
				&emu->err_reg);
		stmovl(&emu->b_reg, emu->stack_reg - 3, emu->stack, emu->stackSize,
				&emu->err_reg);
		stmovl(&emu->c_reg, emu->stack_reg - 2, emu->stack, emu->stackSize,
				&emu->err_reg);
		stmovl(&emu->d_reg, emu->stack_reg - 1, emu->stack, emu->stackSize,
				&emu->err_reg);
		lineCounter++;
	}
	return emu->err_reg;
}

static void movl(long *reg, long value) {
	*reg = value;
}

static void stmovl(long *reg, long value, long *stack, long stackSize,
		long *errReg) {
	if (value >= stackSize) {
		fprintf(stderr, "[Debug] CPU FAULT: 0x%x on get_reg_ptr()!\n",
				STMOVL_INSTR);
		*errReg = STMOVL_INSTR;
		return;
	}
	stack[value] = *reg;
}

static void addl(long *reg, long value) {
	*reg += value;
}

static void subl(long *reg, long value) {
	*reg -= value;
}

static void imul(long *reg, long value) {
	*reg *= value;
}

static void idivl(long *reg, long value) {
	*reg /= value;
}

static void andl(long *reg, long value) {
	*reg &= value;
}

static void orl(long *reg, long value) {
	*reg |= value;
}

static void xorl(long *reg, long value) {
	*reg ^= value;
}

static void shrw(long *reg, long value) {
	*reg >>= value;
}

static void shlw(long *reg, long value) {
	*reg <<= value;
}

static void cmpl(long *reg, long value, long *x_special_reg) {
	movl(x_special_reg, *reg);
	subl(x_special_reg, value);
}

static int je(long lineNum, long x_special_reg, long *instructionCounter) {
	if (x_special_reg == 0) {
		*instructionCounter = lineNum * 4;
		return 1;
	} else {
		return 0;
	}
}

static int jl(long lineNum, long x_special_reg, long *instructionCounter) {
	if (x_special_reg < 0) {
		*instructionCounter = lineNum * 4;
		return 1;
	} else {
		return 0;
	}
}

static int jg(long lineNum, long x_special_reg, long *instructionCounter) {
	if (x_special_reg > 0) {
		*instructionCounter = lineNum * 4;
		return 1;
	} else {
		return 0;
	}
}

static int jle(long lineNum, long x_special_reg, long *instructionCounter) {
	if (x_special_reg <= 0) {
		*instructionCounter = lineNum * 4;
		return 1;
	} else {
		return 0;
	}
}

static int jge(long lineNum, long x_special_reg, long *instructionCounter) {
	if (x_special_reg >= 0) {
		*instructionCounter = lineNum * 4;
		return 1;
	} else {
		return 0;
	}
}

static void jmp(long lineNum, long *instructionCounter) {
	*instructionCounter = lineNum * 4;
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
		fprintf(stderr, "[Debug] CPU FAULT: 0x%x on get_reg_ptr()!\n",
				INTL_INSTR);
		*errReg = INTL_INSTR;
		break;
	}
}

static void pushl(long value, long *specialMemArr, long *specialMemCounter,
		long ramSize, long *errReg) {
	if (*specialMemCounter > ramSize / 2) {
		fprintf(stderr, "[Debug] CPU FAULT: 0x%x on get_reg_ptr()!\n",
				PUSHL_INSTR);
		*errReg = PUSHL_INSTR;
		return;
	}
	*specialMemCounter = *specialMemCounter + 1;
	*(specialMemArr + *specialMemCounter) = value;
}

static void popl(long *regPtr, long *errReg, long *specialMemArr,
		long *specialMemCounter) {
	if (*specialMemCounter < 0) {
		fprintf(stderr, "[Debug] CPU FAULT: 0x%x on get_reg_ptr()!\n",
				POPL_INSTR);
		*errReg = POPL_INSTR;
		return;
	}
	*regPtr = *(specialMemArr + *specialMemCounter);
	*specialMemCounter = *specialMemCounter - 1;
}

static long* get_reg_ptr(long reg, emulator_t *emu) {
	switch (reg) {
	case NOP_REG_HEX:
		return &emu->nop_reg;
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
	case STACK_REG_HEX:
		return &emu->stack_reg;
	case BASE_REG_HEX:
		return &emu->base_reg;
	default:
		fprintf(stderr, "[Debug] CPU FAULT: 0x%x on get_reg_ptr()!\n",
		SEGMENTATION_FAULT);
		emu->err_reg = SEGMENTATION_FAULT;
		return &emu->err_reg; // TODO: replace this with an alternative method because this yields funny results
	}
}

/*
 * If type is POINTER_TYPE then
 * get the register that is in val and do stack[val] to get the value of it
 */
static long get_value_on_type(long type, long val, emulator_t *emu) {
	switch (type) {
	case NOP_TYPE:
		return 0;
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
	default:
		fprintf(stderr, "[Debug] CPU FAULT: 0x%x on get_value_on_type()!\n",
		SEGMENTATION_FAULT);
		emu->err_reg = SEGMENTATION_FAULT;
		return emu->err_reg;
	}
}
