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
#include <ctype.h> //isspace() ���

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

	 // nr_tokens�� ��ū ���� �Ҵ�
	 // tokens[]�� ��ū ���ڿ� �Ҵ�

	int cmdIndex = 0; //command �ε���
	int tokIndex = 0; //tokens �ε���
	int firstPoint = 0; //command �迭���� ��ū�� �ε��� ���� ��ġ
	int endPoint = 0; //command �迭���� ��ū�� �ε��� �� ��ġ
	static char resultStr[MAX_TOKEN_LEN] = { '\0' }; //���� ������� ���ڿ� = ��ū

	int cmdLen = 0; //command ����
	while (command[cmdLen] != '\0') cmdLen++; //command ���̸� ����Ѵ�.

	for (; cmdIndex < cmdLen; cmdIndex++) {
		if (isspace(command[cmdIndex]) == 0) { //������ �ƴ� ���ڸ� ���� -> ��ū ����
			firstPoint = cmdIndex; //��ū�� ó�� ��ġ�� �����صд�.

			for (; cmdIndex < cmdLen; ++cmdIndex) { //����ؼ� command ��ĵ
				if (isspace(command[cmdIndex]) != 0) { //���� ���ڸ� ���� -> ��ū ��
					endPoint = cmdIndex; //��ū�� �� ��ġ�� �����صд�.

					// ���� �̾���̱�
					int strIndex = 0; //��ū ���ڿ� �ε���

					//command �ε����� ��� ���ڸ� ���� �迭 �ε����� ��´�.
					for (int i = firstPoint; i < endPoint; i++, strIndex++) {
						resultStr[strIndex] = command[i];

						//printf("---->>>>>>>>>>>>myCharcat----  strIndex / i : %d / %d\n", strIndex, i);
						//printf("---->>>>>>>>>>>>myCharcat---- resultStr : %s\n", resultStr);
					}
					resultStr[strIndex] = '\0'; //�������� �ι��ڸ� �־������ν� ���ڿ��� �ϼ��Ѵ�.

					int strLen = 0; //resultStr ����
					while (resultStr[strLen] != '\0') strLen++; //resultStr ���̸� ����Ѵ�.

					tokens[tokIndex] = (char*)malloc(sizeof(char) * (strLen + 1)); //���� �޸� �Ҵ�

					//������� ��ū ���ڿ��� tokens[] �޸𸮷� �����Ѵ�.
					int x = 0;
					while (x < strLen + 1) {
						tokens[tokIndex][x] = resultStr[x];
						x++;
					}
					//strcpy(tokens[tokIndex], resultStr); //������� ��ū ���ڿ��� tokens[] �޸𸮷� �����Ѵ�.
					//tokens[tokIndex] = resultStr; //������� ��ū ���ڿ��� tokens[]�� �Ҵ��Ѵ�.
					//printf("---->>>>>>>>>>>>[[[[[[[[myCharcat---- tokens[tokIndex] : %s\n", tokens[tokIndex]);

					tokIndex++;
					break;
				}
				else continue;
			}
		}
		else continue;
	}
	*nr_tokens = tokIndex; //tokIndex ���� ��ū�� ������ �ȴ�.

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
