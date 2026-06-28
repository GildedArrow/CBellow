#ifndef BPARSE_H_INCLUDED
#define BPARSE_H_INCLUDED


typedef struct {
	BLexer *lexer;
	
	BL_Token current;
	BL_Token next;
	
	bool haderror;
} BParser;

BParser createBellowParser(BLexer *lexer);

BL_Token BP_Advance(BParser *parser);

#endif