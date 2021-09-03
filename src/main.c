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
 * main.c
 *
 *  Created on: Aug 29, 2021
 *      Author: suncloudsmoon
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "emulator.h"
#include "assembler.h"

static void createHdd(long hddSize, char *destFile);

int main(int argc, char **argv) {
	clock_t start, end;
	start = clock();

	// Create hdd for the first time...
	createHdd(EIGHT_BIT_MAX_MEM, "src/everything.hdd");

	// Start the assembler
	FILE *input, *hddOutput;
	input = fopen("src/everything.dasm", "r");
	hddOutput = fopen("src/everything.hdd", "r+");
	if (input == NULL || hddOutput == NULL) {
		return -1;
	}
	fseek(hddOutput, 0, SEEK_SET); // the program is accessed without any disk formatting, etc.
	assemble(input, hddOutput);

	fclose(input);
	fclose(hddOutput);

	// Start the emulator
	FILE *hdd;
	hdd = fopen("src/everything.hdd", "r");
	if (hdd == NULL) {
		return -1;
	}

	emulator_t emu = { 0 };
	emulator_init(EIGHT_BIT_MAX_MEM, hdd, &emu);
	emulator_start(&emu);
	emulator_free(&emu);

	fclose(hdd);

	end = clock();
	printf("[main] Benchmarks: %f\n", (double) (end - start) / CLOCKS_PER_SEC);
	return 0;
}

static void createHdd(long hddSize, char *destFile) {
	FILE *hdd;
	hdd = fopen(destFile, "w");
	emulator_create_hdd(hddSize, hdd);
	fclose(hdd);
}
