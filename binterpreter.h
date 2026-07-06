#ifndef BINTERPRETER_H_INCLUDED
#define BINTERPRETER_H_INCLUDED

#define MEMORY_SIZE 4096
#define CALLSTACK_SIZE 64


typedef enum {
	RUNTIME_CALLSTACK_OVERFLOW, RUNTIME_CALLSTACK_UNDERFLOW, MEMORY_INDEX_OUT_OF_BOUNDS
	
	
} BRuntimeErrorType;

typedef struct {
	int memory[MEMORY_SIZE];
	int callstack[CALLSTACK_SIZE];
	
	int pc;
	int sp;
	
	bool haderror;
} BInterpreter;

BInterpreter createBellowInterpreter();

typedef void (*bellow_instruction)(BInterpreter *interpreter, BArgument *args);

void executeBellowInstruction(BInterpreter *interpreter, BInstruction instruction);

void runBellowInterpreter(BInterpreter *interpreter, BProgram *program);

#endif