/**********************************************************************
 * Copyright (c) 2021-2022
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
#include <string.h>
#include <ctype.h>
#include <errno.h>

 /* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
#define MAX_NR_TOKENS	32	/* Maximum length of tokens in a command */
#define MAX_TOKEN_LEN	64	/* Maximum length of single token */
#define MAX_ASSEMBLY	256 /* Maximum length of assembly string */

typedef unsigned char bool;
#define true	1
#define false	0
/*          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/


/***********************************************************************
 * translate()
 *
 * DESCRIPTION
 *   Translate assembly represented in @tokens[] into a MIPS instruction.
 *   This translate should support following 13 assembly commands
 *
 *    - add
 *    - addi
 *    - sub
 *    - and
 *    - andi
 *    - or
 *    - ori
 *    - nor
 *    - lw
 *    - sw
 *    - sll
 *    - srl
 *    - sra
 *    - beq
 *    - bne
 *
 * RETURN VALUE
 *   Return a 32-bit MIPS instruction
 *
 */

// 레지스터 이름에 해당하는 비트를 변환하는 함수
static unsigned int getRegisterBit(char* token) {
	if (strcmp(token, "zero") == 0) return 0;
	if (strcmp(token, "at") == 0) return 1;
	if (strcmp(token, "v0") == 0) return 2;
	if (strcmp(token, "v1") == 0) return 3;
	if (strcmp(token, "a0") == 0) return 4;
	if (strcmp(token, "a1") == 0) return 5;
	if (strcmp(token, "a2") == 0) return 6;
	if (strcmp(token, "a3") == 0) return 7;
	if (strcmp(token, "t0") == 0) return 8;
	if (strcmp(token, "t1") == 0) return 9;
	if (strcmp(token, "t2") == 0) return 10;
	if (strcmp(token, "t3") == 0) return 11;
	if (strcmp(token, "t4") == 0) return 12;
	if (strcmp(token, "t5") == 0) return 13;
	if (strcmp(token, "t6") == 0) return 14;
	if (strcmp(token, "t7") == 0) return 15;
	if (strcmp(token, "s0") == 0) return 16;
	if (strcmp(token, "s1") == 0) return 17;
	if (strcmp(token, "s2") == 0) return 18;
	if (strcmp(token, "s3") == 0) return 19;
	if (strcmp(token, "s4") == 0) return 20;
	if (strcmp(token, "s5") == 0) return 21;
	if (strcmp(token, "s6") == 0) return 22;
	if (strcmp(token, "s7") == 0) return 23;
	if (strcmp(token, "t8") == 0) return 24;
	if (strcmp(token, "t9") == 0) return 25;
	if (strcmp(token, "k1") == 0) return 26;
	if (strcmp(token, "k2") == 0) return 27;
	if (strcmp(token, "gp") == 0) return 28;
	if (strcmp(token, "sp") == 0) return 29;
	if (strcmp(token, "fp") == 0) return 30;
	if (strcmp(token, "ra") == 0) return 31;
}

// 비트를 자릿수에 맞춰서 문자열로 변환하는 함수
static void getBitToString(int value, int n, char* str) {
	char bitString[20];
	int c = 0;
	int index = 0;

	for (int i = n - 1; i >= 0; i--) {
		c = (value & (1 << i)) >> i;
		bitString[index] = c + '0'; //오른쪽부터 i+1번째 자리의 수, 정수를 문자로 변환
		index++;
	}
	bitString[n] = '\0';
	strcpy(str, bitString);

	return;
}

static unsigned int translate(int nr_tokens, char* tokens[])
{
	/* TODO:
	 * This is an example MIPS instruction. You should change it accordingly.
	 */

	 // translate tokens into bits -> 레지스터 이름에 맞는 비트로 변환
	 // and merge them to produce one 32-bit machine instruction. -> 포맷별로 merge 자리 배치

	 // MIPS R-format Instructions : opcode(6 bits) + rs(5 bits) + rt(5 bits) + rd(5 bits) + shamt(5 bits) + funct(6 bits)
	 // MIPS I-format Instructions : opcode(6 bits) + rs(5 bits) + rt(5 bits) + constant or address(16 bits)

	 // 인덱스 1부터 3까지 tokens[]의 각 인덱스에 해당하는 토큰을 비트로 바꾼다.
	 // 해당 포맷에 맞게 자리 배치해서 토큰들의 비트를 합친다.
	 // 합친 2진수 32 bits를 16진수로 바꾼다.

	bool iFormat = false;
	char opcode[7] = { 0 };
	char rs[6] = { 0 };
	char rt[6] = { 0 };
	char rd[6] = { 0 };
	char shamt[6] = { 0 };
	char consOrAddr[17] = { 0 };
	char funct[7] = { 0 };
	char result[33] = { 0 };
	unsigned int resultValue = 0;

	// 첫 인덱스의 토큰을 통해 포맷을 결정한다.
	if (strcmp(tokens[0], "addi") == 0 || strcmp(tokens[0], "andi") == 0 || strcmp(tokens[0], "ori") == 0 ||
		strcmp(tokens[0], "lw") == 0 || strcmp(tokens[0], "sw") == 0 || strcmp(tokens[0], "beq") == 0 || strcmp(tokens[0], "bne") == 0) {
		iFormat = true;
	}

	if (!iFormat) { // R-format인 경우
		// opcode(6 bits) + rs(5 bits) + rt(5 bits) + rd(5 bits) + shamt(5 bits) + funct(6 bits) 할당
		getBitToString(0, 6, opcode);
		if (strcmp(tokens[0], "add") == 0 || strcmp(tokens[0], "sub") == 0 || 
			strcmp(tokens[0], "and") == 0 || strcmp(tokens[0], "or") == 0 || strcmp(tokens[0], "nor") == 0) {
			getBitToString(getRegisterBit(tokens[1]), 5, rd);
			getBitToString(getRegisterBit(tokens[2]), 5, rs);
			getBitToString(getRegisterBit(tokens[3]), 5, rt);
			getBitToString(0, 5, shamt);
		}
		if (strcmp(tokens[0], "add") == 0) {
			getBitToString(0x20, 6, funct);
		}
		if (strcmp(tokens[0], "sub") == 0) {
			getBitToString(0x22, 6, funct);
		}
		if (strcmp(tokens[0], "and") == 0) {
			getBitToString(0x24, 6, funct);
		}
		if (strcmp(tokens[0], "or") == 0) {
			getBitToString(0x25, 6, funct);
		}
		if (strcmp(tokens[0], "nor") == 0) {
			getBitToString(0x27, 6, funct);
		}
		if (strcmp(tokens[0], "sll") == 0 || strcmp(tokens[0], "srl") == 0 || strcmp(tokens[0], "sra") == 0) {
			getBitToString(0, 5, rs);
			getBitToString(getRegisterBit(tokens[1]), 5, rd);
			getBitToString(getRegisterBit(tokens[2]), 5, rt);
			if (tokens[3][1] == 'x' || tokens[3][2] == 'x') { //"0x~~" or "-0x~~"임 -> 16진수인 경우
				getBitToString(strtol(tokens[3], NULL, 16), 5, shamt);
			}
			else { //10진수인 경우
				getBitToString(strtol(tokens[3], NULL, 10), 5, shamt);
			}
		}
		if (strcmp(tokens[0], "sll") == 0) {
			getBitToString(0x00, 6, funct);
		}
		if (strcmp(tokens[0], "srl") == 0) {
			getBitToString(0x02, 6, funct);
		}
		if (strcmp(tokens[0], "sra") == 0) {
			getBitToString(0x03, 6, funct);
		}
		// 비트열 합치기 -> 문자열 연결
		// opcode(6 bits) + rs(5 bits) + rt(5 bits) + rd(5 bits) + shamt(5 bits) + funct(6 bits)
		sprintf(result, "%s%s%s%s%s%s", opcode, rs, rt, rd, shamt, funct); //문자열을 연결시킨 출력값을 result에 저장한다.
	}
	
	if (iFormat) { // I-format인 경우
		// opcode(6 bits) + rs(5 bits) + rt(5 bits) + constant or address(16 bits) 할당
		getBitToString(getRegisterBit(tokens[1]), 5, rt);
		getBitToString(getRegisterBit(tokens[2]), 5, rs);
		if (tokens[3][1] == 'x' || tokens[3][2] == 'x') { //"0x~~" or "-0x~~"임 -> 16진수인 경우
			getBitToString(strtol(tokens[3], NULL, 16), 16, consOrAddr);
		}
		else { //10진수인 경우
			getBitToString(strtol(tokens[3], NULL, 10), 16, consOrAddr);
		}

		if (strcmp(tokens[0], "addi") == 0) {
			getBitToString(0x08, 6, opcode);
		}
		if (strcmp(tokens[0], "andi") == 0) {
			getBitToString(0x0c, 6, opcode);
		}
		if (strcmp(tokens[0], "ori") == 0) {
			getBitToString(0x0d, 6, opcode);
		}
		if (strcmp(tokens[0], "lw") == 0) {
			getBitToString(0x23, 6, opcode);
		}
		if (strcmp(tokens[0], "sw") == 0) {
			getBitToString(0x2b, 6, opcode);
		}
		if (strcmp(tokens[0], "beq") == 0) {
			getBitToString(0x04, 6, opcode);
		}
		if (strcmp(tokens[0], "bne") == 0) {
			getBitToString(0x05, 6, opcode);
		}

		// 비트열 합치기 -> 문자열 연결
		// opcode(6 bits) + rs(5 bits) + rt(5 bits) + constant or address(16 bits)
		sprintf(result, "%s%s%s%s", opcode, rs, rt, consOrAddr);
	}

	/*if (!iFormat) {
		printf("r-format입니다.\n");
		printf("opcode(6 bits): %s\n", opcode);
		printf("rs(5 bits): %s\n", rs);
		printf("rt(5 bits): %s\n", rt);
		printf("rd(5 bits): %s\n", rd);
		printf("shamt(5 bits): %s\n", shamt);
		printf("funct(6 bits): %s\n", funct);
	}
	else {
		printf("i-format입니다.\n");
		printf("opcode(6 bits): %s\n", opcode);
		printf("rs(5 bits): %s\n", rs);
		printf("rt(5 bits): %s\n", rt);
		printf("constant or address(16 bits): %s\n", consOrAddr);
	}
	printf("result>>>>>>>>%s\n", result);*/

	// 32bits result 문자열을 숫자로 바꿔서 리턴한다.
	resultValue = strtol(result, NULL, 2);

	return resultValue;
}


/***********************************************************************
 * parse_command()
 *
 * DESCRIPTION
 *   Parse @assembly, and put each assembly token into @tokens[] and the number
 *   of tokes into @nr_tokens. You may use this implemention or your own
 *   from PA0.
 *
 *   A assembly token is defined as a string without any whitespace (i.e., space
 *   and tab in this programming assignment). For exmaple,
 *     command = "  add t1   t2 s0 "
 *
 *   then, nr_tokens = 4, and tokens is
 *     tokens[0] = "add"
 *     tokens[1] = "t0"
 *     tokens[2] = "t1"
 *     tokens[3] = "s0"
 *
 *   You can assume that the characters in the input string are all lowercase
 *   for testing.
 *
 *
 * RETURN VALUE
 *   Return 0 after filling in @nr_tokens and @tokens[] properly
 *
 */
static int parse_command(char* assembly, int* nr_tokens, char* tokens[])
{
	char* curr = assembly;
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

	return 0;
}


/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING BELOW THIS LINE ******      */

/***********************************************************************
 * The main function of this program.
 */
int main(int argc, char* const argv[])
{
	char assembly[MAX_ASSEMBLY] = { '\0' };
	FILE* input = stdin;

	if (argc > 1) {
		input = fopen(argv[1], "r");
		if (!input) {
			fprintf(stderr, "No input file %s\n", argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (input == stdin) {
		printf("*********************************************************\n");
		printf("*          >> SCE212 MIPS translator  v0.10 <<          *\n");
		printf("*                                                       *\n");
		printf("*                                       .---.           *\n");
		printf("*                           .--------.  |___|           *\n");
		printf("*                           |.------.|  |=. |           *\n");
		printf("*                           || >>_  ||  |-- |           *\n");
		printf("*                           |'------'|  |   |           *\n");
		printf("*                           ')______('~~|___|           *\n");
		printf("*                                                       *\n");
		printf("*                                 Spring 2022           *\n");
		printf("*********************************************************\n\n");
		printf(">> ");
	}

	while (fgets(assembly, sizeof(assembly), input)) {
		char* tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;
		unsigned int machine_code;

		for (size_t i = 0; i < strlen(assembly); i++) {
			assembly[i] = tolower(assembly[i]);
		}

		if (parse_command(assembly, &nr_tokens, tokens) < 0)
			continue;

		machine_code = translate(nr_tokens, tokens);

		fprintf(stderr, "0x%08x\n", machine_code);

		if (input == stdin) printf(">> ");
	}

	if (input != stdin) fclose(input);

	return EXIT_SUCCESS;
}
