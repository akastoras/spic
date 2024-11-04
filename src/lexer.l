/*------------ DECLARATION SECTION ------------*/
%{
	#include <cstdlib>
	#include <iostream>
	#include <string>
	#include <cmath>

	#include "constants.h"

	#include "parser.h"

	// Colors
	#define RED     "\033[31m"      /* Red */
	#define RESET   "\033[0m"

	int print_token(int token);
	float parse_value_with_exponent(const char *text);
%}

%option case-insensitive
%option yylineno
%option noyywrap

/* Declare Start Conditions */
%x COMMENT ELEMENT_PARAMS

WHITECHAR	[ \v\n]

ALPHA		[a-z]
NUM			[0-9]
ALNUM		{ALPHA}|{NUM}

NAME		({ALNUM}|_)+

INTEGER		{NUM}+
FLOAT1		{NUM}+"."{NUM}+
FLOAT2 		{NUM}+("."{NUM}+)?(E[+-]?{NUM}+)
FLOAT		{FLOAT1}|{FLOAT2}

/*------------ RULES SECTION ------------*/
%%

<COMMENT>{
\n		{ BEGIN(INITIAL); }
[^\n]+
}

<ELEMENT_PARAMS>{
\n		{ BEGIN(INITIAL); }

{WHITECHAR}			{ /* Ignore white spaces */ }

{INTEGER}			{ yylval.intval = atoi(yytext); return print_token(T_INTEGER); }
{FLOAT}			{ yylval.floatval = atof(yytext); return print_token(T_FLOAT); }
{NAME}				{ yylval.strval = new std::string(yytext); return print_token(T_NAME); }

"L="				{ return print_token(T_LENGTH); }
"W="				{ return print_token(T_WIDTH); }
}


<INITIAL>{
"*"		{ BEGIN(COMMENT); }

<<EOF>>			{ return print_token(T_EOF); }
{WHITECHAR}		{  /* Ignore white spaces */ }

"V"{NAME}			{ yylval.strval = new std::string(yytext+1); BEGIN(ELEMENT_PARAMS); return print_token(T_V); }
"I"{NAME}			{ yylval.strval = new std::string(yytext+1); BEGIN(ELEMENT_PARAMS); return print_token(T_I); }
"R"{NAME}			{ yylval.strval = new std::string(yytext+1); BEGIN(ELEMENT_PARAMS); return print_token(T_R); }
"C"{NAME}			{ yylval.strval = new std::string(yytext+1); BEGIN(ELEMENT_PARAMS); return print_token(T_C); }
"L"{NAME}			{ yylval.strval = new std::string(yytext+1); BEGIN(ELEMENT_PARAMS); return print_token(T_L); }
"D"{NAME}			{ yylval.strval = new std::string(yytext+1); BEGIN(ELEMENT_PARAMS); return print_token(T_D); }
"M"{NAME}			{ yylval.strval = new std::string(yytext+1); BEGIN(ELEMENT_PARAMS); return print_token(T_M); }
"Q"{NAME}			{ yylval.strval = new std::string(yytext+1); BEGIN(ELEMENT_PARAMS); return print_token(T_Q); }

"+"					{ BEGIN(ELEMENT_PARAMS); }

.					{ std::cout << "\"" << yytext << "\"" << "\n"; yyerror("Unknown Character"); }
}


%%
/*------------ FUNCTIONS SECTION ------------*/

void yyerror(const char *s)
{
	static int errors = 0;
	errors++;

	std::cout << RED "ERROR: " RESET << s << " in line " << yylineno << "\n";

	if (errors > 5) {
		std::cout << errors << " errors encountered. Compilation not possible...\n";
		exit(1);
	}
}

/* Prints a corresponding message for each token */
/* All messages are printed by defining VERBOSE_LEXER in constants.h */
int print_token(int token)
{
#ifdef VERBOSE_LEXER 
	if (token == T_EOF) {
		std::cout << "Found EOF\n";
	} else if (token == T_V) {
		std::cout << "Found Voltage Source " << *yylval.strval << "\n";
	} else if (token == T_I) {
		std::cout << "Found Current Source " << *yylval.strval << "\n";
	} else if (token == T_R) {
		std::cout << "Found Resistor " << *yylval.strval << "\n";
	} else if (token == T_C) {
		std::cout << "Found Capacitor " << *yylval.strval << "\n";
	} else if (token == T_L) {
		std::cout << "Found Load " << *yylval.strval << "\n";
	} else if (token == T_D) {
		std::cout << "Found Diode " << *yylval.strval << "\n";
	} else if (token == T_M) {
		std::cout << "Found MOS Transistor " << *yylval.strval << "\n";
	} else if (token == T_Q) {
		std::cout << "Found BJT Transistor " << *yylval.strval << "\n";
	} else if (token == T_INTEGER) {
		std::cout << "Found Integer " << yylval.intval << "\n";
	} else if (token == T_FLOAT) {
		std::cout << "Found Float " << yylval.floatval << "\n";
	} else if (token == T_EQUAL) {
		std::cout << "Found Equal\n";
	} else {
		yyerror("Unknown token\n");
		exit(1);
	}
#endif

	return token;
}
