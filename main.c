#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "bellow.h"
#include "blex.h"
#include "bparse.h"
#include "binterpreter.h"

#define MAX_INPUT_BUFFER_SIZE 64
#define MAX_INPUT_BUFFER_SIZESTR "64"

bool strings_equal(const char *str1, const char *str2) {
	if (strlen(str1) != strlen(str2)) {
		return false;
	}
	
	if (strcmp(str1, str2) != 0) {
		return false;
	}
	
	return true;
}

BLexer createBellowLexerFromString(char *str) {
	BLexer lexer;
	
	lexer.src = str;
	
	lexer.line = 1;
	lexer.col = 1; 
	lexer.pos = 0;	
	lexer.haderror = false;
	
	return lexer;
}

void repl() {
	BInterpreter interpreter = createBellowInterpreter();
	
	char input_buffer[MAX_INPUT_BUFFER_SIZE];
	
	printf("========== Bellow REPL ==========\n");
	printf("Type \"exit\" to end the program.\n");
	
	for (;;) {
		printf("\n> ");
		scanf("%" MAX_INPUT_BUFFER_SIZESTR "[^\n]", input_buffer);
		printf(" %s\n", input_buffer);
		flush_input_buffer();
		
		if (strings_equal(input_buffer, "exit")) {
			break;
		}
		
		
		BLexer lexer = createBellowLexerFromString(input_buffer);
		BParser parser = createBellowParser(&lexer);
		
		BProgram program = createBellowProgram(&parser);

		BInstruction instruction = parseInstruction(&parser, &program);
		
		if (lexer.haderror || parser.haderror) {
			continue;
		}
		
		executeBellowInstruction(&interpreter, instruction);
		
		freeBellowProgram(&program);
		freeBellowParser(&parser);
	}
	
}

int main(int argc, char *argv[]) {
	
	/*
	BLexer lexer = createBellowLexer("fibonacci.bellow");
	BParser parser = createBellowParser(&lexer);
	BProgram program = parseBProgram(&parser);
	
	saveBellowProgram(&program, "fibonacci.bec");
	
	
	BProgram program2 = loadBellowProgram("fibonacci.bec");
	BInterpreter interpreter = createBellowInterpreter();
	
	freeBellowParser(&parser);
	freeBellowLexer(&lexer);
	
	runBellowInterpreter(&interpreter, &program2);
	
	
	freeBellowProgram(&program);
	*/
	
	/*
	Usage:
		
		Compile source code: ./bellow.exe [file] [outputname]
		Run compiled program: ./bellow.exe [file]
		Repl: ./bellow.exe
	*/
	
	BLexer lexer;
	BParser parser;
	BInterpreter interpreter;
	BProgram program;
	
	switch(argc) {
		case 1:
			repl();
			break;
		case 2:
			interpreter = createBellowInterpreter();
			program = loadBellowProgram(argv[1]);
			
			runBellowInterpreter(&interpreter, &program);
			
			freeBellowProgram(&program);
			
			break;
		case 3:
			lexer = createBellowLexer(argv[1]);
			parser = createBellowParser(&lexer);
			program = parseBProgram(&parser);
			
			if (!lexer.haderror && !parser.haderror) {
				saveBellowProgram(&program, argv[2]);
			}
			
			freeBellowParser(&parser);
			freeBellowLexer(&lexer);
			
			break;
		default:
			printf("Invalid number of arguments.\n");
	}
	
	
	return 0;
}