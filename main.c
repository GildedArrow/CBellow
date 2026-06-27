#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "blex.h"

int main(int argc, char *argv[]) {
	BLexer *lexer = createBellowLexer("fibonacci.bellow");
		
	BL_Token t;
	
	while (t.type != LEX_EOF) {
		t = nextToken(lexer);
		
		printBLexToken(t);
		
		if (lexer->haderror) {
			break;
		}
	}
	
	freeBellowLexer(lexer);
	
	return 0;
}