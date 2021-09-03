/*
 * Copyright (c) 2021, suncloudsmoon and the Dirt Emulator contributors.
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

#define SEGMENTATION_FAULT 5555
#define HDD_BIT_OFFSET 10

typedef enum {
	EIGHT_BIT_MAX_MEM = 256, SIXTEEN_BIT_MAX_MEM = 65535
} MemSizeConstants;

typedef struct {
	// CPU
	long nop_reg, a_reg, b_reg, c_reg, d_reg, err_reg, stack_reg, base_reg; // general purpose registers
	long x_special_reg; // x is for cmpl result storage

	// RAM
	long *stack; // programs are stored here too!
	long *specialMem; // push pop stuff goes here
	long specialMemCounter;
	long stackSize;

	// Program
	long instructionCounter;

	// ROM
	FILE *hdd; // like the text hard drive with the hex stuff
} emulator_t;

typedef enum {
	NOP_REG_HEX = 0x0,
	A_REG_HEX = 0x01,
	B_REG_HEX = 0x02,
	C_REG_HEX = 0x03,
	D_REG_HEX = 0x04,
	ERR_REG_HEX = 0x05,
	STACK_REG_HEX = 0x06,
	BASE_REG_HEX = 0x07
} GeneralPurposeRegisters;

// NOTE: INT = Interrupt
typedef enum {
	NOP_INSTR = 0x0,
	MOVL_INSTR = 0x01,
	STMOVL_INSTR = 0x02,

	ADDL_INSTR = 0x03,
	SUBL_INSTR = 0x04,
	IMUL_INSTR = 0x05,
	IDIVL_INSTR = 0x06,

	ANDL_INSTR = 0x07,
	ORL_INSTR = 0x08,
	XORL_INSTR = 0x09,
	SHRW_INSTR = 0xA,
	SHLW_INSTR = 0xB,

	CMPL_INSTR = 0xC,
	JE_INSTR = 0xD,
	JL_INSTR = 0xE,
	JG_INSTR = 0xF,
	JLE_INSTR = 0x10,
	JGE_INSTR = 0x11,
	JMP_INSTR = 0x12,

	PUSHL_INSTR = 0x14,
	POPL_INSTR = 0x15,
	INTL_INSTR = 0x16
} InstructionSet;

typedef enum {
	INT_STDOUT_CODE = 0x01, INT_SYS_EXIT_CODE = 0x02
} InterruptCodes;

typedef enum {
	NOP_TYPE = 0x0,
	INTEGER_TYPE = 0x01,

	A_REG_TYPE = 0x02,
	B_REG_TYPE = 0x03,
	C_REG_TYPE = 0x04,
	D_REG_TYPE = 0x05,
	ERR_REG_TYPE = 0x06,
	STACK_REG_TYPE = 0x07,
	BASE_REG_TYPE = 0x08
} Types;

int emulator_init(long stackSize, FILE *hdd, emulator_t *emu);
void emulator_free(emulator_t *emu);

int emulator_create_hdd(long hddSize, FILE *hdd);
int emulator_flash_pgrm_to_hdd(long location, FILE *pgrm, FILE *hdd);
/*
 * Returns a error code of -1 or less if it encounters a error or returns 0 if everything went fine
 */
int emulator_start(emulator_t *emu);

#endif /* EMULATOR_H_ */
