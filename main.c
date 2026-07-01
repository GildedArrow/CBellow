#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bellow.h"
#include "blex.h"
#include "bparse.h"

int main(int argc, char *argv[]) {
	
	BLexer lexer = createBellowLexer("fibonacci.bellow");
	BParser parser = createBellowParser(&lexer);
	BProgram program = parseBProgram(&parser);
	
	
	freeBellowProgram(&program);
	freeBellowLexer(&lexer);
	freeBellowParser(&parser);
	
	return 0;
}