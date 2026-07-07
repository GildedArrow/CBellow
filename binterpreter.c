#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bellow.h"
#include "blex.h"
#include "bparse.h"
#include "binterpreter.h"

static const bool runtime_debug = false;

void throwRuntimeError(BInterpreter *interpreter, BRuntimeErrorType errortype) {
	interpreter->haderror = true;
	
	printf("[Runtime Error] ");
	
	switch (errortype) {
		case MEMORY_INDEX_OUT_OF_BOUNDS:
			printf("Memory index out of bounds.\n");
		default:
			printf("Unknown error\n");
	}
}

int get_number(BInterpreter *interpreter, BArgument arg) {
	switch (arg.mode) {
		case MODE_NUMBER:
			return arg.value;
		case MODE_VALUE:
			if (arg.value >= MEMORY_SIZE || arg.value < 0) {
				throwRuntimeError(interpreter, MEMORY_INDEX_OUT_OF_BOUNDS);
				return 0;
			}
		
			return interpreter->memory[arg.value];
		case MODE_POINTER:
			return interpreter->memory[interpreter->memory[arg.value]];
		default:
			return arg.value;
	}
}

void flush_input_buffer() {
	int c;
	
	while ((c = getchar()) != '\n' && c != EOF);
}

void op_mov(BInterpreter *interpreter, BArgument *args) {
	interpreter->memory[get_number(interpreter, args[1])] = get_number(interpreter, args[0]);
	
	if (runtime_debug) {
		printf("[Debug] Moving the value %i to register %i\n", get_number(interpreter, args[0]), get_number(interpreter, args[1]));
	}
}

void op_add(BInterpreter *interpreter, BArgument *args) {
	interpreter->memory[get_number(interpreter, args[2])] = get_number(interpreter, args[0]) + get_number(interpreter, args[1]);
	
	if (runtime_debug) {
		printf("[Debug] Add %i + %i to register %i\n", get_number(interpreter, args[0]), get_number(interpreter, args[1]), get_number(interpreter, args[2]));
	}
}

void op_sub(BInterpreter *interpreter, BArgument *args) {
	interpreter->memory[get_number(interpreter, args[2])] = get_number(interpreter, args[0]) - get_number(interpreter, args[1]);	
}

void op_div(BInterpreter *interpreter, BArgument *args) {
	interpreter->memory[get_number(interpreter, args[2])] = get_number(interpreter, args[0]) / get_number(interpreter, args[1]);	
}

void op_mul(BInterpreter *interpreter, BArgument *args) {
	interpreter->memory[get_number(interpreter, args[2])] = get_number(interpreter, args[0]) * get_number(interpreter, args[1]);	
}

void op_mod(BInterpreter *interpreter, BArgument *args) {
	interpreter->memory[get_number(interpreter, args[2])] = get_number(interpreter, args[0]) % get_number(interpreter, args[1]);
}

void op_shr(BInterpreter *interpreter, BArgument *args) {
	interpreter->memory[get_number(interpreter, args[0])] = interpreter->memory[get_number(interpreter, args[0])] >> get_number(interpreter, args[1]);
}

void op_shl(BInterpreter *interpreter, BArgument *args) {
	interpreter->memory[get_number(interpreter, args[0])] = interpreter->memory[get_number(interpreter, args[0])] << get_number(interpreter, args[1]);
}

void op_inc(BInterpreter *interpreter, BArgument *args) {
	interpreter->memory[get_number(interpreter, args[0])]++;
}

void op_dec(BInterpreter *interpreter, BArgument *args) {
	interpreter->memory[get_number(interpreter, args[0])]--;
}

void op_jmp(BInterpreter *interpreter, BArgument *args) {
	interpreter->pc = get_number(interpreter, args[0]);
}

void op_jnz(BInterpreter *interpreter, BArgument *args) {
	if (get_number(interpreter, args[1]) != 0) {
		interpreter->pc = get_number(interpreter, args[0]);
		
		if (runtime_debug) {
			printf("[Debug] %i != 0, jumping to location %i\n", get_number(interpreter, args[1]), get_number(interpreter, args[0]));
		}
	}
}

void op_jsr(BInterpreter *interpreter, BArgument *args) {
	interpreter->sp++;
	interpreter->callstack[interpreter->sp] = interpreter->pc;
	interpreter->pc = get_number(interpreter, args[0]);
}

void op_jz(BInterpreter *interpreter, BArgument *args) {
	if (get_number(interpreter, args[1]) == 0) {
		interpreter->pc = get_number(interpreter, args[0]);
	}
}

void op_je(BInterpreter *interpreter, BArgument *args) {
	if (get_number(interpreter, args[1]) == get_number(interpreter, args[2])) {
		interpreter->pc = get_number(interpreter, args[0]);
	}
}

void op_jne(BInterpreter *interpreter, BArgument *args) {
	if (get_number(interpreter, args[1]) != get_number(interpreter, args[2])) {
		interpreter->pc = get_number(interpreter, args[0]);
	}
}

void op_jle(BInterpreter *interpreter, BArgument *args) {
	if (get_number(interpreter, args[1]) < get_number(interpreter, args[2])) {
		interpreter->pc = get_number(interpreter, args[0]);
	}
}

void op_jgr(BInterpreter *interpreter, BArgument *args) {
	if (get_number(interpreter, args[1]) > get_number(interpreter, args[2])) {
		interpreter->pc = get_number(interpreter, args[0]);
	}	
}

void op_out(BInterpreter *interpreter, BArgument *args) {
	switch (get_number(interpreter, args[1])) {
		case 0:
			printf("%i", get_number(interpreter, args[0]));
			return;
		case 1:
			printf("%c", (char)get_number(interpreter, args[0]));
			return;
	}
}

void op_input(BInterpreter *interpreter, BArgument *args) {
	interpreter->memory[get_number(interpreter, args[0])] = getchar();
	flush_input_buffer();
}

void op_ret(BInterpreter *interpreter, BArgument *args) {
	interpreter->pc = interpreter->callstack[interpreter->sp--];
}


static const bellow_instruction BJump_Table[] = {
	[MOV] = op_mov,
	[ADD] = op_add,
	[SUB] = op_sub,
	[DIV] = op_div,
	[MUL] = op_mul,
	[MOD] = op_mod,
	[SHR] = op_shr,
	[SHL] = op_shl,
	[INC] = op_inc,
	[DEC] = op_dec,
	[JMP] = op_jmp,
	[JNZ] = op_jnz,
	[JSR] = op_jsr,
	[JZ] = op_jz,
	[JE] = op_je,
	[JNE] = op_jne,
	[JLE] = op_jle,
	[JGR] = op_jgr,
	[OUT] = op_out,
	[INPUT] = op_input,
	[RET] = op_ret
};

BInterpreter createBellowInterpreter() {
	BInterpreter interpreter;
	
	memset(interpreter.memory, 0, sizeof(interpreter.memory));
	memset(interpreter.callstack, 0, sizeof(interpreter.callstack));
	
	interpreter.pc = 0;
	interpreter.sp = -1;
	
	interpreter.haderror = false;
	
	return interpreter;
}

void executeBellowInstruction(BInterpreter *interpreter, BInstruction instruction) {
	BJump_Table[instruction.instruction](interpreter, instruction.args);
}

void runBellowInterpreter(BInterpreter *interpreter, BProgram *program) {
	while (interpreter->pc < program->program_count) {
		executeBellowInstruction(interpreter, program->program[interpreter->pc++]);
	}
}