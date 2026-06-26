#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "blex.h"

static const char *BReservedKeywords[] = {
	"mov", "add", "sub", "div", "mul", "mod",
	"prc", "jmp", "jnz", "jsr", "jz", "ret",
	"inc", "dec", "cmp", "inp"
};

void loadSource(BLexer *lexer, const char *filename) {
	FILE *file = fopen(filename, "rb");
	
	if (file == NULL) {
		perror("Error opening source file");
		return;
	}
	if (fseek(file, 0, SEEK_END) != 0) {
		perror("Error seeking file");
		fclose(file);
		return;
	}
	
	long filesize = ftell(file);
	if (filesize < 0) {
		perror("Error getting file size");
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

BLexer *createBellowLexer(const char *source) {
	BLexer *lexer = malloc(sizeof(BLexer));
	
	if (lexer == NULL) {
		perror("Unable to allocate memory for lexer");
		return NULL;
	}
	
	loadSource(lexer, source);
	
	lexer->line = 1;
	lexer->pos = 0;
	lexer->haderror = false;
	
	return lexer;
}

void freeBellowLexer(BLexer *lexer) {
	free(lexer->src);
	free(lexer);
}

char advance(BLexer *lexer) {
	char c = lexer->src[lexer->pos++];
	
	if (c == '\n') {
		lexer->line++;
		lexer->col = 0;
	} else {
		lexer->col++;
	}
	
	return c;
}

char peek(BLexer *lexer) {
	return lexer->pos + 1;
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

bool isEof(BLexer *lexer) {
	return lexer->src[lexer->pos] == '\0';
}

void skipWhitespace(BLexer *lexer) {
	for (;;) {
		char c = peek(lexer);
		
		if (c == ' ' || c == '\t' || c == '\r') {
			advance(lexer);
			continue;
		}
		
		if (c == ';') {
			while (!isEof(lexer) && peek(lexer) != '\n') {
				advance(lexer);
			}
			continue;
		}
		
		break;
	}
}

BToken instructionOrLabel(BLexer *lexer) {
	
}

BToken number(BLexer *lexer) {
	
}

BToken makeToken(BLexer *lexer, BLexTokenType type) {
	BToken t;
	
	t.type = type;
	t.line = lexer->line;
	t.col = lexer->col;
	
	return t;
}

void printBLexToken(BToken t) {
	switch (t.type) {
		default:
			printf("Unknown token - line: %i - col %i\n", t.line, t.col);
			break;
	}
}

BToken nextToken(BLexer *lexer) {
	BToken t;
	char c;
	
	skipWhitespace(lexer);
	
	while (c != '\0') {
		c = advance(lexer);
		
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
		}
		
		if (isAlpha(c)) {
			return instructionOrLabel(lexer);
		}
		
		if (isNumeric(c)) {
			return number(lexer);
		}
	}
}