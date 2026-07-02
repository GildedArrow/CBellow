#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "bellow.h"
#include "blex.h"
#include "bparse.h"

#define MINIMUM_LIST_SIZE 8

static bool parse_debug = false;

static const P_argcount parse_argcounts[] = {
	{MOV, 2}, {ADD, 3}, {SUB, 3}, {DIV, 3}, {MUL, 3}, {MOD, 3}, {SHR, 3}, {SHL, 3}, {INC, 1}, {DEC, 1},
	{JMP, 1}, {JNZ, 2},	{JSR, 1}, {JZ, 2},
	{OUT, 2}, {INPUT, 1},
	{RET, 0},
};

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
		case PARSE_EXPECTING_ARG:
			printf("Expecting argument on line %i\n", parser->current.line);
			break;
		case PARSE_EXPECTING_LABEL_DEF:
			printf("Expecting label definition on line %i\n", parser->current.line);
			break;
		case PARSE_EXPECTING_NEWLINE:
			printf("Expecting end of statement on line %i\n", parser->current.line);
			break;
		case PARSE_EXPECTING_NUMBER:
			printf("Expecting number on line %i\n", parser->current.line);
			break;
		case PARSE_EXPECTING_COMMA:
			printf("Expecting comma on line %i\n", parser->current.line);
			break;
		case PARSE_TOO_MANY_ARGS:
			printf("Too many arguments for instruction on line %i\n", parser->current.line);
			break;
		case PARSE_TOO_FEW_ARGS:
			printf("Too few arguments for instruction on line %i\n", parser->current.line);
			break;
		default:
			printf("Unknown error on line %i\n", parser->current.line);
			break;
	}
}

void printBLabelDefinition(BLabelDefinition label) {
	printf("[Parser] [%.*s.%i]\n", label.string.length, label.string.start, label.line);
}

void printBInstruction(BInstruction instruction) {
	printf("%i", instruction.instruction);
	
	for (int i = 0; i < instruction.argscount; i++) {
		printf(" (%i %i)", instruction.args[i].mode, instruction.args[i].value);
	}
	
	printf("\n");
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
	
	parser->labels[parser->labelcount++] = newlabel;
}

int get_arg_count(BKeyword keyword) {
	int len = sizeof(parse_argcounts) / sizeof(parse_argcounts[0]);

	for (int i = 0; i < len; i++) {
		if (keyword != parse_argcounts[i].keyword) {
			continue;
		}
		
		return parse_argcounts[i].argcount;
	}
}

void pushLabelReference(BParser *parser) {
	
}

void pushInstruction(BParser *parser, BProgram *program) {
	BInstruction instruction;
	
	BL_Token command = parser->current;
	
	instruction.instruction = command.data.keyword;
	
	
	int counted_args = 0;
	int expected_argcount = get_arg_count(instruction.instruction);
	
	instruction.argscount = expected_argcount;
	
	while (parser->current.type != LEX_NEWLINE) {
		BP_Advance(parser);
		
		BArgument arg;
		
		switch (parser->current.type) {
			case LEX_NUMBER:
				arg.mode = MODE_NUMBER;
				arg.value = parser->current.data.value;
				
				break;
			case LEX_AMPERSAND:
				arg.mode = MODE_POINTER;
				
				if (parser->next.type != LEX_NUMBER) {
					throwParseError(parser, PARSE_EXPECTING_NUMBER);
					return;
				}
				
				BP_Advance(parser);
				
				arg.value = parser->current.data.value;
				
				break;
			case LEX_HASHTAG:
				arg.mode = MODE_VALUE;
				
				if (parser->next.type != LEX_NUMBER) {
					throwParseError(parser, PARSE_EXPECTING_NUMBER);
					return;
				}
				
				BP_Advance(parser);
				
				arg.value = parser->current.data.value;

				break;
			case LEX_LABEL: //Placeholder for the first pass
				arg.mode = MODE_LABEL;
				arg.value = 0;
				
				break;
			default:
				throwParseError(parser, PARSE_EXPECTING_ARG);
				return;
		}
		
		instruction.args[counted_args++] = arg;
		
		if (parser->next.type != LEX_COMMA && expected_argcount != counted_args) {
			if (parser->next.type == LEX_NEWLINE) { //incorrect number of arguments?
				if (counted_args > expected_argcount) {
					throwParseError(parser, PARSE_TOO_MANY_ARGS);
					return;
				} else {
					throwParseError(parser, PARSE_TOO_FEW_ARGS);
					return;
				}
			}
			
			throwParseError(parser, PARSE_EXPECTING_COMMA);
			return;
		} else {
			BP_Advance(parser);
		}
	}
	
	if (program->program_count >= program->program_capacity) {
		program->program_capacity *= 2;
		
		BInstruction *temp = realloc(program->program, program->program_capacity*sizeof(BInstruction));
		
		if (temp == NULL) {
			perror("Failed to reallocate memory for program stack");
			return;
		}
		
		program->program = temp;
	}
	
	printBInstruction(instruction);
	
	program->program[parser->pc++] = instruction;
}

void parseNextInstruction(BParser *parser, BProgram *program) {
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
			
			if (parse_debug) {
				printBLabelDefinition(parser->labels[parser->labelcount - 1]);
			}
			
			if (parser->next.type != LEX_NEWLINE) {
				throwParseError(parser, PARSE_EXPECTING_NEWLINE);
				break;
			}
			
			break;
		case LEX_INSTRUCTION:
			if (parse_debug) {
				printf("[Parser] Parsing instruction\n");
			}
			
			pushInstruction(parser, program);

			break;
		case LEX_EOF:
			if (parse_debug) {
				printf("[Parser] Reached end of token stream\n");
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
	
	program.program_capacity = MINIMUM_LIST_SIZE;
	program.program_count = 0;
	
	program.program = malloc(sizeof(BInstruction)*program.program_capacity);
	
	if (program.program == NULL) {
		printf("[Parser] Error: cannot allocate memory to initialize program");
		parser->haderror = true;
	}
	
	//First pass
	while (parser->current.type != LEX_EOF) {
		parseNextInstruction(parser, &program);
		
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