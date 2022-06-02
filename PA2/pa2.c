/**********************************************************************
 * Copyright (c) 2019-2022
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

 /*====================================================================*/
 /*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */

 /* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#define MAX_NR_TOKENS	32	/* Maximum length of tokens in a command */
#define MAX_TOKEN_LEN	64	/* Maximum length of single token */
#define MAX_COMMAND	256 /* Maximum length of command string */

typedef unsigned char bool;
#define true	1
#define false	0

const char* __color_start = "[1;32;40m";
const char* __color_end = "[0m";

/**
 * memory[] emulates the memory of the machine
 */
static unsigned char memory[1 << 20] = {	/* 1MB memory at 0x0000 0000 -- 0x0100 0000 */
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0xde, 0xad, 0xbe, 0xef, 0x00, 0x00, 0x00, 0x00,
	'h',  'e',  'l',  'l',  'o',  ' ',  'w',  'o',
	'r',  'l',  'd',  '!',  '!',  0x00, 0x00, 0x00,
	'a',  'w',  'e',  's',  'o',  'm',  'e',  ' ',
	'c',  'o',  'm',  'p',  'u',  't',  'e',  'r',
	' ',  'a',  'r',  'c',  'h',  'i',  't',  'e',
	'c',  't',  'u',  'r',  'e',  '!',  0x00, 0x00,
};

#define INITIAL_PC	0x1000	/* Initial value for PC register */
#define INITIAL_SP	0x8000	/* Initial location for stack pointer */

/**
 * Registers of the machine
 */
static unsigned int registers[32] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0x10, INITIAL_PC, 0x20, 3, 0xbadacafe, 0xcdcdcdcd, 0xffffffff, 7,
	0, 0, 0, 0, 0, INITIAL_SP, 0, 0,
};

/**
 * Names of the registers. Note that $zero is shorten to zr
 */
const char* register_names[] = {
	"zr", "at", "v0", "v1", "a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

/**
 * Program counter register
 */
static unsigned int pc = INITIAL_PC;

/**
 * strmatch()
 *
 * DESCRIPTION
 *   Compare strings @str and @expect and return 1 if they are the same.
 *   You may use this function to simplify string matching :)
 *
 * RETURN
 *   1 if @str and @expect are the same
 *   0 otherwise
 */
static inline bool strmatch(char* const str, const char* expect)
{
	return (strlen(str) == strlen(expect)) && (strncmp(str, expect, strlen(expect)) == 0);
}

/*          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/

/**********************************************************************
 * process_instruction
 *
 * DESCRIPTION
 *   Execute the machine code given through @instr. The following table lists
 *   up the instructions to support. Note that a pseudo instruction 'halt'
 *   (0xffffffff) is added for the testing purpose. Also '*' instrunctions are
 *   the ones that are newly added to PA2.
 *
 * | Name   | Format    | Opcode / opcode + funct |
 * | ------ | --------- | ----------------------- |
 * | `add`  | r-format  | 0 + 0x20                |
 * | `addi` | i-format  | 0x08                    |
 * | `sub`  | r-format  | 0 + 0x22                |
 * | `and`  | r-format  | 0 + 0x24                |
 * | `andi` | i-format  | 0x0c                    |
 * | `or`   | r-format  | 0 + 0x25                |
 * | `ori`  | i-format  | 0x0d                    |
 * | `nor`  | r-format  | 0 + 0x27                |
 * | `sll`  | r-format  | 0 + 0x00                |
 * | `srl`  | r-format  | 0 + 0x02                |
 * | `sra`  | r-format  | 0 + 0x03                |
 * | `lw`   | i-format  | 0x23                    |
 * | `sw`   | i-format  | 0x2b                    |
 * | `slt`  | r-format* | 0 + 0x2a                |
 * | `slti` | i-format* | 0x0a                    |
 * | `beq`  | i-format* | 0x04                    |
 * | `bne`  | i-format* | 0x05                    |
 * | `jr`   | r-format* | 0 + 0x08                |
 * | `j`    | j-format* | 0x02                    |
 * | `jal`  | j-format* | 0x03                    |
 * | `halt` | special*  | @instr == 0xffffffff    |
 *
 * RETURN VALUE
 *   1 if successfully processed the instruction.
 *   0 if @instr is 'halt' or unknown instructions
 */
static int process_instruction(unsigned int instr)
{
	// MIPS R-format Instructions : opcode(6 bits) + rs(5 bits) + rt(5 bits) + rd(5 bits) + shamt(5 bits) + funct(6 bits)
	// MIPS I-format Instructions : opcode(6 bits) + rs(5 bits) + rt(5 bits) + constant or address(16 bits)
	// MIPS J-format Instructions : opcode(6 bits) + address(26 bits)

	// 0x012a4020 이런 instruction이 있으면 opcode(6 bits)를 본다.
	// opcode가 0이면 r포맷이고 funct(6 bits)를 보고 명령어 이름을 결정한다.
	// opcode가 0이 아니면 i포맷이거나 j포맷이고 opcode로 명령어 이름을 결정한다.

	// 해당 포맷에 맞게 instruction을 쪼개서 보고 연산이 필요하면 수행한다.
	// 레지스터 번호가 registers 배열의 인덱스 번호와 같으므로 배열에 값을 가져오거나 넣어준다.

	// i포맷의 lw, sw -> 메모리에서 값 가져오고 넣어줘야 함
	// r포맷의 jr, i포맷의 beq, bne, j포맷의 j, jal -> pc 이동해줘야 함

	// unsigned, signed -> unsigned로 더하나 signed로 더하나 값은 똑같이 나옴. 컴퓨터에 저장되는 비트는 같기 때문

	// 문제 : sra, lw, sw, ra (해결)
	// sra -> (signed)로 형변환 / lw, sw -> 1 word씩 가져오고 저장하는 거였음 / ra -> 코드 순서 바꿈

	// 문제: bne할 때 pc에서 마이너스 값 계산이 안됨 -> immedi 값을 비트 수에 맞게 signed으로 형변환했더니 해결됨

	// 문제: 그냥 basic에서는 signed 고려 안하는 것 같고, run basic하면 signed로 고려되는 듯. 왜???

	unsigned int opcode, rs, rt, rd, shamt, immedi, funct;

	if (instr == 0xffffffff) return 0; // halt

	// 0000 00/01 0010 1010 0100 0000 0010 0000
	opcode = instr >> 26;
	if (opcode == 0) { // R-format
		funct = instr & 0b111111;
		shamt = (instr >> 6) & 0b11111;
		rd = (instr >> 11) & 0b11111;
		rt = (instr >> 16) & 0b11111;
		rs = (instr >> 21) & 0b11111;

		switch (funct)
		{
		case 0x20: // add
			registers[rd] = registers[rs] + registers[rt];
			break;
		case 0x22: // sub
			registers[rd] = registers[rs] - registers[rt];
			break;
		case 0x24: // and
			registers[rd] = registers[rs] & registers[rt];
			break;
		case 0x25: // or
			registers[rd] = registers[rs] | registers[rt];
			break;
		case 0x27: // nor
			registers[rd] = ~(registers[rs] | registers[rt]);
			break;
		case 0x00: // sll
			registers[rd] = registers[rt] << shamt;
			break;
		case 0x02: // srl
			registers[rd] = registers[rt] >> shamt;
			break;
		case 0x03: // sra
			registers[rd] = (signed)registers[rt] >> shamt; // signed로 shift
			break;
		case 0x2a: // slt
			if (pc == INITIAL_PC) registers[rd] = registers[rs] < registers[rt]; // basic
			else registers[rd] = (char)registers[rs] < (char)registers[rt]; // run basic
			break;
		case 0x08: // jr
			pc = registers[rs]; // rs 레지스터가 가지고 있는 주소위치로 jump
		}
	}
	else if ((opcode == 0x02) || (opcode == 0x03)) { // J-format
		immedi = instr & 0b11111111111111111111111111;

		switch (opcode)
		{
		case 0x02: // j
			pc = (pc >> 27 << 27) | (immedi << 2); // pc[31...28](4 bits) + immedi(26 bits) + 00(2 bits)
			break;
		case 0x03: // jal
			registers[31] = pc; // ra가 jal 다음의 instruction을 가리키도록 한다.
			pc = (pc >> 27 << 27) | (immedi << 2); // pc[31...28](4 bits) + immedi(26 bits) + 00(2 bits)
		}
	}
	else { // I-format
		immedi = (short)(instr & 0xFFFF);

		rt = (instr >> 16) & 0b11111;
		rs = (instr >> 21) & 0b11111;

		switch (opcode)
		{
		case 0x08: // addi
			registers[rt] = registers[rs] + immedi;
			break;
		case 0x0c: // andi
			registers[rt] = registers[rs] & (unsigned short)immedi;
			break;
		case 0x0d: // ori
			registers[rt] = registers[rs] | (unsigned short)immedi;
			break;
		case 0x23: // lw -> 1 word (32 bits)씩 가져온다.
			registers[rt] = (*(memory + registers[rs] + immedi) << 24)
				| (*(memory + registers[rs] + immedi + 1) << 16)
				| (*(memory + registers[rs] + immedi + 2) << 8)
				| *(memory + registers[rs] + immedi + 3);
			break;
		case 0x2b: // sw -> 1 word (32 bits)씩 저장한다.
			*(memory + registers[rs] + immedi) = registers[rt] >> 24;
			*(memory + registers[rs] + immedi + 1) = (registers[rt] >> 16) & 0xFF;
			*(memory + registers[rs] + immedi + 2) = (registers[rt] >> 8) & 0xFF;
			*(memory + registers[rs] + immedi + 3) = registers[rt] & 0xFF;
			break;
		case 0x0a: // slti
			registers[rt] = registers[rs] < immedi;
			break;
		case 0x04: // beq
			if (registers[rt] == registers[rs]) pc = pc + 4 * immedi; // 여기서 immedi는 offset임
			break;
		case 0x05: // bne
			if (registers[rt] != registers[rs]) pc = pc + 4 * immedi; // 여기서 immedi는 offset임
		}
	}
	return 1;
}


/**********************************************************************
 * load_program(filename)
 *
 * DESCRIPTION
 *   Load the instructions in the file @filename onto the memory starting at
 *   @INITIAL_PC. Each line in the program file looks like;
 *
 *	 [MIPS instruction started with 0x prefix]  // optional comments
 *
 *   For example,
 *
 *   0x8c090008
 *   0xac090020	// sw t1, zero + 32
 *   0x8c080000
 *
 *   implies three MIPS instructions to load. Each machine instruction may
 *   be followed by comments like the second instruction. However you can simply
 *   call strtoimax(linebuffer, NULL, 0) to read the machine code while
 *   ignoring the comment parts.
 *
 *	 The program DOES NOT include the 'halt' instruction. Thus, make sure the
 *	 'halt' instruction is appended to the loaded instructions to terminate
 *	 your program properly.
 *
 *	 Refer to the @main() for reading data from files. (fopen, fgets, fclose).
 *
 * RETURN
 *	 0 on successfully load the program
 *	 any other value otherwise
 */

static int load_program(char* const filename)
{
	// 메모리에 instruction을 넣어줘야 함. 배열 한 칸에 8 비트씩 -> memory[] = 0x00
	// fgets로 파일 안에 데이터를 한 줄씩 읽어서 메모리에 로딩한다.

	FILE* input = NULL;
	input = fopen(filename, "r");
	char linebuffer[MAX_COMMAND] = { '\0' };
	unsigned int instr;
	int memIndex = 0;

	if (input != NULL) {
		while (fgets(linebuffer, sizeof(linebuffer), input)) {
			instr = strtoimax(linebuffer, NULL, 0);

			*(memory + pc + 4 * memIndex) = instr >> 24;
			*(memory + pc + 4 * memIndex + 1) = (instr >> 16) & 0xFF;
			*(memory + pc + 4 * memIndex + 2) = (instr >> 8) & 0xFF;
			*(memory + pc + 4 * memIndex + 3) = instr & 0xFF;
			memIndex++;
		}
		// append 'halt' instruction
		*(memory + pc + 4 * memIndex) = 0xFF;
		*(memory + pc + 4 * memIndex + 1) = 0xFF;
		*(memory + pc + 4 * memIndex + 2) = 0xFF;
		*(memory + pc + 4 * memIndex + 3) = 0xFF;

		fclose(input);
		return 0;
	}
	else printf("error!\n");

	return -EINVAL;
}


/**********************************************************************
 * run_program
 *
 * DESCRIPTION
 *   Start running the program that is loaded by @load_program function above.
 *   If you implement @load_program() properly, the first instruction is placed
 *   at @INITIAL_PC. Using @pc, which is the program counter of this processor,
 *   you can emulate the MIPS processor by
 *
 *   1. Read instruction from @pc
 *   2. Increment @pc by 4
 *   3. Call @process_instruction(instruction)
 *   4. Repeat until @process_instruction() returns 0
 *
 * RETURN
 *   0
 */
static int run_program(void)
{
	// 메모리에 로드된 instruction을 process_instruction(instr)로 실행시켜줘야 함
	pc = INITIAL_PC;
	unsigned int instr;

	while (true) {
		instr = (memory[pc] << 24) | (memory[pc + 1] << 16) | (memory[pc + 2] << 8) | memory[pc + 3];
		pc = pc + 4;
		if (!process_instruction(instr)) return 0;
	}

	return 0;
}


/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
static void __show_registers(char* const register_name)
{
	int from = 0, to = 0;
	bool include_pc = false;

	if (strmatch(register_name, "all")) {
		from = 0;
		to = 32;
		include_pc = true;
	}
	else if (strmatch(register_name, "pc")) {
		include_pc = true;
	}
	else {
		for (int i = 0; i < sizeof(register_names) / sizeof(*register_names); i++) {
			if (strmatch(register_name, register_names[i])) {
				from = i;
				to = i + 1;
			}
		}
	}

	for (int i = from; i < to; i++) {
		fprintf(stderr, "[%02d:%2s] 0x%08x    %u\n", i, register_names[i], registers[i], registers[i]);
	}
	if (include_pc) {
		fprintf(stderr, "[  pc ] 0x%08x\n", pc);
	}
}

static void __dump_memory(unsigned int addr, size_t length)
{
	for (size_t i = 0; i < length; i += 4) {
		fprintf(stderr, "0x%08lx:  %02x %02x %02x %02x    %c %c %c %c\n",
			addr + i,
			memory[addr + i], memory[addr + i + 1],
			memory[addr + i + 2], memory[addr + i + 3],
			isprint(memory[addr + i]) ? memory[addr + i] : '.',
			isprint(memory[addr + i + 1]) ? memory[addr + i + 1] : '.',
			isprint(memory[addr + i + 2]) ? memory[addr + i + 2] : '.',
			isprint(memory[addr + i + 3]) ? memory[addr + i + 3] : '.');
	}
}

static void __process_command(int argc, char* argv[])
{
	if (argc == 0) return;

	if (strmatch(argv[0], "load")) {
		if (argc == 2) {
			load_program(argv[1]);
		}
		else {
			printf("Usage: load [program filename]\n");
		}
	}
	else if (strmatch(argv[0], "run")) {
		if (argc == 1) {
			run_program();
		}
		else {
			printf("Usage: run\n");
		}
	}
	else if (strmatch(argv[0], "show")) {
		if (argc == 1) {
			__show_registers("all");
		}
		else if (argc == 2) {
			__show_registers(argv[1]);
		}
		else {
			printf("Usage: show { [register name] }\n");
		}
	}
	else if (strmatch(argv[0], "dump")) {
		if (argc == 3) {
			__dump_memory(strtoimax(argv[1], NULL, 0), strtoimax(argv[2], NULL, 0));
		}
		else {
			printf("Usage: dump [start address] [length]\n");
		}
	}
	else {
#ifdef INPUT_ASSEMBLY
		/**
		 * You may hook up @translate() from pa1 here to allow assembly code input!
		 */
		unsigned int instr = translate(argc, argv);
		process_instruction(instr);
#else
		process_instruction(strtoimax(argv[0], NULL, 0));
#endif
	}
}

static int __parse_command(char* command, int* nr_tokens, char* tokens[])
{
	char* curr = command;
	int token_started = false;
	*nr_tokens = 0;

	while (*curr != '\0') {
		if (isspace(*curr)) {
			*curr = '\0';
			token_started = false;
		}
		else {
			if (!token_started) {
				tokens[*nr_tokens] = curr;
				*nr_tokens += 1;
				token_started = true;
			}
		}
		curr++;
	}

	/* Exclude comments from tokens */
	for (int i = 0; i < *nr_tokens; i++) {
		if (strmatch(tokens[i], "//") || strmatch(tokens[i], "#")) {
			*nr_tokens = i;
			tokens[i] = NULL;
		}
	}

	return 0;
}

int main(int argc, char* const argv[])
{
	char command[MAX_COMMAND] = { '\0' };
	FILE* input = stdin;

	if (argc > 1) {
		input = fopen(argv[1], "r");
		if (!input) {
			fprintf(stderr, "No input file %s\n", argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (input == stdin) {
		printf("%s", __color_start);
		printf("*****************************************************\n");
		printf(" Welcome to SCE212 MIPS Termlink v0.2\n");
		printf("\n");
		printf(" SCE212 Model 2022-S is the most reliable client\n");
		printf(" terminal ever developed to run MIPS programs in\n");
		printf(" Vault 212.\n");
		printf("\n");
		printf("- VALID :   May 20 (Fri)\n");
		printf("- MANUAL:   https://git.ajou.ac.kr/sslab/ca-pa2.git\n");
		printf("- SUBMIT:   https://sslab.ajou.ac.kr/pasubmit\n");
		printf("\n");
		printf("- QNA AT AJOUBB ENCOURAGED!!\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf(">> ");
		printf("%s", __color_end);
	}

	while (fgets(command, sizeof(command), input)) {
		char* tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;

		for (size_t i = 0; i < strlen(command); i++) {
			command[i] = tolower(command[i]);
		}

		if (__parse_command(command, &nr_tokens, tokens) < 0)
			continue;

		__process_command(nr_tokens, tokens);

		if (input == stdin) printf("%s>> %s", __color_start, __color_end);
	}

	if (input != stdin) fclose(input);

	return EXIT_SUCCESS;
}
