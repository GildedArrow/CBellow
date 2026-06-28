#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bellow.h"
#include "blex.h"
#include "bparse.h"

int main(int argc, char *argv[]) {
	BLexer lexer = createBellowLexer("fibonacci.bellow");
	
	BL_Token t;
	
	while (t.type != LEX_EOF) {
		if (lexer.haderror) {
			break;
		}
		
		t = nextToken(&lexer);
		
		printBLexToken(t);
	}
	
	freeBellowLexer(&lexer);
	
	return 0;
}