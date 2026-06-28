#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "bellow.h"
#include "blex.h"
#include "bparse.h"

BParser createBellowParser(BLexer *lexer) {
	BParser parser;
	
	parser.lexer = lexer;
	parser.haderror = false;
	
	parser.current = nextToken(lexer);
	parser.next = nextToken(lexer);
	
	return parser;
}

BL_Token BP_Advance(BParser *parser) {
	parser->current = parser->next;
	parser->next = nextToken(parser->lexer);
}

BProgram *parseProgram(BParser *parser) {
	
}