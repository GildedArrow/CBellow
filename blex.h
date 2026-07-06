#ifndef BLEX_H_INCLUDED
#define BLEX_H_INCLUDED

typedef struct {
	char *src;
	int line, col;
	int pos;
	bool haderror;
} BLexer;

typedef enum {
	LEX_ILLEGAL_CHARACTER, LEX_MALFORMED_STRING
} BLexErrorType;

typedef enum {
	LEX_EOF, LEX_NUMBER, LEX_AMPERSAND, LEX_HASHTAG,
	LEX_LABEL, LEX_COMMA, LEX_NEWLINE,
	LEX_PERIOD, LEX_INSTRUCTION, LEX_MINUS
} BL_TokenType;

typedef struct {
	const char *string;
	BKeyword keyword;
} BL_Keyword;

typedef struct {
	const char *start;
	int length;
} BL_String;

typedef struct {
	BL_TokenType type;
	int line;
	int col;
	union {
		int value;
		BL_String string;
		BKeyword keyword;
	} data;
} BL_Token;

BLexer createBellowLexer(const char *source);

void freeBellowLexer(BLexer *lexer);

void loadSource(BLexer *lexer, const char *filename);

void printBLexToken(BL_Token t);

BL_Token nextToken(BLexer *lexer);

#endif