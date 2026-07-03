#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "bellow.h"
#include "blex.h"

/*static const char *BReservedKeywords[] = {
	"mov", "add", "sub", "div", "mul", "mod",
	"out", "jmp", "jnz", "jsr", "jz", "ret",
	"inc", "dec", "cmp", "inp"
};
*/

static const bool lex_debug = false;


static const BL_Keyword BReservedKeywords[] = {
	{"mov",   MOV},
	{"add",   ADD},
	{"sub",   SUB},
	{"div",   DIV},
	{"mul",   MUL},
	{"mod",   MOD},
	{"shr",   SHR},
	{"shl",   SHL},
	{"inc",   INC},
	{"dec",   DEC},	
	{"jmp",   JMP},
	{"jnz",   JNZ},
	{"jsr",   JSR},
	{"jz",    JZ},
	{"out",   OUT},
	{"input", INPUT},
	{"ret",   RET}
};


void loadSource(BLexer *lexer, const char *filename) {
	FILE *file = fopen(filename, "rb");
	
	if (file == NULL) {
		perror("Error opening source file");
		return;
	}
	if (fseek(file, 0, SEEK_END) != 0) {
		perror("Error seeking source file");
		fclose(file);
		return;
	}
	
	long filesize = ftell(file);
	if (filesize < 0) {
		perror("Error getting source file size");
		fclose(file);
		return;
	}
	
	rewind(file);
	
	lexer->src = malloc(filesize + 1);
	if (lexer->src == NULL) {
		perror("Error allocating memory for source buffer");
		fclose(file);
		return;
	}
	
	size_t bytes_read = fread(lexer->src, 1, filesize, file);
	
	lexer->src[filesize] = '\0';
	
	fclose(file);
}

BLexer createBellowLexer(const char *source) {
	BLexer lexer;
	
	loadSource(&lexer, source);
	
	lexer.line = 1;
	lexer.col = 0; 
	lexer.pos = 0;
	lexer.haderror = false;
	
	return lexer;
}

void freeBellowLexer(BLexer *lexer) {
	free(lexer->src);
}

char BL_Advance(BLexer *lexer) {
	char c = lexer->src[lexer->pos++];
	
	if (c == '\n') {
		lexer->line++;
		lexer->col = 0;
	} else {
		lexer->col++;
	}
	
	return c;
}

char BL_Peek(BLexer *lexer) {
	return lexer->src[lexer->pos];
}

bool isAlpha(char c) {
	return (c > 64 && c < 91) || (c > 96 && c < 123) || (c == 95);
}

bool isNumeric(char c) {
	return c > 47 && c < 58;
}

bool isAlphaNumeric(char c) {
	return isAlpha(c) || isNumeric(c);
}

bool BL_isEof(BLexer *lexer) {
	return lexer->src[lexer->pos] == '\0';
}

void skipWhitespace(BLexer *lexer) {
	
	for (;;) {
		char c = BL_Peek(lexer);
		
		if (c == ' ' || c == '\t' || c == '\r') {
			BL_Advance(lexer);
			
			if (lex_debug) {
				printf("Skipping whitespace\n");
			}
			
			continue;
		}
		
		if (c == ';') {
			while (!BL_isEof(lexer) && BL_Peek(lexer) != '\n') {
				BL_Advance(lexer);
			}
			
			if (lex_debug) {
				printf("Skipping comments\n");
			}
			
			continue;
		}
		
		break;
	}
}

BL_Token makeToken(BLexer *lexer, BL_TokenType type) {
	BL_Token t;
	
	t.type = type;
	t.line = lexer->line;
	t.col = lexer->col;
	
	if (lex_debug) {
		printf("MAKING TOKEN %d %d\n", t.line, t.col);
	}
	
	return t;
}


BL_Token instructionOrLabel(BLexer *lexer) {
	int keyword_count = sizeof(BReservedKeywords)/sizeof(BReservedKeywords[0]);
	
	int start = lexer->pos - 1;
	
	while (isAlphaNumeric(BL_Peek(lexer))) {
		BL_Advance(lexer);
	}

	int length = lexer->pos - start;
	
	char *startpos = lexer->src + start;
	BL_Token t;
	

	for (int i = 0; i < keyword_count; i++) {
		BL_Keyword k = BReservedKeywords[i];

		if (length == strlen(k.string) && memcmp(startpos, k.string, length) == 0) {
			t = makeToken(lexer, LEX_INSTRUCTION);
			t.data.keyword = k.keyword;
						
			return t;
		}
	}
	
	t = makeToken(lexer, LEX_LABEL);
	
	t.data.string.length = length;
	t.data.string.start = startpos;

	return t;
}

BL_Token number(BLexer *lexer, char first) {
	bool negative = false;
	BL_Token t = makeToken(lexer, LEX_NUMBER);

	if (first == '-') {
		negative = true;
		if (!isNumeric(BL_Peek(lexer))) {
			printf("[Lexer] Error: Malformed string on line %i, col %i\n", lexer->line, lexer->col);
			lexer->haderror = true;
			return t;
		}
	}
	
	int value = first - '0';
	
	while (isNumeric(BL_Peek(lexer))) {
		value = value * 10 + (BL_Advance(lexer) - '0');
	}
	
	
	if (!negative) {
		t.data.value = value;
	} else {
		t.data.value = -value;
	}
	return t;
}

void printBLexToken(BL_Token t) {
	printf("[Lexer] ");
	switch (t.type) {
		case LEX_INSTRUCTION:
			printf("Instruction token   [%s]  - line %d - col %i\n", BReservedKeywords[t.data.keyword].string, t.line, t.col);
			break;
		case LEX_LABEL:
			printf("Label token         [%.*s]  - line %d - col %i\n", t.data.string.length, t.data.string.start, t.line, t.col);
		
			break;
		case LEX_PERIOD:
			printf("Period token        [.]  - line %d - col %i\n", t.line, t.col);
			break;
		case LEX_COMMA:
			printf("Comma token         [,]  - line %d - col %i\n", t.line, t.col);
			break;
		case LEX_AMPERSAND:
			printf("Ampersand token     [&]  - line %d - col %i\n", t.line, t.col);
			break;
		case LEX_HASHTAG:
			printf("Hashtag token       [#]  - line %d - col %i\n", t.line, t.col);
			break;
		case LEX_NUMBER:
			printf("Number token        [%d]  - line %d - col %i\n", t.data.value, t.line, t.col);
			break;
		case LEX_NEWLINE:
			printf("Newline token            - line %d - col %d\n", t.line, t.col);
			break;
		case LEX_EOF:
			printf("EOF token                - line %d - col %d\n", t.line, t.col);
			break;		
		default:
			printf("Unknown token            - line: %d - col %d\n", t.line, t.col);
			break;
	}
}

void throwLexerError(BLexer *lexer, char offending_char) {
	lexer->haderror = true;	
	
	printf("[Lexer] Error: invalid symbol '%c' on line %i - col %i\n", offending_char, lexer->line, lexer->col - 1);
}

BL_Token nextToken(BLexer *lexer) {
	if (lex_debug) {
		printf("Fetching next token\n");
	}
	
	if (BL_isEof(lexer)) {
		if (lex_debug) {
			printf("Eof!\n");
		}
		
		return makeToken(lexer, LEX_EOF);
	}
	
	skipWhitespace(lexer);

	char c;
	while (!BL_isEof(lexer)) {
		c = BL_Advance(lexer);
		
		switch (c) {
			case '\n':
				return makeToken(lexer, LEX_NEWLINE);
			case '.':
				return makeToken(lexer, LEX_PERIOD);
			case ',':
				return makeToken(lexer, LEX_COMMA);
			case '&':
				return makeToken(lexer, LEX_AMPERSAND);
			case '#':
				return makeToken(lexer, LEX_HASHTAG);
			case '-':
				return number(lexer, c);
		}
		
		if (isAlpha(c)) {
			return instructionOrLabel(lexer);
		}
		
		if (isNumeric(c)) {
			return number(lexer, c);
		}
		
		if (c != '\r') {
			throwLexerError(lexer, c);
		}
	}
}