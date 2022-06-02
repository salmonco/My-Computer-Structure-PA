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
#include <errno.h>
#include <ctype.h> //isspace() 사용

 /* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#define MAX_NR_TOKENS 32	/* Maximum number of tokens in a command */
#define MAX_TOKEN_LEN 64	/* Maximum length of single token */
#define MAX_COMMAND	256		/* Maximum length of command string */

/***********************************************************************
 * parse_command(command, nr_tokens, tokens)
 *
 * DESCRIPTION
 *	Parse @command, and put each command token into @tokens[] and the number of
 *	tokes into @nr_tokens.
 *
 *  A command token is defined as a string without any whitespace (i.e., *space*
 *  and *tab* in this programming assignment) in the middle. For exmaple,
 *    command = "  Hello world   Ajou   University!!  "
 *
 *  then, the command can be split into four command tokens, which are;
 *   tokens[0] = "Hello"
 *   tokens[1] = "world"
 *   tokens[2] = "Ajou"
 *   tokens[3] = "University!!"
 *
 *  Accordingly, nr_tokens should be 4. Another exmaple is;
 *   command = "  add r0   r1 r2 "
 *
 *  then, nr_tokens = 4, and tokens are
 *   tokens[0] = "add"
 *   tokens[1] = "r0"
 *   tokens[2] = "r1"
 *   tokens[3] = "r2"
 *
 *
 * RESTRICTION
 *  DO NOT USE strtok or equivalent libraries. You should implement the
 *  feature by your own to get the points.
 *
 *
 * RETURN VALUE
 *	Return 0 after filling in @nr_tokens and @tokens[] properly
 *
 */
static int parse_command(char* command, int* nr_tokens, char* tokens[])
{
	/* TODO
	 * Followings are example code. You should delete them and implement
	 * your own code here
	 */

	 // nr_tokens에 토큰 개수 할당
	 // tokens[]에 토큰 문자열 할당

	int cmdIndex = 0; //command 인덱스
	int tokIndex = 0; //tokens 인덱스
	int firstPoint = 0; //command 배열에서 토큰의 인덱스 시작 위치
	int endPoint = 0; //command 배열에서 토큰의 인덱스 끝 위치
	static char resultStr[MAX_TOKEN_LEN] = { '\0' }; //최종 만들어질 문자열 = 토큰

	int cmdLen = 0; //command 길이
	while (command[cmdLen] != '\0') cmdLen++; //command 길이를 계산한다.

	for (; cmdIndex < cmdLen; cmdIndex++) {
		if (isspace(command[cmdIndex]) == 0) { //공백이 아닌 문자를 만남 -> 토큰 시작
			firstPoint = cmdIndex; //토큰의 처음 위치를 저장해둔다.

			for (; cmdIndex < cmdLen; ++cmdIndex) { //계속해서 command 스캔
				if (isspace(command[cmdIndex]) != 0) { //공백 문자를 만남 -> 토큰 끝
					endPoint = cmdIndex; //토큰의 끝 위치를 저장해둔다.

					// 문자 이어붙이기
					int strIndex = 0; //토큰 문자열 인덱스

					//command 인덱스의 요소 문자를 각각 배열 인덱스에 담는다.
					for (int i = firstPoint; i < endPoint; i++, strIndex++) {
						resultStr[strIndex] = command[i];

						//printf("---->>>>>>>>>>>>myCharcat----  strIndex / i : %d / %d\n", strIndex, i);
						//printf("---->>>>>>>>>>>>myCharcat---- resultStr : %s\n", resultStr);
					}
					resultStr[strIndex] = '\0'; //마지막에 널문자를 넣어줌으로써 문자열을 완성한다.

					int strLen = 0; //resultStr 길이
					while (resultStr[strLen] != '\0') strLen++; //resultStr 길이를 계산한다.

					tokens[tokIndex] = (char*)malloc(sizeof(char) * (strLen + 1)); //동적 메모리 할당

					//만들어진 토큰 문자열을 tokens[] 메모리로 복사한다.
					int x = 0;
					while (x < strLen + 1) {
						tokens[tokIndex][x] = resultStr[x];
						x++;
					}
					//strcpy(tokens[tokIndex], resultStr); //만들어진 토큰 문자열을 tokens[] 메모리로 복사한다.
					//tokens[tokIndex] = resultStr; //만들어진 토큰 문자열을 tokens[]에 할당한다.
					//printf("---->>>>>>>>>>>>[[[[[[[[myCharcat---- tokens[tokIndex] : %s\n", tokens[tokIndex]);

					tokIndex++;
					break;
				}
				else continue;
			}
		}
		else continue;
	}
	*nr_tokens = tokIndex; //tokIndex 값이 토큰의 개수가 된다.

	return 0;
}


/***********************************************************************
 * The main function of this program. DO NOT CHANGE THE CODE BELOW
 */
int main(int argc, char* const argv[])
{
	char line[MAX_COMMAND] = { '\0' };
	FILE* input = stdin;

	if (argc == 2) {
		input = fopen(argv[1], "r");
		if (!input) {
			fprintf(stderr, "No input file %s\n", argv[1]);
			return -EINVAL;
		}
	}

	while (fgets(line, sizeof(line), input)) {
		char* tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens;

		parse_command(line, &nr_tokens, tokens);

		fprintf(stderr, "nr_tokens = %d\n", nr_tokens);
		for (int i = 0; i < nr_tokens; i++) {
			fprintf(stderr, "tokens[%d] = %s\n", i, tokens[i]);
		}
		printf("\n");
	}

	if (input != stdin) fclose(input);

	return 0;
}
