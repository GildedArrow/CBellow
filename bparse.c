#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "bellow.h"
#include "blex.h"
#include "bparse.h"

#define MINIMUM_LIST_SIZE 8

static const bool parse_debug = false;

static const P_argcount parse_argcounts[] = {
	{MOV, 2}, {ADD, 3}, {SUB, 3}, {DIV, 3}, {MUL, 3}, {MOD, 3}, {SHR, 2}, {SHL, 2}, {INC, 1}, {DEC, 1},
	{JMP, 1}, {JNZ, 2},	{JSR, 1}, {JZ, 2}, {JE, 3}, {JNE, 3},
	{OUT, 2}, {INPUT, 1},
	{RET, 0},
};

BParser createBellowParser(BLexer *lexer) {
	BParser parser;
	
	parser.lexer = lexer;
	parser.haderror = false;
	

	parser.current = nextToken(lexer);
	parser.next = nextToken(lexer);

	parser.labelcount = 0;	
	parser.labeldefcapacity = MINIMUM_LIST_SIZE;
	parser.labels = malloc(sizeof(BLabelDefinition)*parser.labeldefcapacity);
	
	parser.labelrefcount = 0;
	parser.labelrefcapacity = MINIMUM_LIST_SIZE;
	parser.labelrefs = malloc(sizeof(BLabelReference)*parser.labelrefcapacity);
	
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
		case PARSE_EXPECTING_INSTRUCT:
			printf("Expecting instruction on line %i\n", parser->current.line);
			break;
		case PARSE_UNDEFINED_LABEL:
			printf("Undefined label on line %i\n", parser->current.line);
			break;
		default:
			printf("Unknown error on line %i\n", parser->current.line);
			break;
	}
}

void printBLabelDefinition(BLabelDefinition label) {
	printf("[Parser] [%.*s.%i]\n", label.string.length, label.string.start, label.line);
}

void printBLabelReference(BLabelReference label) {
	printf("[Parser] [%.*s.%i.%i]\n", label.string.length, label.string.start, label.line, label.pos);
}

void printBInstruction(BInstruction instruction) {
	printf("%i", instruction.instruction);
	
	for (int i = 0; i < instruction.argscount; i++) {
		printf(" (%i %i)", instruction.args[i].mode, instruction.args[i].value);
	}
	
	printf("\n");
}

void pushLabelDefinition(BParser *parser, BProgram *program) {
	BP_Advance(parser);
	
	BL_String str = parser->current.data.string;
		
	if (parser->labelcount >= parser->labeldefcapacity) {
		parser->labeldefcapacity *= 2;
		
		BLabelDefinition *temp = realloc(parser->labels, parser->labeldefcapacity*sizeof(BLabelDefinition));
		
		if (temp == NULL) {
			perror("Failed to resize parser label stack");
			return;
		}
		
		parser->labels = temp;
	}
	
	BLabelDefinition newlabel = {str, program->program_count};
		
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

/*
	1. Collect all label definitions and their program counter in an array, and collect all label reference
	2. On the second pass, point each
*/

void pushLabelReference(BParser *parser, BProgram *program, int pos) {
	BLabelReference label;
	
	label.string = parser->current.data.string;
	label.line = program->program_count;
	label.pos = pos;
	
	if (parser->labelrefcount >= parser->labelrefcapacity) {
		parser->labelrefcapacity *= 2;
		
		BLabelReference *temp = realloc(parser->labelrefs, parser->labelrefcapacity*sizeof(BLabelReference));
		
		if (temp == NULL) {
			perror("Cannot reallocate memory for label reference stack");
			return;
		}
		
		parser->labelrefs = temp;
	}
	
	//printBLabelReference(label);
	
	parser->labelrefs[parser->labelrefcount++] = label;
}

BInstruction parseInstruction(BParser *parser, BProgram *program) {
	BInstruction instruction = {0};
	
	BL_Token command = parser->current;
	
	if (command.type == LEX_LABEL) {
		throwParseError(parser, PARSE_UNRECOGNIZED_KEYWORD);
		return instruction;
	} else if (command.type != LEX_INSTRUCTION) {
		throwParseError(parser, PARSE_EXPECTING_INSTRUCT);
		return instruction;		
	}
	
	instruction.instruction = command.data.keyword;
	
	
	int counted_args = 0;
	int expected_argcount = get_arg_count(instruction.instruction);
	
	instruction.argscount = expected_argcount;

	while (parser->current.type != LEX_NEWLINE && parser->current.type != LEX_EOF) {
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
					return instruction;
				}
				
				BP_Advance(parser);
				
				arg.value = parser->current.data.value;
				
				break;
			case LEX_HASHTAG:
				arg.mode = MODE_VALUE;
				
				if (parser->next.type != LEX_NUMBER) {
					throwParseError(parser, PARSE_EXPECTING_NUMBER);
					return instruction;
				}
				
				BP_Advance(parser);
				
				arg.value = parser->current.data.value;

				break;
			case LEX_LABEL: //Placeholder for the first pass
				arg.mode = MODE_LABEL;
				arg.value = 0;
				
				pushLabelReference(parser, program, counted_args);
				
				break;
			case LEX_NEWLINE:
			case LEX_EOF:
				break;
			default:
				//printBLexToken(parser->current);
				throwParseError(parser, PARSE_EXPECTING_ARG);
				return instruction;
		}
		
		if (parser->current.type != LEX_NEWLINE && parser->current.type != LEX_EOF) {
			instruction.args[counted_args++] = arg;
		}

		if (parser->next.type != LEX_COMMA && expected_argcount != counted_args) {
			if (parser->next.type == LEX_NEWLINE || parser->next.type == LEX_EOF) { //incorrect number of arguments?
				if (counted_args > expected_argcount) {
					printf("%i\n", counted_args);
					throwParseError(parser, PARSE_TOO_MANY_ARGS);
					return instruction;
				} else {
					throwParseError(parser, PARSE_TOO_FEW_ARGS);
					return instruction;
				}
			}
			
			throwParseError(parser, PARSE_EXPECTING_COMMA);
			return instruction;
		} else {
			BP_Advance(parser);
		}
	}
	
	return instruction;
}

void pushInstruction(BParser *parser, BProgram *program) {
	BInstruction instruction = parseInstruction(parser, program);
	
	if (program->program_count >= program->program_capacity) {
		program->program_capacity *= 2;
		
		BInstruction *temp = realloc(program->program, program->program_capacity*sizeof(BInstruction));
		
		if (temp == NULL) {
			perror("Failed to reallocate memory for program stack");
			return;
		}
		
		program->program = temp;
	}
	
	//printBInstruction(instruction);
	
	program->program[program->program_count++] = instruction;
	return;
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
			
			
			pushLabelDefinition(parser, program);
			
			if (parse_debug) {
				printBLabelDefinition(parser->labels[parser->labelcount - 1]);
			}
			
			if (parser->next.type != LEX_NEWLINE && parser->next.type != LEX_EOF) {
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

bool is_BL_String_Equal(BL_String str1, BL_String str2) {
	if (str1.length != str2.length) {
		return false;
	}
	
	if (memcmp(str1.start, str2.start, str1.length) != 0) {
		return false;
	}
	
	return true;
}

BProgram createBellowProgram(BParser *parser) {
	BProgram program;

	program.program_capacity = MINIMUM_LIST_SIZE;
	program.program_count = 0;
	
	program.program = malloc(sizeof(BInstruction)*program.program_capacity);
	
	if (program.program == NULL) {
		printf("[Parser] Error: cannot allocate memory to initialize program");
		parser->haderror = true;
	}
	
	return program;
}

BProgram parseBProgram(BParser *parser) {
	BProgram program = createBellowProgram(parser);
	
	//First pass
	while (parser->current.type != LEX_EOF) {
		parseNextInstruction(parser, &program);
		
		if (parser->haderror) {
			break;
		}
		
		BP_Advance(parser);
	}
	
	//Second pass, this part was a bitch to figure out
	for (int i = 0; i < parser->labelrefcount; i++) {
		BLabelReference labelref = parser->labelrefs[i];
		
		bool found_definition = false;
		
		for (int j = 0; j < parser->labelcount; j++) {
			BLabelDefinition labeldef = parser->labels[j];
			
			//Found a matching label, go to its location and replace it with its program line
			if (is_BL_String_Equal(labelref.string, labeldef.string)) {
				program.program[labelref.line].args[labelref.pos].value = labeldef.line;
				found_definition = true;
				break;
			}
		}
		
		if (!found_definition) {
			throwParseError(parser, PARSE_UNDEFINED_LABEL);
			return program;
		}
	}
	
	/*
	for (int i = 0; i < program.program_count; i++) {
		BInstruction instruction = program.program[i];
		
		printBInstruction(instruction);
	}
	*/
	
	return program;
}

void freeBellowProgram(BProgram *program) {
	free(program->program);
}

void freeBellowParser(BParser *parser) {
	free(parser->labels);
	free(parser->labelrefs);
}