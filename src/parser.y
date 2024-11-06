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
	spic::node_id_t find_or_append_node_int(int node);
	spic::node_id_t find_or_append_node_str(std::string *node);

	extern spic::NodeTable *node_table;
	extern spic::Netlist *netlist;
%}

%define parse.error verbose

%union	{
	int					intval;
	float				floatval;
	std::string			*strval;
	spic::Netlist		*netlist;
	spic::VoltageSource	*voltage_source;
	spic::CurrentSource *current_source;
	spic::Resistor 		*resistor;
	spic::Capacitor 	*capacitor;
	spic::Inductor 			*Inductor;
	spic::Diode			*diode;
	spic::MOS			*mos_transistor;
	spic::BJT			*bj_transistor;
}

//  optional required  optional  optional
// %token    <type>    <name>    <number>  "description"

%token	<strval> T_V	"Voltage Source"
%token	<strval> T_I	"Current Source"
%token	<strval> T_R	"Resistor"
%token	<strval> T_C	"Capacitor"
%token	<strval> T_L	"Inductor"
%token	<strval> T_D	"Diod"
%token	<strval> T_M	"MOS Transistor"
%token	<strval> T_Q	"BJT Transistor"

%token	T_LENGTH	"MOS Length"
%token	T_WIDTH		"MOS Width"
%token	T_AREA		"Area Factor of BJT/Diode"

%token	<intval>	T_INTEGER	"Integer Number"
%token	<floatval>	T_FLOAT		"Floating Point Number"
%token	<strval>	T_NAME 		"String Name"

%token	T_EOF	0	"EOF"

%type <floatval> value
%type <intval> node
%type <netlist> netlist spicefile
%type <voltage_source> v
%type <current_source> i
%type <resistor> r
%type <capacitor> c
%type <Inductor> l
%type <diode> d
%type <mos_transistor> m
%type <bj_transistor> q

%%
/* Rules */

// Structure of the file
spicefile: netlist { netlist = $1; }

// Netlist can contain multiple elements
netlist: netlist v { $$ = $1; $$->add_voltage_source($2); }
		| netlist i { $$ = $1; $$->add_current_source($2); }
		| netlist r { $$ = $1; $$->add_resistor($2); }
		| netlist c { $$ = $1; $$->add_capacitor($2); }
		| netlist l { $$ = $1; $$->add_inductor($2); }
		| netlist d { $$ = $1; $$->add_diode($2); }
		| netlist m { $$ = $1; $$->add_mos($2); }
		| netlist q { $$ = $1; $$->add_bjt($2); }
		| v { $$ = netlist; $$->add_voltage_source($1); }
		| i { $$ = netlist; $$->add_current_source($1); }
		| r { $$ = netlist; $$->add_resistor($1); }
		| c { $$ = netlist; $$->add_capacitor($1); }
		| l { $$ = netlist; $$->add_inductor($1); } 
		| d { $$ = netlist; $$->add_diode($1); }
		| m { $$ = netlist; $$->add_mos($1); }
		| q { $$ = netlist; $$->add_bjt($1); }

// Specifications for each element

v: T_V node node value { $$ = new spic::VoltageSource($1, $2, $3, $4); }

i: T_I node node value { $$ = new spic::CurrentSource($1, $2, $3, $4); }

r: T_R node node value { $$ = new spic::Resistor($1, $2, $3, $4); }

c: T_C node node value { $$ = new spic::Capacitor($1, $2, $3, $4); }

l: T_L node node value { $$ = new spic::Inductor($1, $2, $3, $4); }

d: T_D node node T_NAME T_AREA value { $$ = new spic::Diode($1, $2, $3, $4, $6); }
 | T_D node node T_NAME { $$ = new spic::Diode($1, $2, $3, $4, 1.0); }

m: T_M node node node node T_NAME T_LENGTH value T_WIDTH value { $$ = new spic::MOS($1, $2, $3, $4, $5, $6, $8, $10); }

q: T_Q node node node T_NAME T_AREA value { $$ = new spic::BJT($1, $2, $3, $4, $5, $7); }
 | T_Q node node node T_NAME { $$ = new spic::BJT($1, $2, $3, $4, $5, 1.0); }

node: T_INTEGER { $$ = find_or_append_node_int($1); }
	| T_NAME { $$ = find_or_append_node_str($1); }

value: T_FLOAT
	| T_INTEGER { $$ = (float) $1; }

%%

/* Search for an int node in the NodeTable and if it doesn't exist append it */
spic::node_id_t find_or_append_node_int(int node)
{
	spic::node_id_t id = node_table->find_node(node);
	if (id < 0)
		id = node_table->append_node(node);
	return id;
}

/* Search for a string node in the NodeTable and if it doesn't exist append it */
spic::node_id_t find_or_append_node_str(std::string *node)
{
	spic::node_id_t id = node_table->find_node(node);
	if (id < 0)
		id = node_table->append_node(node);
	return id;
}