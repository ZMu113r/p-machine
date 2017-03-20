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
#include <string.h>

#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3

// Structure definition of CPU instructions
// OP : the Opcode that defines which operation to execute
// R  : the register being operated on
// L  : the lexicographical level
// M  : Modifier
typedef struct instruction 
{
	int OP; 
	int R; 
	int L; 
	int M; 
} instruction;


// Structure definition of CPU registers that store the state
// of the CPU
// PC : the Program Counter points to the current instruction being executed
// SP : the Stack Pointer points to the current top of the stack
// BP : the Base Pointer points to the bottom of the current Activation Record
// IR : Holds the current instruction that was just fetched
typedef struct CPU_Registers
{
	int PC;
	int SP;
	int BP;
	instruction IR;
} CPU_Registers;



// String names of opcodes
char  *OPCODES[] = { "", "LIT", "RTN", "LOD", "STO", "CAL", "INC", "JMP", "JPC", "SIO", "SIO", "HALT","NEG", "ADD", "SUB", "MUL", "DIV", "ODD", "MOD", "EQL", "NEQ", "LSS", "LEQ", "GTR", "GEQ"};
// Global Stack
int Stack[MAX_STACK_HEIGHT];



void Print(int tick, int instructionNum, char *OPCODES, CPU_Registers regs[MAX_CODE_LENGTH], int Stack[])
{	
	int spCount = 7;
	int tick2 = 0;
	printf("%-4d %-4s %-4d %-4d %-4d %-4d %-4d %-4d %-4d %-4d %-4d %-4d %-4d %-2d", instructionNum, OPCODES, regs->IR.R, regs->IR.L, regs->IR.M, regs->PC, regs->BP, regs->SP, Stack[1], Stack[2], Stack[3], Stack[4], Stack[5], Stack[6]);
	while(tick > 0)
	{	
		printf(" |  ");
		while(spCount <= regs->SP)
		{	
			if(tick2 == 4)
			{
				printf(" |  ");
				tick2 = 0;
			}
			printf("%-4d", Stack[spCount]);
			tick2 += 1;
			spCount++;
		}
		tick -= 1;
	}
	printf("\n");
}


// Find a variable in a 
// different activation record some L levels down
int base(int L, int BP)
{	
	int b1; // find base L levels down
	b1 = BP;
	while(L > 0)
	{
		b1 = Stack[b1+1];
		L--;
	}
	return b1;
}


// Instruction Fetch Cycle
// an instruction is fetched from code store that
// sits at the bottom of the stack
instruction Fetch(instruction currentInstruction, CPU_Registers regs[MAX_CODE_LENGTH])
{	
	regs->IR.OP = currentInstruction.OP;
	regs->IR.R = currentInstruction.R;
	regs->IR.L = currentInstruction.L;
	regs->IR.M = currentInstruction.M;

	return regs->IR;
}


// Instruction Execute Cycle
// IR.op indicated the operation to be executed
int Execute(int *tick, int *instructionNum, int Stack[2000], int R[16], CPU_Registers regs[MAX_CODE_LENGTH], int Halt)
{	
	switch(regs->IR.OP)
	{	
		// LIT
		case 1:
			R[regs->IR.R] = regs->IR.M;
			Print(*tick, *instructionNum, OPCODES[1], regs, Stack);
			break;

		// RTN
		case 2:
			regs->SP = regs->BP - 1;
			regs->BP = Stack[regs->SP+3];
			regs->PC = Stack[regs->SP+4];

			*tick -= 1;

			Print(*tick, *instructionNum, OPCODES[2], regs, Stack);
			break;

		// LOD
		case 3:
			R[regs->IR.R] = Stack[base(regs->IR.L, regs->BP) + regs->IR.M];
			Print(*tick, *instructionNum, OPCODES[3], regs, Stack);

			break;

		// STO
		case 4:
			Stack[base(regs->IR.L, regs->BP) + regs->IR.M] = R[regs->IR.R];
			Print(*tick, *instructionNum, OPCODES[4], regs, Stack);

			break;

		// CAL
		case 5:
			Stack[regs->SP+1] = 0; // Space to return value
			Stack[regs->SP+2] = base(regs->IR.L, regs->BP); // Static Link
			Stack[regs->SP+3] = regs->BP; // Dynamic Link
			Stack[regs->SP+4] = regs->PC; // Return Adress
			regs->BP = regs->SP + 1;
			regs->PC = regs->IR.M;

			Print(*tick, *instructionNum, OPCODES[5], regs, Stack);

			*tick += 1;
			break;

		// INC
		case 6:
			regs->SP = regs->SP + regs->IR.M;
			Print(*tick, *instructionNum, OPCODES[6], regs, Stack);
			break;

		// JMP
		case 7:
			regs->PC = regs->IR.M;
			Print(*tick, *instructionNum, OPCODES[7], regs, Stack);
			*instructionNum = regs->PC;
			break;

		// JPC
		case 8:
			if(R[regs->IR.R] == 0){
				regs->PC = regs->IR.M;
				*instructionNum = regs->PC;
			}
			Print(*tick, *instructionNum, OPCODES[8], regs, Stack);
			break;

		// SIO write a register to the screen
		case 9:
			Print(*tick, *instructionNum, OPCODES[9], regs, Stack);
			break;

		// SIO Read in input from user
		// and store it in a register
		case 10:
			scanf("%d", &R[regs->IR.R]);
			Print(*tick, *instructionNum, OPCODES[10], regs, Stack);
			break;

		// End of program
		case 11:
			Print(*tick, *instructionNum, OPCODES[11], regs, Stack);
			Halt = 1;
			break;

		// NEG
		case 12:
			R[regs->IR.R] = -1 * R[regs->IR.L];
			Print(*tick, *instructionNum, OPCODES[12], regs, Stack);
			break;

		// ADD
		case 13:
			R[regs->IR.R] = R[regs->IR.L] + R[regs->IR.M];
			Print(*tick, *instructionNum, OPCODES[13], regs, Stack);
			break;

		// SUB
		case 14:
			R[regs->IR.R] = R[regs->IR.L] - R[regs->IR.M];
			Print(*tick, *instructionNum, OPCODES[14], regs, Stack);
			break;

		// MUL
		case 15:
			R[regs->IR.R] = R[regs->IR.L] * R[regs->IR.M];
			Print(*tick, *instructionNum, OPCODES[15], regs, Stack);
			break;

		// DIV
		case 16:
			R[regs->IR.R] = R[regs->IR.L] / R[regs->IR.M];
			Print(*tick, *instructionNum, OPCODES[16], regs, Stack);
			break;

		// ODD
		case 17:
			R[regs->IR.R] %= 2;
			Print(*tick, *instructionNum, OPCODES[17], regs, Stack);
			break;

		// MOD
		case 18:
			R[regs->IR.R] = R[regs->IR.L] % R[regs->IR.M];
			Print(*tick, *instructionNum, OPCODES[18], regs, Stack);
			break;

		// EQL
		case 19:
			R[regs->IR.R] = R[regs->IR.L] == R[regs->IR.M];
			Print(*tick, *instructionNum, OPCODES[19], regs, Stack);
			break;

		// NEQ
		case 20:
			R[regs->IR.R] = R[regs->IR.L] != R[regs->IR.M];
			Print(*tick, *instructionNum, OPCODES[20], regs, Stack);
			break;

		// LSS
		case 21:
			R[regs->IR.R] = R[regs->IR.L] < R[regs->IR.M];
			Print(*tick, *instructionNum, OPCODES[21], regs, Stack);
			break;

		// LEQ
		case 22:
			R[regs->IR.R] = R[regs->IR.L] <= R[regs->IR.M];
			Print(*tick, *instructionNum, OPCODES[22], regs, Stack);
			break;

		// GTR
		case 23:
			R[regs->IR.R] = R[regs->IR.L] > R[regs->IR.M];
			Print(*tick, *instructionNum, OPCODES[23], regs, Stack);
			break;

		// GEQ
		case 24:
			R[regs->IR.R] = R[regs->IR.L] >= R[regs->IR.M];
			Print(*tick, *instructionNum, OPCODES[24], regs, Stack);
			break;

		}

	return Halt;
}


int main(int argc, char **argv) 
{	
	// Variable to count number of instructions in code store
	int numInstructions = 0;
	// Buffers to hold instructions when reading file
	int *opcode = (int *)malloc(sizeof(int));
	int *reg = (int *)malloc(sizeof(int));
	int *lex = (int *)malloc(sizeof(int));
	int *m = (int *)malloc(sizeof(int));
	int index = 0;
	int Halt = 0;
	//Code register = Array of instructions
	instruction codeRegister[MAX_CODE_LENGTH];
	// "Stack" variable
	int Stack[MAX_STACK_HEIGHT];
	// Registers
	int R[16];
	// Create CPU_registers
	CPU_Registers regs[MAX_CODE_LENGTH];
	instruction IR;
	int *instructionNum = (int *)malloc(sizeof(int));
	*instructionNum = 0;
	int *tick = (int *)malloc(sizeof(int));;
	int i;

	//Initialize Registers
	regs->PC = 0;
	regs->SP = 0;
	regs->BP = 1;
	regs->IR.OP = 0;
	regs->IR.R = 0;
	regs->IR.L = 0;
	regs->IR.M = 0;
	
	//Get file from command line and open it
	char *filename = argv[1];
	FILE *file = fopen(filename, "r");
	
	// Err if file not found
	if(file == NULL)
	{	
		printf("Unable to find specified file.\n");
	}

	// Print out header for code register
	printf("%s %-4s %-4s %-4s %-4s\n", "Line", "OP", "R", "L", "M");

	//run through file and put instructions into
	//appropritate struct field
	while(fscanf(file, "%d %d %d %d", opcode, reg, lex, m) != EOF)
	{	
		instruction inst = *(instruction *)malloc(sizeof(instruction));
		inst.OP = *opcode;
		inst.R = *reg;
		inst.L = *lex;
		inst.M = *m;

		// Print out instructions in code register
		printf("%-4d %-4s %-4d %-4d %-4d\n", numInstructions++, OPCODES[*opcode], *reg, *lex, *m);

		//Put new instruction into code register
		codeRegister[index] = inst;

		index++;
	}
	// Don't forget to free
	free(opcode);
	free(reg);
	free(lex);
	free(m); 
	printf("\n");

	//Initialize Stack
	for(i = 0; i < MAX_STACK_HEIGHT; i++)
	{
		Stack[i] = 0;
	}
	// Initialize registers
	for(i = 0; i < 16; i++)
	{
		R[i] = 0;
	}

	// Print out headers
	printf("%-24s %-4s %-4s %-45s\n", "Initial Values", "PC", "BP", "SP");

	// Run instructions
	while(Halt == 0)
	{	
		regs->IR = IR;

		// Call instruction fetch on instruction where
		// the program counter is pointing
		regs->IR = Fetch(codeRegister[regs->PC], regs);

		//Increment program counter
		regs->PC += 1;
	
		//Execute instruction
		Halt = Execute(tick, instructionNum, R, Stack, regs, Halt);
		if(regs->IR.OP != 7)
		{
			*instructionNum = *instructionNum + 1;
		}
	}

	fclose(file);
	return 0;
} 