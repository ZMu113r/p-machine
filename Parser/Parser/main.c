#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define IDENT_LIMIT 11
#define DIGIT_LIMIT 5
#define SYMBOL_LIMIT 10000
#define MAX_CODE_LENGTH 500
#define MAX_STACK_HEIGHT 2000
#define TOTAL_REGISTERS 16
#define MAX_PROCEDURES 99
#define MAX_LEVEL 3

//this will be used to store tokens that meet grammar requirements
typedef struct Symbol
{
    int kind;             //const = 1, var = 2, proc = 3
    char name[20];        //name up to 11 characters
    int val;              //number in ASCII
    int level;            //L
    int addr;             //M address
} Symbol;

//this will be used to maintain the symbol table
//symbol table is a hash table of nodes
struct Node
{
    Symbol sym;
    struct Node *next;
};

//this is the structure for the hash table of linked lists
//count is used to see total number of nodes for each list
typedef struct hash
{
    struct Node *head;
    int count;
} hash;

//used for the sym's
typedef enum{nulsym=1, identsym=2, numbersym=3, plussym=4, minussym=5, multsym=6,
    slashsym=7, oddsym=8, eqsym=9, neqsym=10, lessym=11, leqsym=12,
    gtrsym=13, geqsym=14, lparentsym=15, rparentsym=16, commasym=17, semicolonsym=18,
    periodsym=19, becomessym=20, beginsym=21, endsym=22, ifsym=23, thensym=24,
    whilesym=25, dosym=26, callsym=27, constsym=28, varsym=29, procsym=30, writesym=31,
    readsym=32, elsesym=33, intsym=34} token_type;

//this is used to store the current token, and possibly the next token if it is an
//ident or number
typedef struct
{
    token_type token;
    int token_cnt;
    char ident[IDENT_LIMIT];
    int number;
} this_token;

//this is the basic outline of each instruction
typedef struct instruction
{
    int OP;
    int R;
    int L;
    int M;
} instruction;

//this will be the entire set of instructions
typedef struct instructions
{
    instruction instructions[MAX_CODE_LENGTH];
    int IC;
} instructions;

//this struct is used to store total number of procedures and there locations
typedef struct
{
    int savedProc[MAX_PROCEDURES];
    int count;
} savedProcIC;

//this stores the procedure names
typedef struct
{
    char array[MAX_PROCEDURES][IDENT_LIMIT];
    int count;
} procedure_store;

//file used by all parts programs
FILE *fp;

/* **** these are the global variables for the scanner **** */
//this is used to keep track of the number of tokens for the scanner
int tokenCount = 0;
//array holds all the lexemes and tokens
token_type array[MAX_CODE_LENGTH];
//this keeps track of identifiers and numbers
char tokens[MAX_CODE_LENGTH][IDENT_LIMIT];

/* **** parser global variables **** */
//this is the current token
this_token cur_token;
//this is the symbol table (hash table)
hash symbol_table[SYMBOL_LIMIT];
//this next global variables i the address counter for the symbol table
int addr;
//this represents where the instruction count and instructions will be stored
instructions code;
//this will keep track of whcih registers are being used and which are not when emitting
//E.G. if we store something in reg 0, then we want to make sure to not use that reg
//until we are done using it
int reg_count = 0;
//the F is used to as a global variable for all instructions as well as the defining of these variables
int level = 0;
//this is the current lexigraphical level
int lex_level = 0;
//this is to indicate when we are inside main to properly
//calibrate the procedure jumps
int start_main = 0;
//this is used to saved positions of the procedures
savedProcIC saved;
//first, follow and other sets used for error recovery
token_type declbegsys[3] = {constsym, varsym, procsym};
token_type statbegsys[4] = {beginsym, callsym, ifsym, whilesym};
token_type facebegsys[3] = {identsym, numbersym, lparentsym};
//total error
int total_error = 0;

/* *** Virtual Machine Global Variables *** */
//these are the registers used to do arithmetic
int registers[TOTAL_REGISTERS] = {0};
//this is is used to control what is on the stack
int stack[MAX_STACK_HEIGHT] = {0};

/* *** these are all the functions with there appropriate comments * ***/
//this will print input1.txt
int countLine(int lineCount, char input[300][MAX_CODE_LENGTH]);

//takes array of tokens, and matches it to the correct token name
int matchTokenTypes(char tokens[MAX_CODE_LENGTH][IDENT_LIMIT], int tokenCount);

//this will print an error and exit with a particular value
//the name index is used for more specific error messages
void error(int error_num, char *name);

//this prints the tokens when we have a -l
void printTokens(int tokenCount);

//reads next token into the global variable token
void getNextToken();

//this starts the parser
void program(int print_assembly);

//deals with declaration and calls statement
void block(token_type *fsys, int size);

//this deals with various amounts of things
//this includes the beginnings of assignments
//calling a procedure
//any begin to end block
//if statements
//while loops
void statement(token_type *fsys, int size);

//this function just checks the current token to see if it is a relation
//this is used only in condition
int checkRelation();

//this deals with any sort of if or while condition
void condition(token_type *fsys, int size);

//this deals with any number of subtraction or addition
void expression(token_type *fsys, int size);

//this function deals with any number of multiplication or division
void term(token_type *fsys, int size);

//this function is the bottom most function of the parser and deals
//with resolving identifiers, numbers, and parenthesis
void factor(token_type *fsys, int size);

//this creates a node given a symbol
struct Node *createNode(Symbol s);

//this creates a symbol based off of given information
Symbol *createSymbol(int kind, char *name, int val, int level, int addr);

//this inserts a symbol into a list with a head
struct Node *insertNode(Symbol sym, struct Node *head);

//This hash value was created by Robert Sedgewick and performs
//at a high enough optimal level for this program
unsigned int hashValue(char *str, unsigned int len);

//this looks something up the identifier in the symbol table
//returns 1 if found and 0 otherwise
int lookUp(char *name, int isDecl);

//looks up a particular symbol and returns it
struct Node *lookUpSym(char *name, int isDecl);

//inserts a symbol into the hash table
void insertHash(int kind, char *name, int value, int level, int addr);

//this function takes some values and puts it into the code array that stores the
//assembly code, it also checks to see if the code us too long, and increments the
//instruction count (IC)
void emit(int OP, int reg, int L, int M);

//this function will print the assembly code
void print_Assembly();

//this function decrements the register count or increments regCount depending on OP value
void changeRegCount(int operation);

//this function represents only the VM
void virtualMachine(int print);

//this function finds the base of the stack in question
int findBase(int L, int base, int stack[]);

//this function focuses on returning a string in accordance to a OP code
char * getInstructName(int opcode);

//this is the function that allows for error recovery
//we will pass the enum of which function we are in as well
//as the error number
//first set and follow set of each function are stored in here, and will be accessed
//depending on the function
void test(token_type *s1, token_type *s2, int size1, int size2, int error_num, char *name);
//this function will take a set and an element of that set, return 1
//if the element is in the set and zero otherwise
int isElement(token_type elem, token_type *set, int size);
//this function takes two array's and unions them together
//luckily the array sizes will be really small, so a naive approach is used
token_type *unionTok(token_type *s1, token_type *s2, int size1, int size2);

/* *** The start of the actual program *** */

token_type *unionTok(token_type *s1, token_type *s2, int size1, int size2)
{
    token_type *newArray = (token_type*) malloc((size1 + size2)*sizeof(token_type));

    int i, j;

    for(i = 0; i < size1; i++)
        newArray[i] = s1[i];

    for(j = 0; j < size2; j++)
        newArray[i+j] = s2[j];

    return newArray;
}

int isElement(token_type elem, token_type *set, int size)
{
    for(int i = 0; i < size; i++)
    {
        if(elem == set[i])
            return 1;
    }

    return 0;
}
void test(token_type *s1, token_type *s2, int size1, int size2, int error_num, char *name)
{

    if(!isElement(cur_token.token, s1, size1))
    {
        error(error_num, name);

        while(!isElement(cur_token.token, s1,  size1) &&  !isElement(cur_token.token, s2,  size2))
            getNextToken();
    }
}

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

int findBase(int L, int base, int stack[])
{
    int result;

    result = base;

    while(L > 0){
        result = stack[result + 1];

        L--;
    }

    return result;
}

void virtualMachine(int print)
{
    int PC = 0;
    int SP = 0;
    int BP = 1;
    instruction IR;
    int halt = 0;
    int AR_Marker[MAX_STACK_HEIGHT] = {-1};
    int lastARIndex = 0;

    if(print)
        printf("Initial Values\t\t\t\tpc\tbp\tsp\n");

    int opcode, line;

    while(halt == 0){
        line = PC;

        IR = code.instructions[PC];

        PC++;

        opcode = IR.OP;

        switch (opcode){
            case 1:
                registers[IR.R] = IR.M;

                break;

            case 2:
                SP = BP - 1;

                BP = stack[SP+3];

                PC = stack[SP+4];

                AR_Marker[lastARIndex] = -1;

                break;

            case 3:
                registers[IR.R] = stack[findBase(IR.L, BP, stack) + IR.M];

                break;

            case 4:
                stack[findBase(IR.L, BP, stack) + IR.M] = registers[IR.R];
                break;

            case 5:
                stack[SP + 1] = 0;

                stack[SP + 2] = findBase(IR.L, BP, stack);

                stack[SP + 3] = BP;

                stack[SP + 4] = PC;

                BP = SP + 1;

                PC = IR.M;

                AR_Marker[SP] = 1;

                lastARIndex = SP;

                break;

            case 6:
                SP = SP + IR.M;

                break;

            case 7:
                PC = IR.M;

                break;

            case 8:
                if(registers[IR.R] == 0)
                    PC = IR.M;

                break;

            case 9:
                printf("%d\n", registers[IR.R]);

                break;

            case 10:
                scanf("%d", &registers[IR.R]);

                break;

            case 11:
                halt = 1;

                break;

            case 12:
                registers[IR.R] = (-1) * registers[IR.L];

                break;

            case 13:
                registers[IR.R] = registers[IR.L] + registers[IR.M];

                break;

            case 14:
                registers[IR.R] = registers[IR.L] - registers[IR.M];

                break;

            case 15:
                registers[IR.R] = registers[IR.L] * registers[IR.M];

                break;

            case 16:
                registers[IR.R] = registers[IR.L] / registers[IR.M];

                break;

            case 17:
                registers[IR.R] = registers[IR.R] % 2;

                break;

            case 18:
                registers[IR.R] = registers[IR.L] % registers[IR.M];

                break;

            case 19:
                registers[IR.R] = (registers[IR.L] == registers[IR.M]);

                break;

            case 20:
                registers[IR.R] = (registers[IR.L] != registers[IR.M]);

                break;

            case 21:
                registers[IR.R] = (registers[IR.L] < registers[IR.M]);

                break;

            case 22:
                registers[IR.R] = (registers[IR.L] <= registers[IR.M]);

                break;

            case 23:
                registers[IR.R] = (registers[IR.L] > registers[IR.M]);

                break;
            case 24:
                registers[IR.R] = (registers[IR.L] >= registers[IR.M]);

                break;
        }

        if(print)
        {
            printf("%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t", line, getInstructName(IR.OP), IR.R, IR.L, IR.M, PC, BP, SP);

            for(int i=1; i<=SP; i++)
            {
                if(AR_Marker[i-1] == 1)
                    printf("|\t");

                printf("%d\t", stack[i]);
            }

            printf("\n");
        }

    }
}

void changeRegCount(int operation)
{
    if(operation == 0)
    {
        if(reg_count > 0)
            reg_count--;
    }

    else
        reg_count++;

}

void emit(int OP, int reg, int L, int M)
{
    if(code.IC > 499)
        error(17, "");

    code.instructions[code.IC].OP = OP;
    code.instructions[code.IC].R = reg;
    code.instructions[code.IC].L = L;
    code.instructions[code.IC].M = M;

    code.IC++;
}

void searchReserves()
{
    token_type reservedwords[12];

    reservedwords[0] = constsym;

    reservedwords[1] = varsym;

    reservedwords[2] = beginsym;

    reservedwords[3] = endsym;

    reservedwords[4] = ifsym;

    reservedwords[5] = thensym;

    reservedwords[6] = whilesym;

    reservedwords[7] = dosym;

    reservedwords[8] = readsym;

    reservedwords[9] = writesym;

    reservedwords[10] = oddsym;

    reservedwords[11] = procsym;

    for(int i = 0; i < 12; i++)
    {
        if(cur_token.token == reservedwords[i])
            error(16, cur_token.ident);
    }
}

struct Node *createNode(Symbol s)
{
    struct Node *temp = (struct Node *) malloc(sizeof(struct Node));

    temp->next = NULL;

    temp->sym = s;

    return temp;
}

Symbol *createSymbol(int kind, char *name, int val, int level, int addr)
{
    Symbol *temp = (Symbol *) malloc(sizeof(Symbol));

    temp->kind = kind;

    strcpy(temp->name, name);

    if(kind == 1)
    {
        temp->val = val;
        temp->level = -1;
        temp->addr = -1;
    }

    else
    {
        temp->level = level;
        temp->addr = addr;
        temp->val = 0;
    }

    return temp;
}

struct Node *insertNode(Symbol sym, struct Node *head)
{
    struct Node *temp = createNode(sym);

    if(!head)
        return temp;

    temp->next = head;

    return temp;
}

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

   return result%SYMBOL_LIMIT;
}

int lookUp(char *name, int isDecl)
{
    int hash_val = hashValue(name, strlen(name));

    struct Node *temp = symbol_table[hash_val].head;

    if(temp == NULL)
        return -1;

    while(1)
    {
        if(isDecl)
        {
            if(strcmp(temp->sym.name, name) == 0 && temp->sym.level == level)
            break;
        }

        else
        {
            if(strcmp(temp->sym.name, name) == 0 && temp->sym.level <= level)
            break;
        }

        temp = temp->next;

        if(temp == NULL)
            return -1;
    }

    return 0;
}

struct Node *lookUpSym(char *name, int isDecl)
{
    struct Node *temp;

    if(lookUp(name, isDecl) < 0)
        error(15, name);

    int hash = hashValue(name, strlen(name));

    temp = symbol_table[hash].head;

    if(isDecl)
    {
        while(strcmp(temp->sym.name, name) != 0 && temp->sym.level == level)
        temp = temp->next;
    }

    else
    {
        while(strcmp(temp->sym.name, name) != 0 && temp->sym.level == level)
        {
            if(temp != NULL)
                temp =  temp->next;
            else break;
        }

        if(temp != NULL)
            return temp;

        temp = symbol_table[hash].head;

        while(strcmp(temp->sym.name, name) != 0 && temp->sym.level <= level)
        {
            temp = temp->next;
        }
    }

    return temp;
}

void insertHash(int kind, char *name, int value, int lev, int addr)
{
    Symbol sym = *createSymbol(kind, name, value, lev, addr);

    int hash = hashValue(sym.name, strlen((sym.name)));

    if(lookUp(name, 1) == 0)
        error(22, name);

    if(kind == 2 || kind == 3)
        sym.level = lev;

    symbol_table[hash].head = insertNode(sym, symbol_table[hash].head);
}

void getNextToken()
{
    cur_token.token = array[cur_token.token_cnt];

    if(cur_token.token == identsym)
        strcpy(cur_token.ident,tokens[cur_token.token_cnt]);


    else if(cur_token.token == numbersym)
        cur_token.number = atoi(tokens[cur_token.token_cnt]);

    cur_token.token_cnt++;

}

void error(int error_num, char *name)
{
    if(error_num == -1)
    {
        printf("File was not found.\n");
        exit(-1);
    }

    else if(error_num == 20)
    {
        printf("A lexical error was detected.\n");
        exit(20);
    }

    else if(error_num == 2)
        printf("Period symbol Missing.\n");

    else if(error_num == 3)
        printf("Identifier symbol is missing after declaration.\n");

    else if(error_num == 4)
        printf("Equal sign required for constant declaration.\n");

    else if(error_num == 5)
        printf("Number should follow a constant declaration.\n");

    else if(error_num == 6)
        printf("Semicolon is missing.\n");

    else if(error_num == 7)
        printf("Becomes symbol missing after identifier.\n");

    else if(error_num == 8)
        printf("Identifier missing for call statement.\n");

    else if(error_num == 9)
        printf("End symbol missing for begins symbol.\n");

    else if(error_num == 10)
        printf("Then symbol missing after preceding if.\n");

    else if(error_num == 11)
        printf("Do symbol missing after preceding while.\n");

    else if(error_num == 12)
        printf("A relation must follow the identifier in a condition.\n");

    else if(error_num == 13)
        printf("Right parenthesis is needed following a left parenthesis\n");

    else if(error_num == 14)
        printf("Number or identifier expected.\n");

    else if(error_num == 15)
        printf("The identifier %s is not declared\n", name);

    else if(error_num == 16)
        printf("An identifier is a reserved word.\n");

    else if(error_num == 17)
        printf("The code is too long to run.\n");

    else if(error_num == 18)
        printf("Constant values cannot be assigned values.\n");

    else if(error_num == 19)
        printf("Identifier must follow a read or write symbol.\n");

    else if(error_num == 21)
        printf("Superfluous semicolon found.\n");

    else if(error_num == 22)
        printf("%s is already declared.\n", name);

    else if(error_num == 23)
        printf("The preceding factor cannot begin with this symbol.\n");

    else if(error_num == 24)
        printf("An expression cannot begin with this symbol.\n");

    else if(error_num == 25)
        printf("Incorrect symbol following statement.\n");

    else if(error_num == 26)
        printf("Incorrect symbol after statement part in block.\n");

    else if(error_num == 27)
        printf("Too many lexicographical levels.\n");

    total_error++;
}

//iterates through the input file and counts the number of lines
int countLine(int lineCount, char input[300][MAX_CODE_LENGTH])
{
    // while file is not empty
    while(!feof(fp)){
        // store an entire line of input and print it out
        fgets(input[lineCount], MAX_CODE_LENGTH, fp);
        lineCount++;
    }

    return lineCount;
}

void print_Assembly()
{
    printf("OP\tR\tL\tM\n");
    for(int i = 0; i < code.IC; i++)
        printf("%d\t%d\t%d\t%d\n", code.instructions[i].OP, code.instructions[i].R, code.instructions[i].L, code.instructions[i].M);
}
void printTokens(int tokenCount)
{
    // print out output
    printf("Lexeme Table:\n");
    printf("lexeme\ttoken type\n");
    for(int i=0; i<tokenCount; i++){
        printf("%s\t%d\n", tokens[i], array[i]);
    }

    printf("\nLexeme List:\n");
    for(int i=0; i<tokenCount; i++){
        printf("%d ", array[i]);
        if(array[i] == identsym || array[i] == numbersym)
            printf("%s ", tokens[i]);
    }
}

void scanner(int print_tok)
{
    // array that holds the source program
    char input[300][MAX_CODE_LENGTH];

    int lineCount = 0;
    lineCount = countLine(lineCount, input);

    fclose(fp);

    int row = 0;
    int col = 0;
    char firstChar;
    char testString[IDENT_LIMIT + 1];
    int i = 0; // to increment through testString
    int otherChar = 1; // a boolean used to detect special characters

    // for each token
    while(row < lineCount){
        // read in the very first character
        firstChar = input[row][col++];

        // new line
        if(firstChar == 10 || firstChar == 13 || firstChar == 0){
            row++;
            col = 0;
            continue;
        }
        // tab
        if(firstChar == 9){
            continue;
        }
        // space
        if(firstChar == 32){
            continue;
        }

        // add the first character to testString
        testString[i++] = firstChar;

        // if the first character is a letter of the alphabet
        if(isalpha((int) firstChar)){
            otherChar = 0;

            // for each character after the first character
            while(1){
                // add the next character to the testString
                testString[i++] = input[row][col++];

                // if the next character is not a letter or digit, remove it from the testString
                // add the current token, and go to next iteration of the while loop
                if(!isalnum(testString[i-1])){
                    testString[i-1] = '\0';
                    strcpy(tokens[tokenCount++], testString);
                    col--;
                    break;
                }

                // else continue adding letters to current the token
            }
        }

        // if the first character is a digit
        if(isdigit((int) firstChar)){
            otherChar = 0;

            // while digit token is not too long
            while(1){

                // get next character in input
                testString[i++] = input[row][col++];

                // if the next character is not a digit, end of token
                if(!isdigit((int)testString[i-1])){
                    // error: we have a variable that does not start with a letter
                    if(isalpha((int)testString[i-1])){
                        error(20, "");
                    }
                    testString[i-1] = '\0';
                    strcpy(tokens[tokenCount++], testString);
                    col--;
                    break;
                }
            }
        }

        // test for the becomes symbol
        if(firstChar == ':'){
            otherChar = 0;

            // get next character
            testString[i++] = input[row][col++];

            // this token is :=, the becomes symbol
            if(testString[i-1] == '='){
                strcpy(tokens[tokenCount++], testString);
            }

            // this token is :, the colon
            else{
                testString[i-1] = '\0';
                strcpy(tokens[tokenCount++], testString);
                col--;
            }
        }

        // test for <> or <= or <
        if(firstChar == '<'){
            otherChar = 0;

            // get next character
            testString[i++] = input[row][col++];

            // this token is <>, not equal to
            if(testString[i-1] == '>'){
                strcpy(tokens[tokenCount++], testString);
            }

            // this token is <=, greater than or equal to
            else if(testString[i-1] == '='){
                strcpy(tokens[tokenCount++], testString);
            }

            // this token is <, less than
            else{
                testString[i-1] = '\0';
                strcpy(tokens[tokenCount++], testString);
                col--;
            }
        }

        // test for >= or >
        if(firstChar == '>'){
            otherChar = 0;

            // get next character
            testString[i++] = input[row][col++];

            // this token is >=, greater than or equal to
            if(testString[i-1] == '='){
                strcpy(tokens[tokenCount++], testString);
            }

            // this token is >, greater than
            else{
                testString[i-1] = '\0';
                strcpy(tokens[tokenCount++], testString);
                col--;
            }
        }

        // test for comments
        if(firstChar == '/'){
            otherChar = 0;

            // get next character
            testString[i++] = input[row][col++];
            if(testString[i-1] == '*'){

                // this is a comment, ignore all input until */
                while(!(input[row][col] == '*' && input[row][col+1] == '/')){
                    if(input[row][col] == 10){
                        row++;
                        col = 0;
                    }
                    else
                        col++;
                }

                for(i=0; i<12; i++){
                    testString[i] = '\0';
                }
                i = 0;
                continue;
            }

            else if(input[row][col-3] != '*'){
                testString[i-1] = '\0';
                strcpy(tokens[tokenCount++], testString);
                col--;
            }
        }

        if(firstChar == '.'){
            otherChar = 0;
            strcpy(tokens[tokenCount++], testString);
            break;
        }

        if(firstChar == ' '){
            otherChar = 0;
        }

        if(otherChar){
            strcpy(tokens[tokenCount++], testString);
        }

        int s;
        for(s=0; s<12; s++){
            testString[s] = '\0';
        }
        i = 0;
        otherChar = 1;
    }

    if(matchTokenTypes(tokens, tokenCount)){
        printf("A lexical error was detected.\n");
    }

    if(print_tok)
        printTokens(tokenCount);
}

int matchTokenTypes(char tokens[MAX_CODE_LENGTH][IDENT_LIMIT], int tokenCount){

    int i;
    char thisOne[IDENT_LIMIT+1];

    for(i=0; i<tokenCount; i++){
        strcpy(thisOne, tokens[i]);

        if(strcmp(thisOne, "null") == 0)
            array[i] = nulsym;
        else if(strcmp(thisOne, "+") == 0)
            array[i] = plussym;
        else if(strcmp(thisOne, "-") == 0)
            array[i] = minussym;
        else if(strcmp(thisOne, "*") == 0)
            array[i] = multsym;
        else if(strcmp(thisOne, "/") == 0)
            array[i] = slashsym;
        else if(strcmp(thisOne, "odd") == 0)
            array[i] = oddsym;
        else if(strcmp(thisOne, "=") == 0)
            array[i] = eqsym;
        else if(strcmp(thisOne, "<>") == 0)
            array[i] = neqsym;
        else if(strcmp(thisOne, "<") == 0)
            array[i] = lessym;
        else if(strcmp(thisOne, "<=") == 0)
            array[i] = leqsym;
        else if(strcmp(thisOne, ">") == 0)
            array[i] = gtrsym;
        else if(strcmp(thisOne, ">=") == 0)
            array[i] = geqsym;
        else if(strcmp(thisOne, "(") == 0)
            array[i] = lparentsym;
        else if(strcmp(thisOne, ")") == 0)
            array[i] = rparentsym;
        else if(strcmp(thisOne, ",") == 0)
            array[i] = commasym;
        else if(strcmp(thisOne, ";") == 0)
            array[i] = semicolonsym;
        else if(strcmp(thisOne, ".") == 0)
            array[i] = periodsym;
        else if(strcmp(thisOne, ":=") == 0)
            array[i] = becomessym;
        else if(strcmp(thisOne, "begin") == 0)
            array[i] = beginsym;
        else if(strcmp(thisOne, "end") == 0)
            array[i] = endsym;
        else if(strcmp(thisOne, "if") == 0)
            array[i] = ifsym;
        else if(strcmp(thisOne, "then") == 0)
            array[i] = thensym;
        else if(strcmp(thisOne, "while") == 0)
            array[i] = whilesym;
        else if(strcmp(thisOne, "do") == 0)
            array[i] = dosym;
        else if(strcmp(thisOne, "call") == 0)
            array[i] = callsym;
        else if(strcmp(thisOne, "const") == 0)
            array[i] = constsym;
        else if(strcmp(thisOne, "var") == 0)
            array[i] = varsym;
        else if(strcmp(thisOne, "procedure") == 0)
            array[i] = procsym;
        else if(strcmp(thisOne, "write") == 0)
            array[i] = writesym;
        else if(strcmp(thisOne, "read") == 0)
            array[i] = readsym;
        else if(strcmp(thisOne, "else") == 0)
            array[i] = elsesym;
        else if(strcmp(thisOne, "int") == 0)
            array[i] = intsym;
        else{
            if(isdigit(thisOne[0])){
                if(strlen(thisOne) > DIGIT_LIMIT)
                    return 1; // error: number too long
                else
                    array[i] = numbersym;
            }
            else if(isalpha(thisOne[0])){
                if(strlen(thisOne) > IDENT_LIMIT)
                    return 1; // error: identifier too long
                else
                    array[i] = identsym;
            }
            else{
                return 1; // error: invalid symbol
            }
        }

    }

    return 0;
}

void factor(token_type *fsys, int size)
{
    test(facebegsys, fsys, 3, size, 24, "");

    while(isElement(cur_token.token, facebegsys, 3))
    {

        if(cur_token.token == identsym)
        {
            if(lookUpSym(cur_token.ident, 0)->sym.kind == 1)
            {
                emit(1, reg_count, 0, lookUpSym(cur_token.ident, 0)->sym.val);
                changeRegCount(1);
            }

            else if(lookUpSym(cur_token.ident, 0)->sym.kind == 2)
            {
                if(start_main)
                    emit(3, reg_count, 0, lookUpSym(cur_token.ident, 0)->sym.addr+1);
                else
                    emit(3, reg_count, level - lookUpSym(cur_token.ident, 0)->sym.level, lookUpSym(cur_token.ident, 0)->sym.addr+1);

                changeRegCount(1);
            }

            getNextToken();
        }

        else if(cur_token.token == numbersym)
        {
            emit(1, reg_count, 0, cur_token.number);

            changeRegCount(1);

            getNextToken();
        }


        else if(cur_token.token == lparentsym)
        {
            getNextToken();

            token_type temp[] = {rparentsym};

            token_type *combined = unionTok(fsys, temp, size, 1);

            expression(combined, size+1);

            free(combined);

            if(cur_token.token != rparentsym)
                error(13, "");

            getNextToken();
        }

        else
            error(14, "");

        token_type temp[] = {lparentsym};

        test(fsys, temp, size, 1, 23, "");
    }
}

void term(token_type *fsys, int size)
{
    token_type temp[] = {multsym, slashsym};

    token_type *combined = unionTok(fsys, temp, size, 2);

    factor(combined, size+2);

    while(cur_token.token == multsym || cur_token.token == slashsym)
    {
        token_type temp = cur_token.token;

        getNextToken();

        factor(combined, size+2);

        changeRegCount(0);

        if(temp == multsym)
            emit(15, reg_count-1, reg_count-1, reg_count);

        else
            emit(16, reg_count-1, reg_count-1, reg_count);
    }

    free(combined);

}
void expression(token_type *fsys, int size)
{
    int isNeg = 0;

    token_type temp;

    token_type temp2[] = {plussym, minussym};

    token_type *combined = unionTok(fsys, temp2, size, 2);

    if(cur_token.token == plussym || cur_token.token == minussym)
    {
        temp = cur_token.token;

        isNeg = 1;

        getNextToken();
    }

    term(combined, size+2);

    if(isNeg)
    {
        if(temp == minussym)
            emit(12, reg_count-1, reg_count-1, 0);

        isNeg = 0;
    }

    while(cur_token.token == plussym || cur_token.token == minussym)
    {
        token_type temp = cur_token.token;

        getNextToken();

        term(combined, size+2);

        changeRegCount(0);

        if(temp == plussym)
            emit(13, reg_count-1, reg_count-1, reg_count);
        else
            emit(14, reg_count-1, reg_count-1, reg_count);
    }

    free(combined);
}

int checkRelation()
{
    if(cur_token.token == eqsym)
        return 1;
    else if(cur_token.token == neqsym)
        return 1;
    else if(cur_token.token == lessym)
        return 1;
    else if(cur_token.token == leqsym)
        return 1;
    else if(cur_token.token == gtrsym)
        return 1;
    else if(cur_token.token == geqsym)
        return 1;
    else
        return 0;
}

void condition(token_type *fsys, int size)
{
    if(cur_token.token == oddsym)
    {
        getNextToken();

        expression(fsys, size);

        changeRegCount(0);

        emit(17, reg_count, 0, 0);
    }

    else
    {
        token_type temp1[] = {eqsym, neqsym, lessym, gtrsym, leqsym, geqsym};

        token_type *combined = unionTok(fsys, temp1, size, 6);

        expression(combined, size+6);

        free(combined);

        if(!checkRelation())
            error(12, "");

        token_type temp = cur_token.token;

        getNextToken();

        expression(fsys, size);

        changeRegCount(0);

        if(temp == eqsym)
            emit(19, reg_count-1, reg_count-1, reg_count);
        else if(temp == neqsym)
            emit(20, reg_count-1, reg_count-1, reg_count);
        else if(temp == lessym)
            emit(21, reg_count-1, reg_count-1, reg_count);
        else if(temp == leqsym)
            emit(22, reg_count-1, reg_count-1, reg_count);
        else if(temp == gtrsym)
            emit(23, reg_count-1, reg_count-1, reg_count);
        else
            emit(24, reg_count-1, reg_count-1, reg_count);
    }
}

void statement(token_type *fsys, int size)
{
    if(cur_token.token == identsym)
    {
        char temp[IDENT_LIMIT];

        strcpy(temp, cur_token.ident);

        getNextToken();

        if(lookUpSym(temp, 0)->sym.kind == 1)
            error(18, "");

        if(cur_token.token != becomessym)
            error(7, "");

        getNextToken();

        expression(fsys, size);

        changeRegCount(0);

        if(lex_level - lookUpSym(temp, 0)->sym.level < 0)
            emit(4, reg_count, 0, lookUpSym(temp, 0)->sym.addr+1);
        else
            emit(4, reg_count, level - lookUpSym(temp, 0)->sym.level, lookUpSym(temp, 0)->sym.addr+1);
    }

    else if(cur_token.token == callsym)
    {
        getNextToken();

        if(cur_token.token != identsym)
            error(8, "");

        searchReserves();

        emit(5, 0, level, lookUpSym(cur_token.ident, 0)->sym.addr);

        getNextToken();
    }

    else if(cur_token.token == beginsym)
    {
        getNextToken();

        token_type temp[] = {semicolonsym, endsym};

        token_type *combined = unionTok(fsys, temp, size, 2);

        statement(combined, size+2);

        while(cur_token.token == semicolonsym)
        {
            getNextToken();

            if(cur_token.token != beginsym && cur_token.token != readsym && cur_token.token != callsym && cur_token.token != ifsym && cur_token.token != identsym)
               if(cur_token.token != whilesym && cur_token.token != writesym && cur_token.token != semicolonsym)
                   error(21, "");

            statement(combined, size+2);
        }

        if(cur_token.token != endsym)
            error(9, "");

        getNextToken();

        free(combined);
    }

    else if(cur_token.token == ifsym)
    {
        getNextToken();

        token_type temp[] = {thensym, dosym};

        token_type *combined = unionTok(fsys, temp, size, 2);

        condition(combined, size+2);

        int savedIC = code.IC;

        emit(8, reg_count-1, 0, -1);

        if(cur_token.token != thensym)
            error(10, "");

        getNextToken();

        statement(fsys, size);

        if(array[cur_token.token_cnt] == elsesym)
        {
            getNextToken();

            int savedIC2 = code.IC;

            emit(7, 0, 0, -1);

            code.instructions[savedIC].M = code.IC;

            getNextToken();

            statement(fsys, size);

            code.instructions[savedIC2].M = code.IC;
        }

        else
            code.instructions[savedIC].M = code.IC;
    }

    else if(cur_token.token == whilesym)
    {
        token_type temp[] = {dosym};

        token_type *combined = unionTok(fsys, temp, size, 1);

        getNextToken();

        int savedIC1 = code.IC;

        condition(combined, size+1);

        free(combined);

        int savedIC2 = code.IC;

        emit(8, reg_count-1, 0, -1);

        if(cur_token.token != dosym)
            error(11, "");

        getNextToken();

        statement(fsys, size);

        emit(7, 0, 0, savedIC1);

        code.instructions[savedIC2].M = code.IC;
    }

    else if(cur_token.token == writesym || cur_token.token == readsym)
    {
        token_type temp = cur_token.token;

        getNextToken();

        if(cur_token.token != identsym)
            error(19, "");

        if(temp == writesym)
        {
            if(lookUpSym(cur_token.ident, 0)->sym.kind == 1)
                emit(1, reg_count, 0, lookUpSym(cur_token.ident, 0)->sym.val);
            else
            {
                if(lex_level - lookUpSym(cur_token.ident, 0)->sym.level < 0)
                    emit(3, reg_count, 0, lookUpSym(cur_token.ident, 0)->sym.addr+1);
                else
                    emit(3, reg_count, level - lookUpSym(cur_token.ident, 0)->sym.level, lookUpSym(cur_token.ident, 0)->sym.addr+1);
            }

            emit(9, reg_count, 0, 1);
        }

        else
        {
            if(lookUpSym(cur_token.ident, 0)->sym.kind == 1)
                error(18 ,"");

            emit(10, reg_count, 0, 2);

            changeRegCount(1);

            if(start_main)
                emit(4, reg_count-1, 0, lookUpSym(cur_token.ident, 0)->sym.addr+1);
            else
                emit(4, reg_count-1, level - lookUpSym(cur_token.ident, 0)->sym.level, lookUpSym(cur_token.ident, 0)->sym.addr+1);

            changeRegCount(0);
        }

        getNextToken();
    }

    token_type temp[] = {};

    test(fsys, temp, size, 0, 25, "");
}

void constantDeclaration()
{
    do
    {
        getNextToken();

        if(cur_token.token != identsym)
            error(3, "");

        getNextToken();

        if(cur_token.token != eqsym)
            error(4, "");

        getNextToken();

        if(cur_token.token != numbersym)
            error(5, "");

        searchReserves(cur_token.ident);

        insertHash(1, cur_token.ident, cur_token.number, 0, 0);

        getNextToken();

    } while(cur_token.token == commasym);

    if(cur_token.token != semicolonsym)
        error(6, "");

    getNextToken();

}

int varDeclaration()
{
    int val = 0;
    do
    {
        getNextToken();

        if(cur_token.token != identsym)
            error(3, "");

        searchReserves(cur_token.ident);

        getNextToken();

        insertHash(2, cur_token.ident, 0, lex_level, addr);

        val++;

        addr++;

    } while(cur_token.token == commasym);

    if(cur_token.token != semicolonsym)
        error(6, "");

    getNextToken();

    return val;
}

int procDeclaration(int jump_address, token_type *fsys, int size)
{
    while(cur_token.token == procsym)
    {
        getNextToken();

        if(cur_token.token != identsym)
            error(3, "");

        getNextToken();

        searchReserves();

        insertHash(3, cur_token.ident, 0, lex_level, jump_address+1);

        if(cur_token.token != semicolonsym)
            error(6, "");

        getNextToken();

        level++;

        lex_level++;

        token_type temp[] = {semicolonsym};

        fsys = unionTok(fsys, temp, size, 1);

        block(fsys, size+1);

        lex_level--;

        emit(2, 0, 0, 0);

        if(cur_token.token != semicolonsym)
            error(6, "");

        getNextToken();
    }
}

void block(token_type *fsys, int size)
{
    addr = 3;
    int var_total = 4;
    int proc_x;

    if(lex_level > MAX_LEVEL)
        error(27, "");

    int jump_to = code.IC;

    emit(7, 0, 0, 0);

    if(cur_token.token == constsym)
        constantDeclaration();

    if(cur_token.token == varsym || cur_token.token == intsym)
        var_total += varDeclaration();

    if(cur_token.token == procsym)
        procDeclaration(jump_to, fsys, size);

    code.instructions[jump_to].M = code.IC;

    if(jump_to == 0)
        start_main = 1;

    emit(6, 0, 0, var_total);

    token_type temp[] = {semicolonsym, endsym};

    token_type *combined = unionTok(fsys, temp, size, 2);

    statement(combined, size + 2);

    token_type temp2[] = {};

    test(combined, temp2, size, 0, 26, "");

    free(combined);
}

void program(int print_assembly){

    getNextToken();

    token_type temp[1] = {periodsym};

    token_type *combined = unionTok(temp, unionTok(declbegsys, statbegsys, 3, 4), 1, 7);

    block(combined, 7);

    free(combined);

    if(cur_token.token != periodsym)
        error(2, "");

    emit(11, 0, 0, 3);

    if(print_assembly)
        print_Assembly();
}

void initializeToken()
{
    cur_token.token_cnt = 0;
    cur_token.number = 0;
}

int main(int argc, char **argv)
{
    // read in source program
    fp = fopen("input1.txt", "r");
    int print_lexemes = 0, print_assembly = 0, print_stack = 0;

    //set the instruction count to 0 and procedure count to 0
    code.IC = 0;
    saved.count = 0;

    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-l") == 0)
            print_lexemes = 1;
        else if(strcmp(argv[i], "-a") == 0)
            print_assembly = 1;
        else if(strcmp(argv[i], "-v") == 0)
            print_stack = 1;
        else
            printf("%s was not recognized as a command.\n", argv[i]);
    }

    if(fp)
        scanner(print_lexemes);

    else
        error(-1, "");

    initializeToken();

    program(print_assembly);

    virtualMachine(print_stack);

    printf("\n%d error(s) where detected.\n", total_error);
}
