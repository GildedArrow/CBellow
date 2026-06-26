#ifndef BLEX_H_INCLUDED
#define BLEX_H_INCLUDED

typedef enum {
	LEX_EOF, LEX_NUMBER, LEX_AMPERSAND, LEX_HASHTAG,
	LEX_LABEL, LEX_LABELDEF, LEX_COMMA, LEX_NEWLINE,
	LEX_PERIOD, LEX_INSTRUCTION
} BLexTokenType;


typedef struct {
	BLexTokenType type;
	int line;
	int col;
	union {
		int value;
		char character;
		char *string;
	} data;
} BToken;

typedef struct {
	char *src;
	int line, col;
	int pos;
	bool haderror;
} BLexer;


BLexer *createBellowLexer(const char *source);

void freeBellowLexer(BLexer *lexer);

void loadSource(BLexer *lexer, const char *filename);

void printBLexToken(BToken t);

BToken nextToken(BLexer *lexer);

#endif