/*------------ DECLARATION SECTION ------------*/
%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <string.h>
	#include "../include/constants.h"

	#include "parser.h"

	#define IS_EOF(token) (token==T_EOF)

	// Colors
	#define RED     "\033[31m"      /* Red */
	#define RESET   "\033[0m"

	int print_token(int token);
%}

%option case-insensitive
%option yylineno
%option noyywrap

/* Declare Start Conditions */
%x COMMENT

WHITECHAR	[ \t\v\n]

ALPHA		[a-z]
NUM			[0-9]
ALNUM		[0-9a-z]
/* Here put all regexes */

/* Q: Can the name of an element be anything other than a number???? */
NAME		[{ALNUM}]+


/*------------ RULES SECTION ------------*/
%%

<COMMENT>{
\n		{ BEGIN(INITIAL); }
[^\n]+
}

<INITIAL>{
"*"		{ BEGIN(COMMENT); }

<<EOF>>			{ return print_token(T_EOF); }
{WHITECHAR}		{  /* Ignore white spaces */ }

"V"{NAME}			{ yylval.strval = (char*)malloc(strlen(yytext)); return print_token(T_V); }
"I"{NAME}			{ yylval.strval = (char*)malloc(strlen(yytext)); return print_token(T_I); }
"R"{NAME}			{ yylval.strval = (char*)malloc(strlen(yytext)); return print_token(T_R); }
"C"{NAME}			{ yylval.strval = (char*)malloc(strlen(yytext)); return print_token(T_C); }
"L"{NAME}			{ yylval.strval = (char*)malloc(strlen(yytext)); return print_token(T_L); }
"D"{NAME}			{ yylval.strval = (char*)malloc(strlen(yytext)); return print_token(T_D); }
"M"{NAME}			{ yylval.strval = (char*)malloc(strlen(yytext)); return print_token(T_M); }
"Q"{NAME}			{ yylval.strval = (char*)malloc(strlen(yytext)); return print_token(T_Q); }

 /* {INTEGER}			{ yyval.intval = atoi(yytext); return print_token(T_INTEGER); } */
 /* { FLOAT}				{ yylval.floatval = atof(yytext); return print_token(T_FLOAT); } */

"="					{ return print_token(T_EQUAL); }

.					{ yyerror("Unknown Character"); }
}


%%
/*------------ FUNCTIONS SECTION ------------*/

void yyerror(const char *s)
{
	static int errors = 0;
	errors++;

	printf(RED "ERROR: " RESET "%s in line %d\n", s, yylineno);

	if (errors > 5) {
		printf("%d errors encountered. Compilation not possible...\n", errors);
		exit(1);
	}
}

/* Prints a corresponding message for each token */
/* All messages are printed by defining VERBOSE_LEXER in constants.h */
int print_token(int token)
{
#ifdef VERBOSE_LEXER 
	printf("Printing a token\n");
	if (IS_EOF(token)) {
		printf("Found EOF\n");
	} else if (token == T_V) {
		printf("Found Voltage Source %d\n", yylval.intval);
	} else if (token == T_I) {
		printf("Found Current Source %d\n", yylval.intval);
	} else if (token == T_R) {
		printf("Found Resistor %d\n", yylval.intval);
	} else if (token == T_C) {
		printf("Found Capacitor %d\n", yylval.intval);
	} else if (token == T_L) {
		printf("Found Load %d\n", yylval.intval);
	} else if (token == T_D) {
		printf("Found Diod %d\n", yylval.intval);
	} else if (token == T_M) {
		printf("Found MOS Transistor %d\n", yylval.intval);
	} else if (token == T_Q) {
		printf("Found BJT Transistor %d\n", yylval.intval);
	} else if (token == T_INTEGER) {
		printf("Found Integer Number %d\n", yylval.intval);
	} else if (token == T_FLOAT) {
		printf("Found Floating Point Number %f\n", yylval.floatval);
	} else if (token == T_EQUAL) {
		printf("Found Equal\n");
	} else {
		yyerror("Unknown token\n");
		exit(1);
	}
#endif

	return token;
}
