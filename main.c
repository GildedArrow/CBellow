#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "blex.h"

int main(int argc, char *argv[]) {
	BLexer *lexer = createBellowLexer("fibonacci.bellow");
	
	BToken t = {LEX_NEWLINE, 10, 20};
	
	printBLexToken(t);
	
	freeBellowLexer(lexer);
	
	return 0;
}