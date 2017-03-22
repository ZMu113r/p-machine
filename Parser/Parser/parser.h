#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//this will be used to store tokens that meet grammar requirements
typedef struct
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
    int reg;
    int L;
    int M;
} instruction;

//gonna define all procedures here to allow the reader to wrap their mind around what I'm doing
//also avoid implicit declaration:)
void error(int errNum);
void convertToAssembly(int OP, int reg, int L, int M);
void constant_Declaration();
struct Node* createNode(Symbol s);
Symbol *createSymbol(int kind, char *name, int val, int level, int addr);
struct Node *insertNode(Symbol sym, struct Node *head);
int compareSymbols(Symbol s1, Symbol s2);
int destroyNode(Symbol s, struct Node *head);
void getNextToken();
unsigned int hashValue(char *str, unsigned int len);
int lookUp(int kind, char *name, int value, int level, int addr);
void insertHash(int kind, char *name, int value, int level, int addr);
void factor();
int term();
void expression();
void condition();
void statement();
void var_Declaration();
void proc_Declaration();
void block();
void program();
void readFile();



//needed global variables for all functions
char token [11];
const unsigned int MAX_SYMBOLS = 10000;
struct hash symbol_table[10000];
struct Node insertingNode;
int addr;
int test;
temp temp_Val;
FILE *file;
instruction instructions[500];
int instructionCount = 0;
