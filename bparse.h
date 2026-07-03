#ifndef BPARSE_H_INCLUDED
#define BPARSE_H_INCLUDED

typedef enum {
	PARSE_UNRECOGNIZED_KEYWORD,
	
	PARSE_EXPECTING_INSTRUCT_OR_LABEL, PARSE_EXPECTING_LABEL_DEF, PARSE_EXPECTING_NEWLINE,
	PARSE_EXPECTING_COMMA, PARSE_EXPECTING_NUMBER, PARSE_EXPECTING_ARG, PARSE_EXPECTING_INSTRUCT,
	
	PARSE_TOO_MANY_ARGS, PARSE_TOO_FEW_ARGS,
	
	PARSE_DUPLICATE_LABEL_DEFINITION, PARSE_UNDEFINED_LABEL
	
} BParseErrorType;

typedef struct {
	BL_String string;
	int line; //program line, not source code line!
} BLabelDefinition;

typedef struct {
	BL_String string;
	int line;
	int pos;
} BLabelReference;

typedef struct {
	BLexer *lexer;
	
	BLabelDefinition *labels;
	BLabelReference *labelrefs;
	
	BL_Token current;
	BL_Token next;
	
	int labelcount;
	int labeldefcapacity;
	
	int labelrefcount;
	int labelrefcapacity;
	
	int pc;
	bool haderror;
} BParser;

BParser createBellowParser(BLexer *lexer);

BProgram parseBProgram(BParser *parser);

int get_arg_count(BKeyword keyword);

void flush_input_buffer();

void printBInstruction(BInstruction instruction);

BProgram createBellowProgram(BParser *parser);

BInstruction parseInstruction(BParser *parser, BProgram *program);

void pushInstruction(BParser *parser, BProgram *program);

void parseNextInstruction(BParser *parser, BProgram *program);

void freeBellowProgram(BProgram *program);

void freeBellowParser(BParser *parser);

#endif