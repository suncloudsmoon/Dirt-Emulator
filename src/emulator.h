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
 * emulator.h
 *
 *  Created on: Aug 29, 2021
 *      Author: suncloudsmoon
 */

#ifndef EMULATOR_H_
#define EMULATOR_H_

#define STACKSIZE 65535

typedef struct {
	// CPU
	long a_reg, b_reg, c_reg, d_reg, err_reg, stack_reg, base_reg; // general purpose registers
	long x_special_reg; // x is for cmpl result storage

	// RAM
	long stack[STACKSIZE]; // programs are stored here too!
	long specialMem[STACKSIZE / 2]; // push pop stuff goes here
	long specialMemCounter;

	// ROM
	FILE *rom; // like the text hard drive with the hex stuff
} emulator_t;

typedef enum {
	REG_NOT_FOUND_ERR = 100,
	TYPE_NOT_FOUND_ERR = 200,
	INSTR_NOT_FOUND_ERR = 999
} TerribleErrorCodes;

typedef enum {
	A_REG_HEX = 0x01, B_REG_HEX = 0x02, C_REG_HEX = 0x03, D_REG_HEX = 0x04,
	ERR_REG_HEX = 0x05, STACK_PTR_REG_HEX = 0x06, BASE_PTR_REG_HEX = 0x07
} GeneralPurposeRegisters;

// NOTE: INT = Interrupt
typedef enum {
	MOVL_INSTR = 0x01,

	ADDL_INSTR = 0x02,
	SUBL_INSTR = 0x03,
	IMUL_INSTR = 0x04,

	ANDL_INSTR = 0x05,
	ORL_INSTR = 0x06,
	XORL_INSTR = 0x07,
	SHRW_INSTR = 0x08,
	SHLW_INSTR = 0x09,

	CMPL_INSTR = 0xA,
	INTL_INSTR = 0xB,
	PUSHL_INSTR = 0xC,
	POPL_INSTR = 0xD
} InstructionSet;

typedef enum {
	INT_STDOUT_CODE = 0x01,
	INT_SYS_EXIT_CODE = 0x02
} InterruptCodes;

typedef enum {
	INTEGER_TYPE = 0x01,

	A_REG_TYPE = 0x02,
	B_REG_TYPE = 0x03,
	C_REG_TYPE = 0x04,
	D_REG_TYPE = 0x05,
	ERR_REG_TYPE = 0x06,
	STACK_REG_TYPE = 0x07,
	BASE_REG_TYPE = 0x08,

	A_REG_POINTER_TYPE = 0x09,
	B_REG_POINTER_TYPE = 0xA,
	C_REG_POINTER_TYPE = 0xB,
	D_REG_POINTER_TYPE = 0xC,
	ERR_REG_POINTER_TYPE = 0xD,
	STACK_REG_POINTER_TYPE = 0xE,
	BASE_REG_POINTER_TYPE = 0xF
} Types;

void emulator_init(FILE *rom, emulator_t *emu);
void emulator_free(emulator_t *emu);

/*
 * Returns a error code of -1 or less if it encounters a error or returns 0 if everything went fine
 */
int emulator_start(emulator_t *emulator);

#endif /* EMULATOR_H_ */
