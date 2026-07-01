#ifndef BPARSE_H_INCLUDED
#define BPARSE_H_INCLUDED

typedef enum {
	PARSE_UNRECOGNIZED_KEYWORD,
	
	PARSE_EXPECTING_INSTRUCT_OR_LABEL, PARSE_EXPECTING_LABEL_DEF, PARSE_EXPECTING_NEWLINE
} BParseErrorType;

typedef struct {
	BL_String string;
	int line; //program line, not source code line!
} BLabelDefinition;

typedef struct {
	BLexer *lexer;
	
	BLabelDefinition *labels;
	
	BL_Token current;
	BL_Token next;
	
	int labelcount;
	int labeldefcapacity;
	
	int pc;
	bool haderror;
} BParser;

BParser createBellowParser(BLexer *lexer);

BProgram parseBProgram(BParser *parser);

void freeBellowProgram(BProgram *program);

void freeBellowParser(BParser *parser);

#endif