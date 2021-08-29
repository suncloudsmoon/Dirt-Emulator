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

typedef struct {
	// CPU
	unsigned int a_reg, b_reg, c_reg, d_reg, stack_reg, base_reg; // general purpose registers
	unsigned int x_special_reg; // x is for cmpl result storage

	// RAM
	void *stack[1000 * 1000]; // allocate the stack in the "real" stack!
	char **program; // Program loaded to memory

	// ROM
	FILE *rom; // like the text hard drive with the hex stuff
} emulator_t;

// NOTE: INT = Interrupt
typedef enum {
	ADDL_INSTR = 0x01,
	SUBL_INSTR = 0x02,
	IMUL_INSTR = 0x03,
	CMPL_INSTR = 0x04,
	INT_INSTR = 0x05
} InstructionSet;

typedef enum {
	A_REG_HEX = 0x01,
	B_REG_HEX = 0x02,
	C_REG_HEX = 0x03,
	D_REG_HEX = 0x04,
	BASE_PTR_REG_HEX = 0x05,
	STACK_PTR_REG_HEX = 0x06
} GeneralPurposeRegisters;

typedef enum {
	INTEGER_TYPE = 0x01,
	A_REG_TYPE = 0x03,
	B_REG_TYPE = 0x04,
	C_REG_TYPE = 0x05,
	D_REG_TYPE = 0x06,
	A_REG_POINTER_TYPE = 0x07,
	B_REG_POINTER_TYPE = 0x08,
	C_REG_POINTER_TYPE = 0x09,
	D_REG_POINTER_TYPE = 0x10,
	BASE_POINTER_TYPE = 0x11,
	STACK_POINTER_TYPE = 0x12
} Types;

emulator_t* emulator_init(size_t ramSize, FILE *rom);
void emulator_free(emulator_t *emu);

/*
 * Returns a error code of -1 or less if it encounters a error or returns 0 if everything went fine
 */
int emulator_start(emulator_t *emulator);

#endif /* EMULATOR_H_ */
