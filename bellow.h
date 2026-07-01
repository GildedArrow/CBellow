#ifndef BELLOW_H_INCLUDED
#define BELLOW_H_INCLUDED

#define CALLSTACK_SIZE 64
#define MAX_ARGS 4

typedef enum {
	MOV, ADD, SUB, DIV, MUL, MOD, SHR, SHL, INC, DEC,
	JMP, JNZ, JSR, JZ,
	OUT, INPUT,
	RET
} BKeyword;

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
	int callstack[CALLSTACK_SIZE];
	
	BInstruction *program;
	
	int pc;
	int sp;
} BProgram;


#endif