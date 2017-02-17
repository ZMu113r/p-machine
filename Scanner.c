// Zach Muller
// COP 3402 - Spring 2017
//
// Scanner - this program is a virtual machine of the scanner 
// component of a compiler for a basic language
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_DIGITS_OF_NUM 5
#define MAX_CHARS_OF_IDENTIFIERS 11
#define norw 15    // number of reserved words  
#define imax 32767 // maximum integer value  
#define cmax 11    // maximum number of chars for idents  
#define nestmax 5  // maximum depth of block nesting                 
#define strmax 256 // maximum length of strings 

typedef enum 
{
	NULSYM = 1, IDENTSYM = 2, NUMBERSYM = 3, PLUSSYM = 4, MINUSSYM = 5, 
	MULTSYM = 6, SLASHSYM = 7, ODDSYM = 8, EQLSYM = 9, NEQSYM = 10, 
	LESSYM = 11, LEQSYM = 12, GTRSYM = 13, GEQSYM = 14, LPARENTSYM = 15, 
	RPARENTSYM = 16, COMMASYM = 17, SEMICOLONSYM = 18, PERIODSYM = 19, 
	BECOMESSYM = 20, BEGINSYM = 21, ENDSYM = 22, IFSYM = 23, THENSYM = 24, 
	WHILESYM = 25, DOSYM = 26, CALLSYM = 27, CONSTSYM = 28, VARSYM = 29, 
	PROCSYM = 30, WRITESYM = 31, READSYM = 32, ELSESYM = 33 
} token_type;

// struct for tokens
typedef struct tokens
{
	char *lexemes;
	int token_type;
} token;


// GLOBALS
// Symbol table
char SPECIAL_SYMBOLS_TABLE [12] = {'+', '-', '*', '/', '(', ')', '=', ',', '.', '<', '>', ';'};
// Reserverd words table
char RESERVED_WORDS [norw][MAX_CHARS_OF_IDENTIFIERS] = {"begin", "call", "const", "do", "else", "end", "if", "odd", "procedure", "null", "read", "then", "var", "while", "write"};


// Function to pair each lexeme with a token type
void assignTokenType(token *myToken)
{
	// If the lexeme is a null symbol
	if(strcmp(myToken->lexemes, "null") == 0)
		myToken->token_type = 1;

	// If the lexeme is a number
	if(isdigit(myToken->lexemes[0]) != 0)
		myToken->token_type = 3;

	// If the lexeme is a plus symbol
	if(strcmp(myToken->lexemes, "+") == 0)
		myToken->token_type = 4;

	// If the lexeme is a minus symbol
	if(strcmp(myToken->lexemes, "-") == 0)
		myToken->token_type = 5;

	// If the lexeme is a multiplication symbol
	if(strcmp(myToken->lexemes, "*") == 0)
		myToken->token_type = 6;

	// If the lexeme is a slash symbol
	if(strcmp(myToken->lexemes, "/") == 0)
		myToken->token_type = 7;

	// If the lexeme is an odd symbol
	if(strcmp(myToken->lexemes, "odd") == 0)
		myToken->token_type = 8;

	// If the lexeme is an equal symbol
	if(strcmp(myToken->lexemes, "=") == 0)
		myToken->token_type = 9;

	// If the lexeme is a not equal to symbol
	if(strcmp(myToken->lexemes, "!=") == 0)
		myToken->token_type = 10;

	// If the lexeme is a less than symbol
	if(strcmp(myToken->lexemes, "<") == 0)
		myToken->token_type = 11;

	// If the lexeme is a less than or equal to symbol
	if(strcmp(myToken->lexemes, "<=") == 0)
		myToken->token_type = 12;

	// If the lexeme is a greater than symbol
	if(strcmp(myToken->lexemes, ">") == 0)
		myToken->token_type = 13;

	// If the lexeme is a greater than or equal to symbol
	if(strcmp(myToken->lexemes, ">=") == 0)
		myToken->token_type = 14;

	// If the lexeme is a left parentheses symbol
	if(strcmp(myToken->lexemes, "(") == 0)
		myToken->token_type = 15;

	// If the lexeme is a right parentheses symbol
	if(strcmp(myToken->lexemes, ")") == 0)
		myToken->token_type = 16;

	// If the lexeme is a comma symbol
	if(strcmp(myToken->lexemes, ",") == 0)
		myToken->token_type = 17;

	// If the lexeme is a semicolon symbol
	if(strcmp(myToken->lexemes, ";") == 0)
		myToken->token_type = 18;

	// If the lexeme is a period symbol
	if(strcmp(myToken->lexemes, ".") == 0)
		myToken->token_type = 19;

	// If the lexeme is a becomes symbol
	if(strcmp(myToken->lexemes, ":=") == 0)
		myToken->token_type = 20;

	// If the lexeme is a begin symbol
	if(strcmp(myToken->lexemes, "begin") == 0)
		myToken->token_type = 21;

	// If the lexeme is an end symbol
	if(strcmp(myToken->lexemes, "end") == 0)
		myToken->token_type = 22;

	// If the lexeme is an if symbol
	if(strcmp(myToken->lexemes, "if") == 0)
		myToken->token_type = 23;

	// If the lexeme is a then symbol
	if(strcmp(myToken->lexemes, "then") == 0)
		myToken->token_type = 24;

	// If the lexeme is a while symbol
	if(strcmp(myToken->lexemes, "while") == 0)
		myToken->token_type = 25;

	// If the lexeme is a do symbol
	if(strcmp(myToken->lexemes, "do") == 0)
		myToken->token_type = 26;

	// If the lexeme is a call symbol
	if(strcmp(myToken->lexemes, "call") == 0)
		myToken->token_type = 27;

	// If the lexeme is a constant symbol
	if(strcmp(myToken->lexemes, "const") == 0)
		myToken->token_type = 28;

	// If the lexeme is a var keyword
	if(strcmp(myToken->lexemes, "var") == 0)
		myToken->token_type = 29;

	// If the lexeme is a procedure symbol
	if(strcmp(myToken->lexemes, "procedure") == 0)
		myToken->token_type = 30;

	// If the lexeme is a write symbol
	if(strcmp(myToken->lexemes, "write") == 0)
		myToken->token_type = 31;

	// If the lexeme is a read symbol
	if(strcmp(myToken->lexemes, "read") == 0)
		myToken->token_type = 32;

	// If the lexeme is an else symbol
	if(strcmp(myToken->lexemes, "else") == 0)
		myToken->token_type = 33;
	
	// else its an identifier
	if(myToken->token_type == 0)
		myToken->token_type = 2;
}

// Function to pair lexemes with token type number
void createTokens(token tokens [1000], char lexemes[1000][MAX_CHARS_OF_IDENTIFIERS], int numTokens)
{
	for(int i = 0; i < numTokens; i++)
	{	
		token *myToken = (token *)malloc(sizeof(token));
		// If the lexem isn't a space, tab, or newline
		if((strcmp(lexemes[i], " ") >= -32) && (strcmp(lexemes[i], " ") != -22))
		{
			myToken->lexemes = (char *)malloc(sizeof(char) * 10);
			myToken->token_type = 0;
			strcpy(myToken->lexemes, lexemes[i]);

			// Function call to assign lexemes token types
			assignTokenType(myToken);

			// Place token in array of tokens
			tokens[i] = *myToken;
		}
	}
}	

void printLexemeTable(token tokens [1000], int numTokens)
{	
	// Headers
	printf("Lexeme Table:\n");
	printf("%-10s %s\n", "lexeme", "token type");

	// Print table
	for(int i = 0; i < numTokens; i++)
	{	
		// change this to if(tokens[i].lexemes[0] - '0' != -48) for linux systems
		// change this to if(strcmp(tokens[i].lexemes, " ") != -32) for iOS systems
		if(tokens[i].lexemes[0] - '0' != -48)
			printf("%-10s %d\n", tokens[i].lexemes, tokens[i].token_type);
	}
	printf("\n");

	// Print list
	printf("Lexeme List:\n");
	for(int j = 0; j < numTokens; j++)
	{
		printf("%d", tokens[j].token_type);
		if(tokens[j].token_type == 2)
			printf("%s", tokens[j].lexemes);
		printf(" ");
	}
	printf("\n");
}




// Read in program
// Identify errors
// Produce lexeme table and a list of lexemes as output
// Generate symbol table 
int main(int argc, char *argv[])
{	
	char *filename = argv[1];
	FILE *file;
	char c;
	int count = 0;
	int i = 0;
	int j = 0;
	int numTokens = 0;

	// Create char array to build lexemes
	// loop through and initialize it to null
	char word [MAX_CHARS_OF_IDENTIFIERS];
	int k;
	for(k = 0; k < MAX_CHARS_OF_IDENTIFIERS; k++)
		word[k] = '\0';

	file = fopen(filename, "r");

	// Give me a file you fool!
	if(file == NULL)
	{
		return 0;
	}

	// Get number of lexemes we'll have to make
	while((c = fgetc(file)) != EOF)
	{	
		count++;
	}
	rewind(file);

	// Lexeme table
	char lexemes [count][MAX_CHARS_OF_IDENTIFIERS];


	while((c = fgetc(file)) != EOF)
	{	
		// Ignore newline characters
		if(c - '0' != -38)
		{
			// If the char IS a letter or digit
			if(isalpha(c) != 0 || isdigit(c) != 0)
			{
				word[j] = c;
				j++;
			}

			// If char is NOT a letter or digit and not a space
			if((isalpha(c) == 0) && (isdigit(c) == 0) && (c != ' '))
			{	
				// Do these things for all special symbols but comment blocks
				if(((word[0] != '/') && (word[1] != '*')) || ((word[0] != '*') && (word[1] != '/')))
				{	
					// If we're not still dealing with an assignment operator
					if(((word[0] != ':') || (word[0] != '>') || (word[0] != '<') || (word[0] != '!')) && c != '=')
					{	
						strcpy(lexemes[i], word);
						numTokens++;

						// reset word to null
						for(k = 0; k < MAX_CHARS_OF_IDENTIFIERS; k++)
							word[k] = '\0';

						i++;
						j = 0;

						word[j] = c;

						if(word[j] == '.')
						{
							strcpy(lexemes[i], word);
							numTokens++;

							// reset word to null
							for(k = 0; k < MAX_CHARS_OF_IDENTIFIERS; k++)
								word[k] = '\0';
						}

					}

					if((c == ':') || (c == '>') || (c == '<') || (c == '!'))
					{	
						word[0] = c;
						j++;
					}
				
					if(((word[0] == ':') && (c == '=')) || ((word[0] == '>') && (c == '=')) || ((word[0] == '<') && (c == '=')) || ((word[0] == '!') && (c == '=')))
					{	
						word[1] = c;
						strcpy(lexemes[i], word);
						numTokens++;

						// reset word to null
						for(k = 0; k < MAX_CHARS_OF_IDENTIFIERS; k++)
							word[k] = '\0';

						i++;
						j = 0;
					}
				}
			}

			// If char is a space or control character
			if(c == ' ' || (c - '0' == -39))
			{	
				if(((c == ' ') || (c - '0' == -39)) && (word[0] != '\0'))
				{	
					if(word[0] - '0' != -39)
					{
						strcpy(lexemes[i], word);
						numTokens++;

						// reset word to null
						for(k = 0; k < MAX_CHARS_OF_IDENTIFIERS; k++)
							word[k] = '\0';

						i++;
						j = 0;
					}
				}
			}
		}
	}

	token tokens [numTokens];
	// Function call to create tokens with lexeme table
	createTokens(tokens, lexemes, numTokens);

	// Display lexeme table
	printLexemeTable(tokens, numTokens);

	return 0;
}