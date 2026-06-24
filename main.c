#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum {
	LEX_EOF, LEX_NUMBER, LEX_AMPERSAND, LEX_HASHTAG,
	LEX_LABEL, LEX_LABELDEF, LEX_COMMA, LEX_NEWLINE,
	LEX_PERIOD, LEX_INSTRUCTION
} BLexTokenType;

static const char *BReservedKeywords[] = {
	"mov", "add", "sub", "div", "mul", "mod",
	"prc", "jmp", "jnz", "jsr", "jz", "ret",
	"inc", "dec", "cmp", "inp"
};

typedef struct {
	BLexTokenType type;
	int line;
	union data {
		int value;
		char character;
		char *string;
	} data;
} BToken;

typedef struct {
	char *src;
	int line, pos;
	int end;
	bool haderror;
} BLexer;

void Load_Source(BLexer *lexer, const char *filename) {
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

BLexer *Create_Bellow_Lexer(const char *source) {
	BLexer *lexer = malloc(sizeof(BLexer));
	
	if (lexer == NULL) {
		perror("Unable to allocate memory for lexer");
		return NULL;
	}
	
	Load_Source(lexer, source);
	
	lexer->line = 1;
	lexer->pos = 0;
	lexer->haderror = false;
	
	return lexer;
}

void Free_Bellow_Lexer(BLexer *lexer) {
	free(lexer->src);
	free(lexer);
}


char advance(BLexer *lexer) {
	return ++lexer->pos;
}

char peek(BLexer *lexer) {
	return lexer->pos + 1;
}

bool isAlpha(char c) {
	return (c > 64 && c < 91) ||
	(c > 96 && c < 123) ||
	(c == 95);
}

bool isNumeric(char c) {
	return c > 47 && c < 58;
}

bool isAlphaNumeric(char c) {
	return isAlpha(c) || isNumeric(c);
}

void skipwhitespace(BLexer *lexer) {
	
}

void skipcomments(BLexer *lexer) {
	
}

void instructionorlabel(BLexer *lexer) {
	
}

void number(BLexer *lexer) {
	
}

BToken MakeCharToken(BLexTokenType type, int line, char c) {
	BToken t;
	
	t.type = type;
	t.line = line;
	t.data.character = c;
	
	return t;
}

BToken NextToken(BLexer *lexer) {
	BToken t;
	char c;
	
	while (c != '\0') {
		c = advance(lexer);
		
		switch (c) {
			case '\0':	
				return MakeCharToken(LEX_EOF, lexer->line, '\0');
			case ' ':
				skipwhitespace(lexer);
				break;
			case ';':
				skipcomments(lexer);
				break;
			case '\n':
				return MakeCharToken(LEX_NEWLINE, lexer->line, '\n');
			case '.':
				return MakeCharToken(LEX_PERIOD, lexer->line, '.');
			case ',':
				return MakeCharToken(LEX_COMMA, lexer->line, ',');
			case '&':
				return MakeCharToken(LEX_AMPERSAND, lexer->line, '&');
			case '#':
				return MakeCharToken(LEX_HASHTAG, lexer->line, '#');			
		}
		
		if (isAlpha(c)) {
			instructionorlabel(lexer);
		}
		
		if (isNumeric(c)) {
			number(lexer);	
		}
	}
}

int main(int argc, char *argv[]) {
	BLexer *lexer = Create_Bellow_Lexer("fibonacci.bellow");
	
	NextToken(lexer);
	
	Free_Bellow_Lexer(lexer);
	
	return 0;
}