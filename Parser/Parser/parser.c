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

//gonna define all procedures here to allow the reader to wrap their mind around what I'm doing
//also avoid implicit declaration:)
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
void constant_Declaration();
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
    }

    return temp;
}

//inserts Node into the linked list
struct Node *insertNode(Symbol sym, struct Node *head)
{
    //use temp
    struct Node *temp = createNode(sym);

    if(!head)
        return temp;



    temp->next = head;

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

    fscanf(file, "%s", &token);
    printf("Token = %s\n", token);

    //scans next token as the identifier name
    if(strcmp(token, "identsym") == 0)
    {
        fscanf(file, "%s", &temp_Val.ident);
        printf("WE ARE HERE(1)\n");
    }


    //scans the number into the identifier value
    else if(strcmp(token, "numbersym") == 0)
    {

        fscanf(file, "%d", &temp_Val.val);

        printf("WE ARE HERE,IDENT = %s\n", temp_Val.ident);
        printf("DONE (2): %d\n", temp_Val.val);
    }



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
int lookUp(int kind, char *name, int value, int level, int addr)
{

    Symbol sym = *createSymbol(kind, name, value, level, addr);
    //find the hash value
    int hash_val = hashValue(sym.name, strlen(sym.name));

    //make a temp
    struct Node *temp = symbol_table[hash_val].head;

    //if that location is null, then the node isn't there
    if(temp == NULL)
        return -1;

    //check through the list at that position for the node
    while(compareSymbols(temp->sym, sym) != 0)
    {
        temp = temp->next;

        //if we get to the end, then it is not there
        if(temp == NULL)
            return -1;


    }

    //otherwise we found it
    return 0;
}

void insertHash(int kind, char *name, int value, int level, int addr)
{
    Symbol sym = *createSymbol(kind, name, value, level, addr);

    int hash = hashValue(sym.name, strlen((sym.name)));

    symbol_table[hash].head = insertNode(sym, symbol_table[hash].head);

    printf("Insert Complete.\n");
}

void factor()
{
    printf("Current token(5): %s\n", token);

    //check for identifier
    if(strcmp(token, "identsym") == 0)
        getNextToken();


    //check for number
    else if(strcmp(token, "numbersym") == 0)
        getNextToken();

    //check for '('
    else if(strcmp(token, "(") != 0)
    {
        getNextToken();

        //call expression for the parenthesis
        expression();

        //next token has to be the closing parenthesis
        if(strcmp(token, ")") != 0)
        {
            printf("Error 17: Missing Closing Parenthesis.\n");
            exit(17);
        }

        getNextToken();
    }
}

int term()
{
    factor();

    //keep looping when multiplying or dividing
    while(strcmp(token, "multsym") == 0 || strcmp(token, "slashsym") == 0)
    {
        getNextToken();

        //continually call factor
        factor();
    }
}

void expression()
{
    //call the term for the expression
    term();

    //if current term is plus or minus
    if(strcmp(token, "plussym") == 0 || strcmp(token, "minussym") == 0)
    {
        //keep looping when adding or subtracting
        while(strcmp(token, "plussym") == 0 || strcmp(token, "minussym") == 0)
        {
            getNextToken();

            //call term for all
            term();
        }
    }
}

void condition()
{
    //current token is the odd symbol
    if(strcmp(token, "oddsym") == 0)
    {
        getNextToken();

        //call the expression for the odd symb
        expression();
    }

    else
    {
        //call expression
        expression();

        //figure it out sooner or later
        //if(strcmp(token, "RELATION") != 0)
        //{
            //printf("Error 16: Relation Is Missing.\n");
            //exit(16);
        //}

        getNextToken();

        //call expression again after relation
        expression();
    }
}

void statement()
{
    printf("Yas.... %s\n", token);
    //check to see if the current token is the identifier symbol
    if(strcmp(token, "identsym") == 0)
    {
        getNextToken();

        printf("Current Symbol(3): %s\n", token);

        int answer = lookUp(2, temp_Val.ident, 0, 0, 0);

        if(answer < 0)
        {
            printf("Error 17: %s is not declared.\n", token);
            exit(17);
        }

        printf("Current Symbol(4): %s\n", token);
        //next symbol the must be the becomes, otherwise error
        if(strcmp(token, "becomessym") != 0)
        {
            printf("Error 11: Becomes Symbol Is Missing.\n");
            exit(11);
        }

        char saved_ident[11];

        strcpy(saved_ident, temp_Val.ident);

        getNextToken();


        expression();


    }

    //check current token to see if it is call symbol
    else if(strcmp(token, "callsym") == 0)
    {
        getNextToken();

        //next symbol must be the identifier symbol
        if(strcmp(token, "identsym") != 0)
        {
            printf("Error 12: Identifier Missing After Call.\n");
            exit(12);
        }

        getNextToken();
    }

    //check current token for the begins symbol
    else if(strcmp(token, "beginsym") == 0)
    {
        getNextToken();

        //begins symbol initiates another statement
        statement();

        //continue to run statements until we hit something but a semicolon
        while(strcmp(token, "semicolonsym") == 0)
        {
            getNextToken();

            statement();
        }

        printf("Current Symbol(2): %s\n", token);
        //ending symbol for after a begin must be end, else error
        if(strcmp(token, "endsym") != 0)
        {
            printf("Error 13: End Symbol Missing From Statement.\n");
            exit(13);
        }

        getNextToken();
    }

    //current token is if symbol
    else if(strcmp(token, "ifsym") == 0)
    {
        getNextToken();

        //call the condition for the if statement
        condition();

        //next symbol following the if is then, otherwise error
        if(strcmp(token, "thensym") != 0)
        {
            printf("Error 14: Missing Preceding then After If.\n");
            exit(14);
        }

        getNextToken();

        //call statement for the if statement
        statement();
    }

    //current token is the while symbol
    else if(strcmp(token, "whilesym") == 0)
    {
        getNextToken();

        //call condition for the while symbol
        condition();

        //must be followed by do symbol, else error
        if(strcmp(token, "dosym") != 0)
        {
            printf("Error 15: Missing do After while.\n");
            exit(15);
        }

        getNextToken();

        statement();
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
        if(strcmp(token, "identsym") != 0)
        {
            printf("Error 3: Identifier Missing.\n");
            exit(3);
        }

        //identifier name
        getNextToken();

        //check token for equal sign
        if(strcmp(token, "eqsym") != 0)
        {
            printf("Error 4: Equal Sign Missing\n");
            exit(4);
        }

        printf("tempVAL(1) = %s\n", temp_Val.ident);

        getNextToken();

        //we know the next token needs to be a number
        if(strcmp(token, "numbersym") != 0)
        {
            printf("Error 5: Number Missing For Constant Declaration.\n");
            exit(5);
        }

        printf("tempVAL(2) = %s\n", temp_Val.ident);

        //insert into hash table
        insertHash(1, temp_Val.ident, temp_Val.val, 0, 0);


        getNextToken();

        //check for comma
        if(strcmp(token, "commasym") == 0)
            condition = 1;
        else
            condition = 0;
    }

    //finally check for the semicolon and error otherwise
    if(strcmp(token, "semicolonsym") != 0)
    {
        printf("Error 6: Semicolon Missing For Constant Declaration.\n");
        exit(6);
    }

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
        printf("Current Token(1): %s\n", token);


        //check for identifier symbol
        if(strcmp(token, "identsym") != 0)
        {
            printf("Error 16: Identifier Missing for Integer Declaration.\n");
            exit(16);
        }

        //insert into hash table
        insertHash(2, temp_Val.ident, 0, 0, addr++);

        getNextToken();

        //if we have comma continue the loop
        if(strcmp(token, "commasym") != 0)
            condition = 0;

    }

    //check for semicolon
    if(strcmp(token, "semicolonsym") != 0)
    {
        printf("Error 7: Semicolon Missing For Integer Declaration.\n");
        exit(7);
    }

    getNextToken();
}

//if correct, defines the procedure and inserts into hash table
void proc_Declaration()
{
    getNextToken();

    //check for the identifier symbol, else error
    if(strcmp(token, "identsym") != 0)
    {
        printf("Error 8: Identifier Missing For Procedure Declaration.\n");
        exit(8);
    }

    //inserts into the hashtable
    insertHash(3, temp_Val.ident, 0, 0, addr++);

    getNextToken();

    //check for the semicolon after declaration, else error
    if(strcmp(token, "semicolonsym") != 0)
    {
        printf("Error 9: Semicolon Missing For Procedure Declaration.\n");
        exit(9);
    }

    getNextToken();

    //call block again for the procedure
    block();

    //check last token to be semicolon
    if(strcmp(token, "semicolonsym") != 0)
    {
        printf("Error 10: Semicolon Missing For Procedure Declaration.\n");
        exit(10);
    }

    getNextToken();
}

void block()
{


    //token is is constant
    if(strcmp(token, "constsym") == 0)
       constant_Declaration();

    //integer declaration portion
    if(strcmp(token, "varsym") == 0)
        var_Declaration();

    //procedure declaration
    while(strcmp(token, "procsym") == 0)
        proc_Declaration();

    statement();
}

void program()
{
    getNextToken();

    //call block
    block();

    printf("OKAY: %s\n", token);
    //make sure program ends with period
    if(strcmp(token, "periodsym") != 0)
    {
        printf("Error 2: Period Missing.\n");
        exit(2);
    }
}
//this read the file, general
void readFile()
{
    //open file to read
    file = fopen("test.txt", "r");

    //we found the file
    if(file)
    {
        //this will start the analysis
        program();

    }

    //we have not found the file, throw the first error
    else
    {
        printf("Error 1: File Not Found.\n");
        exit(1);
    }



}

int main()
{
    addr = 0;
    //start reading the tokens file
    readFile();

    if(lookUp(1, "x", 10, 0, 0) < 0)
        printf("FAILED.\n");
    else
        printf("SUCCESSFUL.\n");

    printf("DONE: %d\n", temp_Val.val);
}
