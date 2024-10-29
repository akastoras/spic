%code requires {
	#include <string>
	#include "node_table.h"
	#include "element_lists.h"
}

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
	int				intval;
	float			floatval;
	std::string		*strval;
	spic::Netlist	*netlist;
	spic::Element	*element;
	spic::VoltageSource	*voltage_source;
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

%token	T_EQUAL "="

%token	<intval>	T_INTEGER	"Integer Number"
%token	<floatval>	T_FLOAT		"Floating Point Number"
%token	<strval>	T_NAME 		"String Name"

%token	T_EOF	0	"EOF"

%type <floatval> value
%type <intval> node
%type <netlist> netlist
%type <voltage_source> v
%type <element> element

%%

// Rules
netlist: netlist element { $$ = spic::Netlist(); }
	| element { $$ = spic::Netlist(); }
	;

element: v { $$.v = $1 }
	;


v: T_V node node value {}
	;


/* i: T_I T_NUMBER

r: T_R T_NUMBER

c: T_C T_NUMBER

l: T_L T_NUMBER

d: T_D T_NUMBER

m: T_M T_NUMBER

q: T_Q T_NUMBER */

node: T_INTEGER { $$.intval = NodeTable.append_node($1.intval); }
	| T_NAME { $$.intval = NodeTable.append_node($1.strval); }
	;

value: T_FLOAT
	| T_INTEGER { $$ = (float) $1 }
	;

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
