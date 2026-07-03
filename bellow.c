#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bellow.h"
#include "blex.h"
#include "bparse.h"


void saveBellowProgram(BProgram *program, const char* filename) {
	FILE *pfile = fopen(filename, "wb");
	
	if (pfile == NULL) {
		perror("Failed to write file");
		return;
	}
			
	for (int i = 0; i < program->program_count; i++) {
		BInstruction instruction = program->program[i];
		
		fwrite(&instruction.instruction, sizeof(int), 1, pfile);
		
		for (int j = 0; j < MAX_ARGS; j++) {
			fwrite(&instruction.args[j].mode, sizeof(int), 1, pfile);
			fwrite(&instruction.args[j].value, sizeof(int), 1, pfile);
		}
	}
	
	fclose(pfile);
}


BProgram loadBellowProgram(const char* filename) {
	FILE *pfile = fopen(filename, "rb");
	
	BProgram program;
	
	if (pfile == NULL) {
		perror("Failed to open compiled bellow file");
		return program;
	}
	
	if (fseek(pfile, 0, SEEK_END) != 0) {
		perror("Failed to seek compiled bellow file");
		return program;
	}
	
	long filesize = ftell(pfile);
	if (filesize < 0) {
		perror("Failed to get compiled bellow file size");
		fclose(pfile);
		return program;
	}
	
	rewind(pfile);
	
	//Removing 4 bytes that is argscount and adding them back, a bit of a hack
	//and kind of bad if the size of BInstruction changes but whatever
	int num_instructs = filesize / (sizeof(BInstruction) - sizeof(int));
	program.program = malloc(filesize + sizeof(int)*num_instructs);
	
	int i = 0;
	int num;
	while (fread(&num, sizeof(int), 1, pfile) == 1) {
		BInstruction instruction;
		instruction.instruction = num;
		instruction.argscount = get_arg_count(instruction.instruction);
		
		for (int j = 0; j < MAX_ARGS; j++) {
			fread(&num, sizeof(int), 1, pfile);
			instruction.args[j].mode = num;
			
			fread(&num, sizeof(int), 1, pfile);
			instruction.args[j].value = num;
		}
		
		program.program[i++] = instruction;
	}
	
	fclose(pfile);
	
	program.program_capacity = num_instructs;
	program.program_count = num_instructs;
	
	return program;
}