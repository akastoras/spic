%code requires {
	#include <string>
	#include "node_table.h"
	#include "netlist.h"
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
	void spic_parser_init();

	spic::NodeTable *node_table;
%}

%define parse.error verbose

%union	{
	int				intval;
	float			floatval;
	std::string		*strval;
	spic::Netlist	*netlist;
	spic::VoltageSource	*voltage_source;
}

//  optional required  optional  optional
// %token    <type>    <name>    <number>  "description"

%token	<strval> T_V	"Voltage Source"
%token	<strval> T_I	"Current Source"
%token	<strval> T_R	"Resistor"
%token	<strval> T_C	"Capacitor"
%token	<strval> T_L	"Load"
%token	<strval> T_D	"Diod"
%token	<strval> T_M	"MOS Transistor"
%token	<strval> T_Q	"BJT Transistor"

%token	<floatval> T_LENGTH "MOS Length"
%token	<floatval> T_WIDTH "MOS Width"

%token	<intval>	T_INTEGER	"Integer Number"
%token	<floatval>	T_FLOAT		"Floating Point Number"
%token	<strval>	T_NAME 		"String Name"

%token	T_EOF	0	"EOF"

%type <floatval> value
%type <intval> node
%type <netlist> netlist spicefile
%type <voltage_source> v

%%

spicefile: { spic_parser_init(); } netlist { std::cout << *$2; }

// Rules
netlist: netlist v { $$ = $1; $$->add_voltage_source($2); }
	| v { $$ = new spic::Netlist(); $$->add_voltage_source($1); }
	;


v: T_V node node value { $$ = new spic::VoltageSource($1, $2, $3, $4); }
	;


/* i: T_I T_NUMBER

r: T_R T_NUMBER

c: T_C T_NUMBER

l: T_L T_NUMBER

d: T_D T_NUMBER

m: T_M node node node node T_NAME T_LENGTH value T_WIDTH value

q: T_Q T_NUMBER */

node: T_INTEGER { $$ = node_table->append_node($1); }
	| T_NAME { $$ = node_table->append_node($1); }
	;

value: T_FLOAT
	| T_INTEGER { $$ = (float) $1; }
	;

%%

void spic_parser_init()
{
	node_table = new spic::NodeTable();
}
