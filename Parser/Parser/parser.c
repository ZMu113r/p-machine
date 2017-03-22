#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3

//this will be used to store tokens that meet grammar requirements
typedef struct Symbol
{
    int kind;             //const = 1, var = 2, proc = 3
    char name[10];        //name up to 11 characters
    int val;              //number in ASCII
    int level;            //L level
    int addr;             //M address
} Symbol;

//this will be used to maintain the symbol table
//symbol table is a hash table of nodes
struct Node
{
    Symbol sym;
    struct Node *next;
};

typedef struct hash
{
    struct Node *head;
    int count;
} hash;

typedef struct temporary
{
    int val;
    char ident[11];
} temp;

typedef struct instruction
{
    int OP;
    int R;
    int L;
    int M;
} instruction;

typedef struct token
{
    char name[11];
    int id;
    char sym_name[20];

} token;
//gonna define all procedures here to allow the reader to wrap their mind around what I'm doing
//also avoid implicit declaration:)
void error(int errNum, char *name);
struct Node* createNode(Symbol s);
Symbol *createSymbol(int kind, char *name, int val, int level, int addr);
struct Node *insertNode(Symbol sym, struct Node *head);
int compareSymbols(Symbol s1, Symbol s2);
int destroyNode(Symbol s, struct Node *head);
void getNextToken();
unsigned int hashValue(char *str, unsigned int len);
int lookUp(char *name);
void insertHash(int kind, char *name, int value, int level, int addr);
void factor();
void term();
void expression();
void condition();
void statement();
void convertToAssembly(int OP, int reg, int L, int M);
void constant_Declaration();
void var_Declaration();
void proc_Declaration();
void block();
void program();
void readFile();
int findBase(int, int, int[]);
char * getInstructName(int);

//needed global variables for all functions
char cur_token [11];
const unsigned int MAX_SYMBOLS = 10000;
struct hash symbol_table[10000];
struct Node insertingNode;
int addr;
int test;
temp temp_Val;
instruction code[500];
int instructionCount = 0;
token tokens[9999];
int registers[16] = {0};
int stack[MAX_STACK_HEIGHT] = {0};
FILE *scanner_input, *parser_input, *virtual_input, *output;
int var_total = 0;

//all used for converting to assembly code
int toggle = -1;
int have_condition = 0;
int saved_index;

//find base
int findBase(int L, int base, int stack[]){
    int result; // find base L levels down
    result = base;

    while(L > 0){
        result = stack[result + 1];
        L--;
    }

    return result;
}

// function that retrieves the name of an instruction
// to use when printing out that instruction
char * getInstructName(int opcode){
    switch(opcode) {
        case 1:
            return "lit";
        case 2:
            return "rtn";
        case 3:
            return "lod";
        case 4:
            return "sto";
        case 5:
            return "cal";
        case 6:
            return "inc";
        case 7:
            return "jmp";
        case 8:
            return "jpc";
        case 9:
        case 10:
        case 11:
            return "sio";
        case 12:
            return "neg";
        case 13:
            return "add";
        case 14:
            return "sub";
        case 15:
            return "mul";
        case 16:
            return "div";
        case 17:
            return "odd";
        case 18:
            return "mod";
        case 19:
            return "eql";
        case 20:
            return " neq";
        case 21:
            return "lss";
        case 22:
            return "leq";
        case 23:
            return "gtr";
        case 24:
            return "geq";
    } // end switch

    return "";
}

void virtualMachine(int print)
{
    int PC = 0; // program counter
    int SP = 0; // stack pointer
    int BP = 1; // base pointer
    instruction IR; // instruction register

    int halt = 0;
    int AR_Marker[MAX_STACK_HEIGHT] = {-1};
    int lastARIndex = 0;

    int count = 0;

    /*
    while(!feof(virtual_input)){
        fscanf(virtual_input, "%d %d %d %d", &code[count].OP, &code[count].R, &code[count].L, &code[count].M);
        count++;
    }

    fclose(virtual_input);

    */


    /*
    // Output the program in assembly language with line numbers
    printf("Line\tOP\tR\tL\tM\n");
    int i;
    for(i=0; i<count; i++){
        printf("%d\t%s\t%d\t%d\t%d\n", i, getInstructName(code[i].OP), code[i].R, code[i].L, code[i].M);
    }
    printf("\n");

    */

    // Output headers for the stack printout
    //printf("Initial Values\t\t\t\tpc\tbp\tsp\n");

    int opcode, line;
    while(halt == 0){
        line = PC;

        // fetch the instruction
        IR = code[PC];
        PC++;
        opcode = IR.OP;

        // execute the instruction
        switch (opcode){
            case 1: // LIT
                registers[IR.R] = IR.M;
                break;

            case 2: // RTN
                SP = BP - 1;
                BP = stack[SP+3];
                PC = stack[SP+4];

                AR_Marker[lastARIndex] = -1;
                break;

            case 3: // LOD
                registers[IR.R] = stack[findBase(IR.L, BP, stack) + IR.M];
                break;

            case 4: // STO
                stack[findBase(IR.L, BP, stack) + IR.M] = registers[IR.R];
                break;

            case 5: // CAL
                stack[SP + 1] = 0;
                stack[SP + 2] = findBase(IR.L, BP, stack);
                stack[SP + 3] = BP;
                stack[SP + 4] = PC;
                BP = SP + 1;
                PC = IR.M;

                // mark where to place bar in stack to separate activation records
                AR_Marker[SP] = 1;
                lastARIndex = SP;

                break;

            case 6: // INC
                SP = SP + IR.M;
                break;

            case 7: // JMP
                PC = IR.M;
                break;

            case 8: // JPC
                if(registers[IR.R] == 0)
                    PC = IR.M;
                break;

            case 9: // SIO 1
                if(print)
                    printf("%d\n", registers[IR.R]);
                break;

            case 10: // SIO 2
                scanf("%d", &registers[IR.R]);
                break;

            case 11: // SIO 3
                halt = 1;
                break;

            case 12: // NEG
                registers[IR.R] = (-1) * registers[IR.L];
                break;

            case 13: // ADD
                registers[IR.R] = registers[IR.L] + registers[IR.M];
                break;

            case 14: // SUB
                registers[IR.R] = registers[IR.L] - registers[IR.M];
                break;

            case 15: // MUL
                registers[IR.R] = registers[IR.L] * registers[IR.M];
                break;

            case 16: // DIV
                registers[IR.R] = registers[IR.L] / registers[IR.M];
                break;

            case 17: // ODD
                registers[IR.R] = registers[IR.R] % 2;
                break;

            case 18: // MOD
                registers[IR.R] = registers[IR.L] % registers[IR.M];
                break;

            case 19: // EQL
                registers[IR.R] = (registers[IR.L] == registers[IR.M]);
                break;

            case 20: // NEQ
                registers[IR.R] = (registers[IR.L] != registers[IR.M]);
                break;

            case 21: // LSS
                registers[IR.R] = (registers[IR.L] < registers[IR.M]);
                break;

            case 22: // LEQ
                registers[IR.R] = (registers[IR.L] <= registers[IR.M]);
                break;

            case 23: // GTR
                registers[IR.R] = (registers[IR.L] > registers[IR.M]);
                break;
            case 24: // GEQ
                registers[IR.R] = (registers[IR.L] >= registers[IR.M]);
                break;
        } // end switch

        if(print)
        {
            printf("%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t", line, getInstructName(IR.OP), IR.R, IR.L, IR.M, PC, BP, SP);

            // loop to print out the stack
            for(int i=1; i<=SP; i++){
                if(AR_Marker[i-1] == 1){
                    printf("|\t");
                }
                printf("%d\t", stack[i]);
            }
            printf("\n");

        }

    } // end while
}

// A neat function for error printing
void error(int errNum, char *name)
{
    switch(errNum)
    {
        case 1:
            printf("Error 1: File Not Found.\n");
            exit(1);
            break;

        case 2:
            printf("Error 2: Period Missing.\n");
            exit(2);
            break;

        case 3:
            printf("Error 3: Identifier Missing.\n");
            exit(3);
            break;

        case 4:
            printf("Error 4: Equal Sign Missing\n");
            exit(4);
            break;

        case 5:
            printf("Error 5: Number Missing For Constant Declaration.\n");
            exit(5);
            break;

        case 6:
            printf("Error 6: Semicolon Missing For Constant Declaration.\n");
            exit(6);
            break;

        case 7:
            printf("Error 7: Semicolon Missing For Integer Declaration.\n");
            exit(7);
            break;

        case 8:
            printf("Error 8: Identifier Missing For Procedure Declaration.\n");
            exit(8);
            break;

        case 9:
            printf("Error 9: Semicolon Missing For Procedure Declaration.\n");
            exit(9);
            break;

        case 10:
            printf("Error 10: Semicolon Missing For Procedure Declaration.\n");
            exit(10);
            break;

        case 11:
            printf("Error 11: Becomes Symbol Is Missing.\n");
            exit(11);
            break;

        case 12:
            printf("Error 12: Identifier Missing After Call.\n");
            exit(12);
            break;

        case 13:
            printf("Error 13: End Symbol Missing From Statement.\n");
            exit(13);
            break;

        case 14:
            printf("Error 14: Missing Preceding then After If.\n");
            exit(14);
            break;

        case 15:
            printf("Error 15: Missing do After while.\n");
            exit(15);
            break;

        case 16:
            printf("Error 16: Identifier Missing for Integer Declaration.\n");
            exit(16);
            break;

        case 17:
            printf("Error 17: Missing Closing Parenthesis.\n");
            exit(17);
            break;

        case 18:
            printf("Error 18: Declaration Missing For %s\n", name);
            exit(18);
            break;

        case 19:
            printf("Error 19: %s has already been declared.\n", name);
            exit(19);
            break;
    }
}

void convertToAssembly(int OP, int reg, int L, int M)
{
    if(instructionCount > 499)
        error(19, "");

    //puts the instruction
    code[instructionCount].OP = OP;
    code[instructionCount].R = reg;
    code[instructionCount].L = L;
    code[instructionCount].M = M;

    instructionCount++;
}

void printAssembly()
{
    for(int i = 0; i < instructionCount; i++)
        printf("%d %d %d %d\n", code[i].OP, code[i].R, code[i].L, code[i].M);

}

void printFile(){
    printf("Source:\n");
    FILE *file = fopen("input.txt", "r");

    char temp;

    temp = fgetc(file);
    while(temp != EOF) {
        printf("%c", temp);
        temp = fgetc(file);
    }

    printf("\n\nLexeme\tToken Type\n");
    fclose(file);
}

int checkSpecial(char specialChar[], char c)
{
    for(int i = 0; i < 13; i++)
        if(c == specialChar[i])
            return 1;

    return 0;

}

int contains(char c, char alpha[])
{
    for(int i = 0; i < strlen(alpha); i++)
    {
        if(c == alpha[i])
            return 1;
    }

    return 0;
}

int process(char specialChar[], FILE *file, char digits[], char letters[])
{
    //declare things we will use in this function
    char c1, c2;
    char str[12];
    int i, j;
    int in_comment = 0;
    //lets initialize this string
    //initializeString(str);
    //for(int i = 0; i < 200; i++)
    strcpy(str, "");

    //now scan in the first two characters
    c1 = fgetc(file);
    c2 = fgetc(file);

    //outer loop focuses on the checker character
    for(i = 0; c1 != EOF;){

        //inner loop focuses on building the token strings
        for(j = 0; !checkSpecial(specialChar, c2) && c2 != ' ' && c2 != '\n';){

            //the case where the user is stupid and puts too long of
            //a string
            if(j == 11 && (contains(str[0], digits) || contains(str[0], letters) || contains(str[0], specialChar))){
                printf("The string %s is too long to process, please revisit your code.", str);
                exit(2);
            }

            if(checkSpecial(specialChar, c1))
                break;

            //printf("before c1 = (%c)\n", c1);
            //append the character to the string if its not a space
            if(c1 != ' ' && c1 != '\t' && c1 != '\n'){
                //printf("Here!!!\n");
                str[j] = c1;
                str[j+1] = '\0';
                j++;
            }

            //now let c1 be equal to the checker character
            c1 = c2;

            //read the next character
            c2 = fgetc(file);

            if(c1 == EOF)
                break;
        }

        if(c1 != ' ' && c1 != '\t' && c1 != '\n'){
            str[j] = c1;
        }

        if( ( checkSpecial(specialChar, c2) && checkSpecial(specialChar, c1) ) && ( c1 == ':' || c1 == '<' || c1 == '>'))
        {

                c1 = c2;
                c2 = fgetc(file);

                if(checkSpecial(specialChar, c1)){
                    j++;
                    str[j] = c1;
                }
        }

        str[j+1] = '\0';

        if(str[0] == '/' && str[1] == '*')
            in_comment = 1;

        if(!in_comment)
            if(contains(str[0], digits) || contains(str[0], letters) || contains(str[0], specialChar))
                strcpy(tokens[i++].name, str);

        if(str[1] == '/' && str[0] == '*')
            in_comment = 0;

        c1 = c2;
        c2 = fgetc(file);

        //reset the string
        strcpy(str, "");

        //now read in the next character

    }

    return i;

}

void printTokens(int len)
{
    for(int i = 0; i < len ; i++)
        printf("%s\t%d\t\n", tokens[i].name, tokens[i].id);
}

void tokenCheck(int length, char digits[], char letters[])
{
    int var_flag = 0, dig_flag = 0;

    for(int i = 0; i < length; i++)
    {
        if(strcmp(tokens[i].name, "null") == 0)
        {
            tokens[i].id = 1;
            strcpy(tokens[i].sym_name, "nulsym ");
        }

        else if(strcmp(tokens[i].name, "+") == 0)
        {
            tokens[i].id = 4;
            strcpy(tokens[i].sym_name, "plussym ");
        }

        else if(strcmp(tokens[i].name, "-") == 0)
        {
            tokens[i].id = 5;
            strcpy(tokens[i].sym_name, "minussym ");
        }

        else if(strcmp(tokens[i].name, "*") == 0)
        {
            tokens[i].id = 6;
            strcpy(tokens[i].sym_name, "multsym ");
        }

        else if(strcmp(tokens[i].name, "/") == 0)
        {
            tokens[i].id = 7;
            strcpy(tokens[i].sym_name, "slashsym ");
        }

        else if(strcmp(tokens[i].name, "odd") == 0)
        {
            tokens[i].id = 8;
            strcpy(tokens[i].sym_name, "oddsym ");
        }

        else if(strcmp(tokens[i].name, "=") == 0)
        {
            tokens[i].id = 9;
            strcpy(tokens[i].sym_name, "eqsym ");
        }

        else if(strcmp(tokens[i].name, "<>") == 0)
        {
            tokens[i].id = 10;
            strcpy(tokens[i].sym_name, "neqsym ");
        }

        else if(strcmp(tokens[i].name, "<") == 0)
        {
            tokens[i].id = 11;
            strcpy(tokens[i].sym_name, "lessym ");
        }

        else if(strcmp(tokens[i].name, "<=") == 0)
        {
            tokens[i].id = 12;
            strcpy(tokens[i].sym_name, "leqsym ");
        }

        else if(strcmp(tokens[i].name, ">") == 0)
        {
            tokens[i].id = 13;
            strcpy(tokens[i].sym_name, "gtrsym ");
        }

        else if(strcmp(tokens[i].name, ">=") == 0)
        {
             tokens[i].id = 14;
             strcpy(tokens[i].sym_name, "geqsym ");
        }

        else if(strcmp(tokens[i].name, "(") == 0)
        {
            tokens[i].id = 15;
            strcpy(tokens[i].sym_name, "lparentsym ");
        }

        else if(strcmp(tokens[i].name, ")") == 0)
        {
            tokens[i].id = 16;
            strcpy(tokens[i].sym_name, "rparentsym ");
        }

        else if(strcmp(tokens[i].name, ",") == 0)
        {
            tokens[i].id = 17;
            strcpy(tokens[i].sym_name, "commasym ");
        }

        else if(strcmp(tokens[i].name, ";") == 0)
        {
            tokens[i].id = 18;

            strcpy(tokens[i].sym_name, "semicolonsym ");
        }

        else if(strcmp(tokens[i].name, ".") == 0)
        {
            tokens[i].id = 19;
            strcpy(tokens[i].sym_name, "periodsym ");
        }

        else if(strcmp(tokens[i].name, ":=") == 0)
        {
            tokens[i].id = 20;
            strcpy(tokens[i].sym_name, "becomessym ");
        }

        else if(strcmp(tokens[i].name, "begin") == 0)
        {
            tokens[i].id = 21;
            strcpy(tokens[i].sym_name, "beginsym ");
        }

        else if(strcmp(tokens[i].name, "end") == 0)
        {
            tokens[i].id = 22;
            strcpy(tokens[i].sym_name, "endsym ");
        }

        else if(strcmp(tokens[i].name, "if") == 0)
        {
            tokens[i].id = 23;
            strcpy(tokens[i].sym_name, "ifsym ");
        }

        else if(strcmp(tokens[i].name, "then") == 0)
        {
            tokens[i].id = 24;
            strcpy(tokens[i].sym_name, "thensym ");
        }

        else if(strcmp(tokens[i].name, "while") == 0)
        {
            tokens[i].id = 25;
            strcpy(tokens[i].sym_name, "whilesym ");
        }

        else if(strcmp(tokens[i].name, "do") == 0)
        {
            tokens[i].id = 26;
            strcpy(tokens[i].sym_name, "dosym ");
        }

        else if(strcmp(tokens[i].name, "call") == 0)
        {
            tokens[i].id = 27;
            strcpy(tokens[i].sym_name, "callsym ");
        }

        else if(strcmp(tokens[i].name, "const") == 0)
        {
            tokens[i].id = 28;
            strcpy(tokens[i].sym_name, "constsym ");
        }

        else if(strcmp(tokens[i].name, "var") == 0)
        {
             tokens[i].id = 29;
             strcpy(tokens[i].sym_name, "varsym ");
        }

        else if(strcmp(tokens[i].name, "procedure") == 0)
        {
            tokens[i].id = 30;
            strcpy(tokens[i].sym_name, "procsym ");
        }

        else if(strcmp(tokens[i].name, "write") == 0)
        {
            tokens[i].id = 31;
            strcpy(tokens[i].sym_name, "writesym ");
        }

        else if(strcmp(tokens[i].name, "read") == 0)
        {
             tokens[i].id = 32;
             strcpy(tokens[i].sym_name, "readsym ");
        }

        else if(strcmp(tokens[i].name, "else") == 0)
        {
            tokens[i].id = 33;
            strcpy(tokens[i].sym_name, "elsesym ");
        }


        else{
            //printf("Token: %s\tSize: %d\tFunction: %d\tCharacter: %c\n", tokens[i].name, strlen(tokens[i].name),
                   //contains(tokens[i].name[0], letters), tokens[i].name[0]);
            if(contains(tokens[i].name[0], letters))
            {
                for(int i = 1; i < strlen(tokens[i].name); i++)
                {

                    if(strlen(tokens[i].name) > 11)
                    {
                        printf("The variable %s is too long.\n", tokens[i].name);
                        exit(1);
                    }

                    if(!contains(tokens[i].name[i], digits) && !contains(tokens[i].name[i], letters))
                        var_flag = 1;
                }

                if(!var_flag)
                {
                    tokens[i].id = 2;
                    strcpy(tokens[i].sym_name, "identsym ");
                }

            }

            else if(contains(tokens[i].name[0], digits))
            {
                if(strlen(tokens[i].name) > 5)
                {
                    printf("The number %s is too long.\n", tokens[i].name);
                    exit(1);
                }
                for(int i = 1; i < strlen(tokens[i].name); i++)
                {
                    if(!contains(tokens[i].name[i], digits) && strlen(tokens[i].name) <= 5)
                        dig_flag = 1;


                }

                if(!dig_flag)
                {
                    tokens[i].id = 3;
                    strcpy(tokens[i].sym_name, "numbersym ");
                }

            }

            else
            {
                printf("Token %s is not found.\n", tokens[i].name);
                exit(1);
            }

            if(var_flag || dig_flag)
            {
                printf("Token %s is not found.\n", tokens[i].name);
                exit(1);
            }

        }

    }
}

void print_list(int size)
{
    printf("\nLexeme List:\n");

    for(int i = 0; i < size; i++)
    {
        if(tokens[i].id == 2)
            printf("%d %s ", tokens[i].id, tokens[i].name);
        else
            printf("%d ", tokens[i].id);
    }

    printf("\n");
}

//returns 0 if relation is not found, and 1 if it is found
int checkRelations(char* str)
{
    if(strcmp(str, "eqsym") == 0)
    {
        return 1;
    }
}
//creates a Node and returns it
struct Node *createNode(Symbol s)
{
    //create space for the new node
    struct Node *temp = (struct Node *) malloc(sizeof(struct Node));

    //initialize its next pointer to null
    temp->next = NULL;

    //update symbol
    temp->sym = s;

    //return it
    return temp;
}

//creates a symbol
Symbol *createSymbol(int kind, char *name, int val, int level, int addr)
{
    //create a symbol to return
    Symbol *temp = (Symbol *) malloc(sizeof(Symbol));


    //these are common among all symbols
    temp->kind = kind;
    strcpy(temp->name, name);

    //creating a constant
    if(kind == 1)
    {
        //set all values, and let level and addr be non-relevant
        temp->val = val;
        temp->level = -1;
        temp->addr = -1;
    }

    //otherwise we have a procedure or a variable
    else
    {
        //set variables
        temp->level = level;
        temp->addr = addr;
        temp->val = 0;
    }

    return temp;
}

//inserts Node into the linked list
struct Node *insertNode(Symbol sym, struct Node *head)
{
    //use temp
    struct Node *temp = createNode(sym);

    //is there a head?
    if(!head)
        return temp;

    //update the head of the list
    temp->next = head;

    //return this temp
    return temp;
}

//checks if two symbols are the same
//returns 0 if they are, -1 if they are not
int compareSymbols(Symbol s1, Symbol s2)
{
    //all symbols have a kind, and can only be compared if they have the same kind
    if(s1.kind == s2.kind)
    {
        //constant symbols
        if(s1.kind == 1)
        {
            //they are the same
            if(strcmp(s1.name, s2.name) == 0 && s1.val == s2.val)
                return 0;

            //otherwise they are different
            return -1;
        }

        //variable symbols or procedure
        else
        {
            //they are the same
            if(strcmp(s1.name, s2.name) == 0 && s1.level == s2.level && s1.addr == s2.addr)
                return 0;

            //otherwise they are not the same
            return -1;
        }

    }
}

//destroy's target node in linked list
//returns 0 if found, 1 otherwise
int destroyNode(Symbol s, struct Node *head)
{
    var_total--;

    //use temp node
    struct Node *temp = head;

    //keep track of prev node with walking through list
    struct Node *prev;

    //continue to loop till we find the correct symbol
    while(compareSymbols(temp->sym, s) < 0)
    {
        //set the prev to cur
        prev = temp;

        //if we arrive at the end, the node was not found
        if(temp->next == NULL)
        {
            printf("Node not found.\n");
            return - 1;
        }

        //update cur
        temp = temp->next;
    }

    //relink the linked list
    prev->next = temp->next;

    //free the unreachable node
    free(temp);

    //we found it
    return 0;
}

//this read the next token in the output
void getNextToken()
{
    fscanf(parser_input, "%s", &cur_token);

    printf("TOKEN = %s\n", cur_token);
    //scans next token as the identifier name
    if(strcmp(cur_token, "identsym") == 0)
        fscanf(parser_input, "%s", &temp_Val.ident);

    //scans the number into the identifier value
    else if(strcmp(cur_token, "numbersym") == 0)
        fscanf(parser_input, "%d", &temp_Val.val);

}

/*
Hash Value in credit to Robert Sedgewick
this will ensure proper distribution of string to integers
along with avoiding collisions
*/

unsigned int hashValue(char *str, unsigned int len)
{
   unsigned int result = 0;
   unsigned int b    = 378551;
   unsigned int a    = 63689;
   unsigned int i    = 0;

   for(i=0; i<len; str++, i++)
   {
      result = result*a + (*str);
      a = a*b;
   }

   return result%MAX_SYMBOLS;
}

//looks up the node to see if it is in the symbol_table
//returns 0 if we found it, -1 otherwise
int lookUp(char *name)
{
    //find the hash value
    int hash_val = hashValue(name, strlen(name));

    //make a temp
    struct Node *temp = symbol_table[hash_val].head;

    //if that location is null, then the node isn't there
    if(temp == NULL)
        return -1;

    //check through the list at that position for the node
    while(strcmp(temp->sym.name, name) != 0)
    {
        temp = temp->next;

        //if we get to the end, then it is not there
        if(temp == NULL)
            return -1;

    }
    //otherwise we found it
    return 0;
}

//this looks up a particular symbol and returns it
struct Node *lookUpSym(char *name)
{
    //will need this node to loop through
    struct Node *temp;

    //error if we don't find the it all
    if(lookUp(name) < 0)
        error(18, name);

    //hash the name
    int hash = hashValue(name, strlen(name));

    //initialize the temp to the head of the list
    temp = symbol_table[hash].head;

    //iterate through the linked list until we find a match
    while(strcmp(temp->sym.name, name) != 0)
        temp = temp->next;

    //return that symbol
    return temp;
}

//this inserts a symbol into the symbol table (which is really a hash table)
void insertHash(int kind, char *name, int value, int level, int addr)
{
    var_total++;

    //create the symbol
    Symbol sym = *createSymbol(kind, name, value, level, addr);

    //hash the value of the name we are trying to find
    int hash = hashValue(sym.name, strlen((sym.name)));

    //check to see if that ident has been inserted already
    if(lookUp(name) == 0)
        error(19, name);

    //insert into list
    symbol_table[hash].head = insertNode(sym, symbol_table[hash].head);

    printf("Insert Complete.\n");
}

void factor()
{
    //check for identifier
    if(strcmp(cur_token, "identsym") == 0)
    {
        if(lookUpSym(temp_Val.ident)->sym.kind == 1)
        {
            if(toggle != 0)
            {
                convertToAssembly(3, 0, 0, lookUpSym(temp_Val.ident)->sym.val);
                toggle = 0;
            }

            else
            {
                convertToAssembly(3, 1, 0, lookUpSym(temp_Val.ident)->sym.val);
                toggle = 1;
            }

        }

        else if(lookUpSym(temp_Val.ident)->sym.kind == 2)
        {

            if(toggle != 0)
            {
                convertToAssembly(3, 0, 0, lookUpSym(temp_Val.ident)->sym.addr);
                toggle = 0;
            }

            else
            {
                convertToAssembly(3, 1, 0, lookUpSym(temp_Val.ident)->sym.addr);
                toggle = 1;
            }

        }
        getNextToken();
    }


    //check for number
    else if(strcmp(cur_token, "numbersym") == 0)
    {
        if(toggle != 0)
        {
            convertToAssembly(1, 0, 0, temp_Val.val);
            toggle = 0;
        }

        else
        {
            toggle = 1;
            convertToAssembly(1, 1, 0, temp_Val.val);
        }

        getNextToken();
    }


    //check for '('
    else if(strcmp(cur_token, "lparentsym") == 0)
    {
        getNextToken();

        //call expression for the parenthesis
        expression();

        //next token has to be the closing parenthesis
        if(strcmp(cur_token, "rparentsym") != 0)
            error(17, NULL);

        getNextToken();
    }
}

void term()
{
    factor();

    //keep looping when multiplying or dividing
    while(strcmp(cur_token, "multsym") == 0 || strcmp(cur_token, "slashsym") == 0)
    {
        getNextToken();
        factor();
    }

}

void expression()
{
    //if we start with a plus or minus, then get the next token
    if(strcmp(cur_token, "plussym") == 0 || strcmp(cur_token, "minussym") == 0)
        getNextToken();

    //call the term for the expression
    term();

    //keep looping when adding or subtracting
    while(strcmp(cur_token, "plussym") == 0 || strcmp(cur_token, "minussym") == 0)
    {
        getNextToken();
        term();
    }

}

void condition()
{

    have_condition = 1;
    //current token is the odd symbol
    if(strcmp(cur_token, "oddsym") == 0)
    {
        getNextToken();

        //call the expression for the odd symb
        expression();

        convertToAssembly(17, 0, 0, 1);
    }

    else
    {
        //call expression
        expression();


        if(strcmp(cur_token, "eqsym") == 0)
        {
            getNextToken();

            //call expression again after relation
            expression();

            convertToAssembly(19, 0, 0, 1);
        }

        else if(strcmp(cur_token, "neqsym") == 0)
        {
             getNextToken();

             expression();

             convertToAssembly(20, 0, 0, 1);
        }

        else if(strcmp(cur_token, "lessym") == 0)
        {
            getNextToken();

            expression();

            convertToAssembly(21, 0, 0, 1);
        }

        else if(strcmp(cur_token, "leqsym") == 0)
        {
            getNextToken();

            expression();

            convertToAssembly(22, 0, 0, 1);
        }

        else if(strcmp(cur_token, "gtrsym") == 0)
        {
            getNextToken();

            expression();

            convertToAssembly(23, 0, 0, 1);
        }

        else if(strcmp(cur_token, "geqsym") == 0)
        {
            getNextToken();

            expression();

            convertToAssembly(24, 0, 0, 1);
        }

        else
            error(16, NULL);
    }
}

void statement()
{
    //check to see if the current token is the identifier symbol
    if(strcmp(cur_token, "identsym") == 0)
    {

        char saved_ident[11];

        strcpy(saved_ident, temp_Val.ident);

        //check to make sure that the token we get next is in the symbol table
        int answer = lookUp(temp_Val.ident);

        //if it is not then the variable is not declared
        //error this out
        if(answer < 0)
        {
            printf("Error 21: %s is not declared.\n", temp_Val.ident);
            exit(21);
        }


        getNextToken();

        //next symbol the must be the becomes, otherwise error
        if(strcmp(cur_token, "becomessym") != 0)
            error(11, NULL);

        //get next token
        getNextToken();

        //call expression
        expression();

        if(toggle == 0)
        {
            convertToAssembly(4, 0, 0, lookUpSym(saved_ident)->sym.addr);
            toggle = 1;
        }

        else if (toggle == 1)
        {
            convertToAssembly(4, 1, 0, lookUpSym(saved_ident)->sym.addr);
            toggle = 0;
        }
    }

    //check current token to see if it is call symbol
    else if(strcmp(cur_token, "callsym") == 0)
    {
        getNextToken();

        //next symbol must be the identifier symbol
        if(strcmp(cur_token, "identsym") != 0)
            error(12, NULL);

        getNextToken();
    }

    //check current token for the begins symbol
    else if(strcmp(cur_token, "beginsym") == 0)
    {
        getNextToken();

        //begins symbol initiates another statement
        statement();

        //continue to run statements until we hit something but a semicolon
        while(strcmp(cur_token, "semicolonsym") == 0)
        {
            getNextToken();

            statement();
        }

        //ending symbol for after a begin must be end, else error
        if(strcmp(cur_token, "endsym") != 0)
            error(13, NULL);

        if(have_condition)
        {
            int offset = instructionCount - saved_index;

            printf("offset = %d\nCurrent = %d\n", offset, instructionCount);
            code[instructionCount - offset].M = instructionCount;

            have_condition = 0;
        }

        getNextToken();
    }

    //current token is if symbol
    else if(strcmp(cur_token, "ifsym") == 0)
    {
        getNextToken();

        //call the condition for the if statement
        condition();

        convertToAssembly(8, toggle, 0, -1);

        printf("Instruction = %d\n", instructionCount);
        saved_index = instructionCount;

        //next symbol following the if is then, otherwise error
        if(strcmp(cur_token, "thensym") != 0)
            error(14, NULL);

        getNextToken();

        //call statement for the if statement
        statement();

        //optional else statement
        //if(strcmp(token, "elsesym") == 0)
        //{
            //getNextToken();

            //statement();
        //}

    }

    //current token is the while symbol
    else if(strcmp(cur_token, "whilesym") == 0)
    {
        getNextToken();

        //call condition for the while symbol
        condition();

        //must be followed by do symbol, else error
        if(strcmp(cur_token, "dosym") != 0)
            error(15, NULL);

        getNextToken();

        statement();
    }

    else if(strcmp(cur_token, "writesym") == 0 || strcmp(cur_token, "readsym") == 0)
    {
        getNextToken();

        if(strcmp(cur_token, "identsym") != 0)
        {
            printf("Error 24: Identifier Missing For Read In.\n");
            exit(24);
        }

        //check to make sure that the token we get next is in the symbol table
        int answer = lookUp(temp_Val.ident);

        //if it is not then the variable is not declared
        //error this out
        if(answer < 0)
        {
            printf("Error 21: %s is not declared.\n", temp_Val.ident);
            exit(21);
        }

        getNextToken();
    }
}

//if correct, inserts the declaration into the symbol table
void constant_Declaration()
{
    //loop through until we stop the constant declaration
     int condition = 1;

    //loop until token is not const
    while(condition)
    {
        getNextToken();
        //check to make sure next token is the identifer
        if(strcmp(cur_token, "identsym") != 0)
            error(3, NULL);

        //identifier name
        getNextToken();

        //check token for equal sign
        if(strcmp(cur_token, "eqsym") != 0)
            error(4, NULL);

        getNextToken();

        //we know the next token needs to be a number
        if(strcmp(cur_token, "numbersym") != 0)
            error(5, NULL);

        //insert into hash table
        insertHash(1, temp_Val.ident, temp_Val.val, 0, 0);

        getNextToken();

        //check for comma
        if(strcmp(cur_token, "commasym") == 0)
            condition = 1;
        else
            condition = 0;
    }

    //finally check for the semicolon and error otherwise
    if(strcmp(cur_token, "semicolonsym") != 0)
        error(6, NULL);

    getNextToken();
}

//if correct, declares variable and inserts into hashtable
void var_Declaration()
{
    //need this for belated condition in below loop
    int condition = 1;

    //loop until we do not find a comma
    while(condition)
    {
        getNextToken();

        //check for identifier symbol
        if(strcmp(cur_token, "identsym") != 0)
        {
            printf("Error 16: Identifier Missing for Integer Declaration.\n");
            exit(16);
        }

        //insert into hash table
        insertHash(2, temp_Val.ident, 0, 0, addr++);

        getNextToken();

        //if we have comma continue the loop
        if(strcmp(cur_token, "commasym") != 0)
            condition = 0;

    }

    //check for semicolon
    if(strcmp(cur_token, "semicolonsym") != 0)
        error(7, NULL);

    getNextToken();
}

//if correct, defines the procedure and inserts into hash table
void proc_Declaration()
{
    getNextToken();

    //check for the identifier symbol, else error
    if(strcmp(cur_token, "identsym") != 0)
        error(8, NULL);

    //inserts into the hashtable
    insertHash(3, temp_Val.ident, 0, 0, addr++);

    getNextToken();

    //check for the semicolon after declaration, else error
    if(strcmp(cur_token, "semicolonsym") != 0)
        error(9, NULL);

    getNextToken();

    //call block again for the procedure
    block();

    //check last token to be semicolon
    if(strcmp(cur_token, "semicolonsym") != 0)
        error(10, NULL);

    getNextToken();
}

void block()
{
    //token is is constant
    if(strcmp(cur_token, "constsym") == 0)
       constant_Declaration();

    //integer declaration portion
    if(strcmp(cur_token, "varsym") == 0)
        var_Declaration();

    //procedure declaration
    while(strcmp(cur_token, "procsym") == 0)
        proc_Declaration();

    convertToAssembly(6, 0, 0, 4+var_total);
    statement();
}

void program()
{
    getNextToken();

    //call block
    block();

    //make sure program ends with period
    if(strcmp(cur_token, "periodsym") != 0)
        error(2, NULL);

    convertToAssembly(11, 0, 0, 3);
}
//this read the file, general
void readFiles(int argc, char **argv)
{

    //used for scanner program
    char specialChar[13] = {'+','-','*','/','(',')','=',',','.','<','>',';',':'};
    char digits[10] = {'0','1','2', '3', '4', '5', '6', '7', '8', '9'};
    char letters[52] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k',
    'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w' , 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
    int token_len;
    int do_token_print = 0, do_assembly_print = 0, do_virtual_trace = 0;

    //now loop through arguments and set the booleans
    for(int i = 0; i < argc; i++)
    {
        //we want to print the tokens
        if(strcmp(argv[i], "-l") == 0)
            do_token_print = 1;

        //we want to print the assembly code
        else if(strcmp(argv[i], "-a") == 0)
            do_assembly_print = 1;

        //we want to print the stack trace
        else if(strcmp(argv[i], "-v") == 0)
            do_virtual_trace = 1;
    }

    scanner_input = fopen("input1.txt", "r");

    if(scanner_input)
    {
        token_len = process(specialChar, scanner_input, digits, letters);
        tokenCheck(token_len, digits, letters);
    }

    else
        error(1, NULL);

    //decision to print the tokens
    if(do_token_print)
        print_list(token_len);

    //need to output the file
    parser_input = fopen("input2.txt", "w");

    //write the tokens to that file
    for(int i = 0; i < token_len; i++)
    {
        fprintf(parser_input, "%s", tokens[i].sym_name);

        //output the variable or numbers
        if(strcmp(tokens[i].sym_name, "identsym ") == 0 || strcmp(tokens[i].sym_name, "numbersym ") == 0)
            fprintf(parser_input, "%s ", tokens[i].name);

    }


    fclose(parser_input);

    //open file to read
    parser_input = fopen("input2.txt", "r");
    virtual_input = fopen("input3.txt", "w");
    //we found the file
    if(parser_input)
    {
        //this will start the analysis
        program();
    }

    //we have not found the file, throw the first error
    else
        error(1, NULL);

    if(do_assembly_print)
        printAssembly();

    fclose(virtual_input);

    virtual_input = fopen("input3.txt", "r");

    if(virtual_input)
    {
        printAssembly();
        //virtualMachine(do_virtual_trace);
    }

    else
        error(1, NULL);
}


int main(int argc, char **argv)
{
    addr = 4;

    //start reading the tokens file
    readFiles(argc, argv);

    printf("SUCCESSFUL!");
}
