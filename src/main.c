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
#include <time.h>

#include "emulator.h"
#include "assembler.h"

int main(int argc, char **argv) {
	clock_t start, end;
	start = clock();

	// Start the assembler
	FILE *input, *output;
	input = fopen("src/everything.dasm", "r");
	output = fopen("src/everything.hex", "w");
	if (input == NULL || output == NULL) {
		return -1;
	}

	assemble(input, output);

	fclose(input);
	fclose(output);

	// Start the emulator
	FILE *rom;
	rom = fopen("src/everything.hex", "r");
	if (rom == NULL) {
		return -1;
	}

	emulator_t emu;
	emulator_init(EIGHT_BIT_MAX_MEM, rom, &emu);
	emulator_start(&emu);
	emulator_free(&emu);

	end = clock();
	printf("[main] Benchmarks: %f\n", (double) (end - start) / CLOCKS_PER_SEC);
	return 0;
}
