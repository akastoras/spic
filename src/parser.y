%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <string.h>
	#include <errno.h>
	#include <stdbool.h>
	
	#include "parser.h"

	extern FILE *yyin;
	extern int yylex();
	extern void yyerror(const char *err);
%}

%define parse.error verbose

%union	{
	int		intval;
	float	floatval;
	char	*strval;
}

//  optional required  optional  optional
// %token    <type>    <name>    <number>  "description"

%token	T_V	"Voltage Source"
%token	T_I	"Current Source"
%token	T_R	"Resistor"
%token	T_C	"Capacitor"
%token	T_L	"Load"
%token	T_D	"Diod"
%token	T_M	"MOS Transistor"
%token	T_Q	"BJT Transistor"

%token T_EQUAL "="

%token T_INTEGER	"Integer Number"
%token T_FLOAT		"Floating Point Number"
%token	T_LINEBREAK	"Line Break"
%token	T_EOF	0	"EOF"

%%

// Rules
netlist: netlist element T_LINEBREAK
	| element T_LINEBREAK
	;

element: T_LINEBREAK
	;

/* v: T_V T_NUMBER

i: T_I T_NUMBER

r: T_R T_NUMBER

c: T_C T_NUMBER

l: T_L T_NUMBER

d: T_D T_NUMBER

m: T_M T_NUMBER

q: T_Q T_NUMBER */


%%
/* 
int main(int argc, char **argv)
{
	// Get input file.
	if (argc > 1) {
		yyin = fopen(argv[1], "r");
		if (yyin == NULL) {
			char buff[20];
			sprintf(buff, "Error opening file %s", argv[1]);
			perror(buff);
			return -1;
		}
	}
	
	// This is where the magic happens
	yyparse();

	// Free memory & Cleanup
	fclose(yyin);

	return 0;
} */
