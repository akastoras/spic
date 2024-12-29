/*------------ DECLARATION SECTION ------------*/
%{
	#include <cstdlib>
	#include <iostream>
	#include <string>
	#include <cmath>
	#include <cstring>
	#include <cctype>
	#include <bits/stdc++.h>

	#include "parser.h"

	// Colors
	#define RED     "\033[31m"      /* Red */
	#define RESET   "\033[0m"

	#define TOUPPER(str) std::transform(str->begin(), str->end(), str->begin(),[](unsigned char c) { return std::toupper(c); })

	int error_count = 0;

	void yyerror(const char *s);
	int print_token(int token);
	float parse_value_with_exponent(const char *text);
	void toUpper(std::string* str);
%}

%option case-insensitive
%option yylineno
%option noyywrap

/* Declare Start Conditions */
%x COMMENT ELEMENT_PARAMS OPTIONS COMMANDS NODES END

WHITECHAR	[ \r\v\n]

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
{FLOAT}				{ yylval.floatval = atof(yytext); return print_token(T_FLOAT); }
{NAME}				{ yylval.strval = new std::string(yytext); TOUPPER(yylval.strval); return print_token(T_NAME); }

"+"					{ return print_token(T_PLUS); }
"-"					{ return print_token(T_MINUS); }
"L="				{ return print_token(T_LENGTH); }
"W="				{ return print_token(T_WIDTH); }
"area="				{ return print_token(T_AREA); }

.					{ std::cout << "\"" << yytext << "\"" << "\n"; yyerror("Unknown Character"); }
}

<COMMANDS>{
\n		{ BEGIN(INITIAL); }

{WHITECHAR}			{ /* Ignore white spaces */ }

"V"{NAME}			{ yylval.strval = new std::string(yytext+1); TOUPPER(yylval.strval); return print_token(T_V); }
"I"{NAME}			{ yylval.strval = new std::string(yytext+1); TOUPPER(yylval.strval); return print_token(T_I); }
{INTEGER}			{ yylval.intval = atoi(yytext); return print_token(T_INTEGER); }
{FLOAT}				{ yylval.floatval = atof(yytext); return print_token(T_FLOAT); }

"+"					{ return print_token(T_PLUS); }
"-"					{ return print_token(T_MINUS); }

.					{ std::cout << "\"" << yytext << "\"" << "\n"; yyerror("Unknown Character"); }
}

<OPTIONS>{
\n					{ BEGIN(INITIAL); }

{WHITECHAR}			{ /* Ignore white spaces */ }

"SPD"				{ return print_token(T_SPD); }
"CUSTOM"			{ return print_token(T_CUSTOM); }
"ITER"				{ return print_token(T_ITER); }
"ITOL="				{ return print_token(T_ITOL); }
"SPARSE"			{ return print_token(T_SPARSE); }

{FLOAT}				{ yylval.floatval = atof(yytext); return print_token(T_FLOAT); }

.					{ std::cout << "\"" << yytext << "\"" << "\n"; yyerror("Unknown Character"); }
}

<NODES>{
\n					{ BEGIN(INITIAL); }
{WHITECHAR}			{ /* Ignore white spaces */ }

"DC"				{ /* Ignore DC */ }
"V("{NAME}/")"		{ yylval.strval = new std::string(yytext+2); TOUPPER(yylval.strval); return print_token(T_VNODE); }
")"					{ /* Ignore closing parenthesis */ }
"."					{ std::cout << "\"" << yytext << "\"" << "\n"; yyerror("Unknown Character"); }
}

<END>{
{WHITECHAR}			{ /* Ignore white spaces */ }
<<EOF>>				{ return print_token(T_EOF); }

.					{ yyerror("Unexpected character after .END"); }
}

<INITIAL>{
"*"					{ BEGIN(COMMENT); }

".END"				{ BEGIN(END); }
<<EOF>>				{ return print_token(T_EOF); }
{WHITECHAR}			{ /* Ignore white spaces */ }

"V"{NAME}			{ yylval.strval = new std::string(yytext+1); TOUPPER(yylval.strval); BEGIN(ELEMENT_PARAMS); return print_token(T_V); }
"I"{NAME}			{ yylval.strval = new std::string(yytext+1); TOUPPER(yylval.strval); BEGIN(ELEMENT_PARAMS); return print_token(T_I); }
"R"{NAME}			{ yylval.strval = new std::string(yytext+1); TOUPPER(yylval.strval); BEGIN(ELEMENT_PARAMS); return print_token(T_R); }
"C"{NAME}			{ yylval.strval = new std::string(yytext+1); TOUPPER(yylval.strval); BEGIN(ELEMENT_PARAMS); return print_token(T_C); }
"L"{NAME}			{ yylval.strval = new std::string(yytext+1); TOUPPER(yylval.strval); BEGIN(ELEMENT_PARAMS); return print_token(T_L); }
"D"{NAME}			{ yylval.strval = new std::string(yytext+1); TOUPPER(yylval.strval); BEGIN(ELEMENT_PARAMS); return print_token(T_D); }
"M"{NAME}			{ yylval.strval = new std::string(yytext+1); TOUPPER(yylval.strval); BEGIN(ELEMENT_PARAMS); return print_token(T_M); }
"Q"{NAME}			{ yylval.strval = new std::string(yytext+1); TOUPPER(yylval.strval); BEGIN(ELEMENT_PARAMS); return print_token(T_Q); }

".OPTIONS"			{ BEGIN(OPTIONS); return print_token(T_OPTIONS); }
".DC"				{ BEGIN(COMMANDS); return print_token(T_DC); }
".OP"				{ /* Ignore .OP command */ }
".PLOT"				{ BEGIN(NODES); return print_token(T_PLOT); }
".PRINT"			{ BEGIN(NODES); return print_token(T_PRINT); }

"+"					{ BEGIN(ELEMENT_PARAMS); return print_token(T_PLUS); }

.					{ std::cout << "\"" << yytext << "\"" << "\n"; yyerror("Unknown Character"); }
}


%%
/*------------ FUNCTIONS SECTION ------------*/

void yyerror(const char *s)
{
	error_count++;

	std::cout << RED "ERROR: " RESET << s << " in line " << yylineno << "\n";

	if (error_count > 2) {
		std::cout << error_count << " errors encountered. Compilation not possible...\n";
		exit(1);
	}
}

/* Prints a corresponding message for each token */
/* All messages are printed by defining VERBOSE_LEXER in constants.h */
int print_token(int token)
{
#if VERBOSE_LEXER == 1
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
		std::cout << "Found Inductor " << *yylval.strval << "\n";
	} else if (token == T_D) {
		std::cout << "Found Diode " << *yylval.strval << "\n";
	} else if (token == T_M) {
		std::cout << "Found MOS Transistor " << *yylval.strval << "\n";
	} else if (token == T_Q) {
		std::cout << "Found BJT Transistor " << *yylval.strval << "\n";	
	} else if (token == T_MINUS) {
		std::cout << "Found Minus\n";
	} else if (token ==T_PLUS) {
		std::cout << "Found Plus\n";
	} else if (token == T_INTEGER) {
		std::cout << "Found Integer " << yylval.intval << "\n";
	} else if (token == T_FLOAT) {
		std::cout << "Found Float " << yylval.floatval << "\n";
	} else if (token == T_LENGTH) {
		std::cout << "Found MOS Length\n";
	} else if (token == T_WIDTH) {
		std::cout << "Found MOS Width\n";
	} else if (token == T_AREA) {
		std::cout << "Found Area Factor of BJT/Diode\n";
	} else if (token == T_NAME) {
		std::cout << "Found Name " << *yylval.strval << "\n";
	} else if (token == T_OPTIONS) {
		std::cout << "Found Options Command\n";
	} else if (token == T_DC) {
		std::cout << "Found DC Command\n";
	} else if (token == T_SPD) {
		std::cout << "Found SPD Option\n";
	} else if (token == T_CUSTOM) {
		std::cout << "Found Custom Option\n";
	} else if (token == T_PLOT) {
		std::cout << "Found Plot Command\n";
	} else if (token == T_PRINT) {
		std::cout << "Found Print Command\n";
	} else if (token == T_VNODE) {
		std::cout << "Found Voltage at Node " << *yylval.strval << "\n";
	} else if (token == T_ITER) {
		std::cout << "Found Iterative Method Command\n";
	} else if (token == T_ITOL) {
		std::cout << "Found Iteration Tolerance\n";
	} else if (token == T_SPARSE) {
		std::cout << "Found Sparse Matrix Option\n";
	} else {
		yyerror("Unknown token\n");
		exit(1);
	}
#endif

	return token;
}