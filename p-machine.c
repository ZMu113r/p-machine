// Zach Muller
// COP 3402
// P-Machine
//
// This program is the first step to creating a virtual
// machine of the p-machine, a machine that handles 
// processes with a stack, some pointers, and a few 
// registers

#include <stdio.h>
#include <stdlib.h>

#define MAX_STACK_HEIGHT 2000;
#define MAX_CODE_LENGTH 500;
#define MAX_LEXI_LEVELS 3;

typedef struct instruction 
{
	int op; // Opcode
	int r; // Register
	int L; // L
	int M; // M
} instruction;

// Instruction Fetch Cycle
int *Fetch(instruction *currentInstruction, int *IR)
{	
	IR[0] = currentInstruction->op;
	IR[1] = currentInstruction->r;
	IR[2] = currentInstruction->L;
	IR[3] = currentInstruction->M;

	return IR;
}

// Instruction Execute Cycle
void Execute(int *IR)
{

}
/*
// Find a variable in a 
// different activation record some L levels down
int base(L, base)
{
	int b1; // find base L levels down
	b1 = base;

	while(L > 0)
	{
		b1 = stack[b1+1];
		L--;
	}

	return b1;
}
*/
void print()
{
}

int main(int argc, char **argv) 
{	
	char *buffer = (char *)malloc(sizeof(char) * 10);
	int numInstructions = 0;
	int *opcode = (int *)malloc(sizeof(int));
	int *reg = (int *)malloc(sizeof(int));
	int *lex = (int *)malloc(sizeof(int));
	int *m = (int *)malloc(sizeof(int));
	//Code register = Array of instructions
	instruction **codeRegister = (instruction **)malloc(sizeof(instruction *) * numInstructions);
	int index = 0;
	int PC = 0;
	


	//Get file from command line and open it
	char *filename = argv[1];
	FILE *file = fopen(filename, "r");
	
	//Run through file once to count instructions then rewind
	while(fscanf(file, "%s", buffer) != EOF)
	{	
		numInstructions++;
	}
	rewind(file);
	numInstructions /= 4;

	
	//run through file and put instructions into
	//appropritate struct fields
	while(fscanf(file, "%d %d %d %d", opcode, reg, lex, m) != EOF)
	{	
		instruction *inst = (instruction *)malloc(sizeof(instruction));

		inst->op = *opcode;
		inst->r = *reg;
		inst->L = *lex;
		inst->M = *m;
		
		//Put new instruction into code register
		codeRegister[index] = inst;
		//printf("op = %d  r = %d  l = %d  m = %d  \n", array[index]->op, array[index]->r, array[index]->L, array[index]->M);
		index++;
	}

	while(PC < numInstructions)
	{	
		//Instruction Register 
		//Holds currently fetched instruction
		int *IR = (int *)malloc(sizeof(int) * 4);

		// Call instruction fetch on instruction where
		// the program counter is pointing
		Fetch(codeRegister[PC], IR);
		Execute(IR);
	}
	return 0;
} 