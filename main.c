#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum {
	LEX_NUMBER, LEX_AMPERSAND, LEX_HASHTAG,
	LEX_LABEL, LEX_LABELDEF, LEX_COMMA, LEX_NEWLINE,
	LEX_PERIOD, LEX_INSTRUCTION
} BLexTokenType;

typedef struct {
	BLexTokenType type;
	int line;
	union data {
		int value;
		char character;
		char *string;
	};
} BToken;

typedef struct {
	char *src;
	int line, start;
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
	lexer->start = 0;
	lexer->haderror = false;
	
	return lexer;
}

void Free_Bellow_Lexer(BLexer *lexer) {
	free(lexer->src);
	free(lexer);
}

int main(int argc, char *argv[]) {
	
	BLexer *lexer = Create_Bellow_Lexer("fibonacci.bellow");
	
	printf("%s", lexer->src);
	
	
	
	Free_Bellow_Lexer(lexer);
	
	return 0;
}