#ifndef BELLOW_H_INCLUDED
#define BELLOW_H_INCLUDED

#define MAX_ARGS 4

typedef enum {
	MOV, ADD, SUB, DIV, MUL, MOD, SHR, SHL, INC, DEC,
	JMP, JNZ, JSR, JZ, JE, JNE, JLE, JGR,
	OUT, INPUT,
	RET
} BKeyword;

typedef enum {
	MODE_NUMBER,
	MODE_POINTER,
	MODE_VALUE,
	MODE_LABEL
} ArgMode;

typedef struct {
	BKeyword keyword;
	int argcount;
} P_argcount;

typedef struct {
	int mode;
	int value;
} BArgument;

typedef struct {
	BKeyword instruction;
	BArgument args[MAX_ARGS];
	int argscount;
} BInstruction;

typedef struct {
	BInstruction *program;
		
	int program_capacity;
	int program_count;
} BProgram;


void saveBellowProgram(BProgram *program, const char* filename);

BProgram loadBellowProgram(const char* filename);

#endif