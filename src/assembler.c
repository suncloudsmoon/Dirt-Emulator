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
 * assembler.c
 *
 *  Created on: Aug 31, 2021
 *      Author: suncloudsmoon
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "emulator.h"
#include "assembler.h"

static unsigned long opcodeToHex(char *opcode);
static unsigned long regToHex(char *reg);
static unsigned long typeToHex(char *type);
static unsigned long valToHex(char *val);

// Opcodes
char opcodes[13][10] = { "movl", "addl", "subl", "imul", "andl", "orl", "xorl", "shrw", "shlw", "cmpl", "intl", "pushl", "popl" };
long opcodesHex[13] = { MOVL_INSTR, ADDL_INSTR, SUBL_INSTR, IMUL_INSTR, ANDL_INSTR, ORL_INSTR, XORL_INSTR, SHRW_INSTR, SHLW_INSTR, CMPL_INSTR, INTL_INSTR, PUSHL_INSTR, POPL_INSTR};

// Registries
char regs[7][10] = { "a", "b", "c", "d", "err", "stack", "base" };
long regsHex[7] = { A_REG_HEX, B_REG_HEX, C_REG_HEX, D_REG_HEX, ERR_REG_HEX,
		STACK_PTR_REG_HEX, BASE_PTR_REG_HEX };

// Types
char types[15][10] = { "int", "areg", "breg", "creg", "dreg", "ereg", "skreg",
		"bsreg", "aptr", "bptr", "cptr", "dptr", "eptr", "skptr", "bsptr" };
char typesHex[15] = { INTEGER_TYPE, A_REG_TYPE, B_REG_TYPE, C_REG_TYPE,
		D_REG_TYPE, ERR_REG_TYPE, STACK_REG_TYPE, BASE_REG_TYPE,
		A_REG_POINTER_TYPE, B_REG_POINTER_TYPE, C_REG_POINTER_TYPE,
		D_REG_POINTER_TYPE, ERR_REG_POINTER_TYPE, STACK_REG_POINTER_TYPE,
		BASE_REG_POINTER_TYPE };

// NOTE: // Max 500 characters in one assembly line!
int assemble(FILE *input, FILE *output) {
	int returnCode = 0;
	char stream[500];
	char opcode[50], reg[50], type[50], val[50];
	while (!feof(input)) {
		char *successCheck = fgets(&stream, 500, input);
		if (stream[0] == '/' || successCheck == NULL || strlen(successCheck) < 5) {
			continue;
		}
		sscanf(&stream, "%s %s %s %s", &opcode, &reg, &type, &val);
		if (fprintf(output, "0x%lx 0x%lx 0x%lx 0x%lx\n", opcodeToHex(&opcode),
				regToHex(&reg), typeToHex(&type), valToHex(&val)) < 0) {
			returnCode = -1;
		}
	}
	return returnCode;
}

static unsigned long opcodeToHex(char *opcode) {
	for (int i = 0; i < 13; i++) {
		if (strcmp(opcode, opcodes[i]) == 0) {
			return opcodesHex[i];
		}
	}
	return -1;
}

static unsigned long regToHex(char *reg) {
	for (int i = 0; i < 7; i++) {
		if (strcmp(reg, regs[i]) == 0) {
			return regsHex[i];
		}
	}
	return -1;
}

static unsigned long typeToHex(char *type) {
	for (int i = 0; i < 15; i++) {
		if (strcmp(type, types[i]) == 0) {
			return typesHex[i];
		}
	}
	return -1;
}

static unsigned long valToHex(char *val) {
	return strtol(val, NULL, 10);
}
