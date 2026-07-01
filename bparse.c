#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "bellow.h"
#include "blex.h"
#include "bparse.h"

#define MINIMUM_LIST_SIZE 8

static bool parse_debug = true;

BParser createBellowParser(BLexer *lexer) {
	BParser parser;
	
	parser.lexer = lexer;
	parser.haderror = false;
	
	parser.current = nextToken(lexer);
	parser.next = nextToken(lexer);
	
	parser.labeldefcapacity = MINIMUM_LIST_SIZE;
	parser.labels = malloc(sizeof(BLabelDefinition)*parser.labeldefcapacity);
	parser.labelcount = 0;
	
	parser.pc = 0;
	
	return parser;
}

BL_Token BP_Advance(BParser *parser) {
	parser->current = parser->next;
	parser->next = nextToken(parser->lexer);
}

void throwParseError(BParser *parser, BParseErrorType errortype) {
	parser->haderror = true;
	
	printf("[Parser] Error: ");
	
	switch (errortype) {
		case PARSE_UNRECOGNIZED_KEYWORD:
			printf("Unrecognized keyword on line %i\n", parser->current.line);
			break;
		case PARSE_EXPECTING_INSTRUCT_OR_LABEL:
			printf("Expecting instruction or label definition on line %i\n", parser->current.line);
			break;
		default:
			printf("Unknown error");
			break;
	}
}

void printBLabelDefinition(BLabelDefinition label) {
	printf("[Parser] [%.*s.%i]\n", label.string.length, label.string.start, label.line);
}

void pushLabelDefinition(BParser *parser) {
	BP_Advance(parser);
	
	BL_String str = parser->current.data.string;
	
	//printBLexToken(parser->current);
	
	if (parser->labelcount >= parser->labeldefcapacity) {
		parser->labeldefcapacity *= 2;
		
		BLabelDefinition *temp = realloc(parser->labels, parser->labeldefcapacity*sizeof(BLabelDefinition));
		
		if (temp == NULL) {
			perror("Failed to resize parser label stack");
			return;
		}
		
		parser->labels = temp;
	}
	
	BLabelDefinition newlabel = {str, parser->pc};
	
	parser->labels[parser->labelcount] = newlabel;
	parser->labelcount++;
}

BInstruction parseNextInstruction(BParser *parser) {
	/*
	At the beginning of a statement, there are exactly two options:
	
	Either,
	1. Defining a label (ex: ._label), or
	2. Calling a function (mov, add, jmp, etc.)
	
	*/
	
	while (parser->current.type == LEX_NEWLINE) {
		BP_Advance(parser);
	}
	
	switch (parser->current.type) {
		case LEX_PERIOD: //start a label definition with a period
			if (parse_debug) {
				printf("[Parser] Parsing label definition\n");
			}
		
		
			if (parser->next.type != LEX_LABEL) {
				throwParseError(parser, PARSE_EXPECTING_LABEL_DEF);
				break;
			}
			
			pushLabelDefinition(parser);
			printBLabelDefinition(parser->labels[parser->labelcount - 1]);
			
			parser->pc++;
			
			if (parser->next.type != LEX_NEWLINE) {
				throwParseError(parser, PARSE_EXPECTING_NEWLINE);
				break;
			}
			
			break;
		case LEX_INSTRUCTION:
			//TBA
			if (parse_debug) {
				printf("[Parser] Parsing instruction\n");
			}
			
			break;
		default:
			printBLexToken(parser->current);
			throwParseError(parser, PARSE_EXPECTING_INSTRUCT_OR_LABEL);
			break;
	};
}

BProgram parseBProgram(BParser *parser) {
	BProgram program;
	
	//initialize program struct data
	program.pc = 0;
	program.sp = 0;
	memset(program.callstack, 0, sizeof(program.callstack)); //zero out callstack
	
	program.program = malloc(sizeof(BInstruction)*MINIMUM_LIST_SIZE);
	
	if (program.program == NULL) {
		printf("[Parser] Error: cannot allocate memory to initialize program");
		parser->haderror = true;
	}
	
	//First pass
	while (parser->current.type != LEX_EOF) {
		parseNextInstruction(parser);
		
		if (parser->haderror) {
			break;
		}
		
		BP_Advance(parser);
	}
	
	return program;
}

void freeBellowProgram(BProgram *program) {
	free(program->program);
}

void freeBellowParser(BParser *parser) {
	free(parser->labels);
}