%code requires {
	#include <string>
	#include "node_table.h"
	#include "netlist.h"
}

%{
	#include "parser.h"

	extern FILE *yyin;
	extern int error_count;
	extern int yylex();
	extern void yyerror(const char *err);
	void spic_parser_init();
	spic::node_id_t find_or_append_node_int(int node);
	spic::node_id_t find_or_append_node_str(std::string *node);
	void check_add_element(bool res, const std::string &element_name, const std::string &name);

	extern spic::NodeTable *node_table_gptr;
	extern spic::Netlist *netlist_gptr;
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
	spic::Inductor 		*Inductor;
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
%token	T_MINUS		"Minus Operator"
%token 	T_PLUS		"Plus Operator"

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
spicefile: netlist

// Netlist can contain multiple elements
netlist:  netlist v { $$ = $1; check_add_element($$->add_voltage_source($2),"Voltage Source",	$2->name); delete $2; }
		| netlist i { $$ = $1; check_add_element($$->add_current_source($2),"Current Source",	$2->name); delete $2; }
		| netlist r { $$ = $1; check_add_element($$->add_resistor($2), 		"Resistor",			$2->name); delete $2; }
		| netlist c { $$ = $1; check_add_element($$->add_capacitor($2), 	"Capacitor",		$2->name); delete $2; }
		| netlist l { $$ = $1; check_add_element($$->add_inductor($2), 		"Inductor",			$2->name); delete $2; }
		| netlist d { $$ = $1; check_add_element($$->add_diode($2), 		"Diode",			$2->name); delete $2; }
		| netlist m { $$ = $1; check_add_element($$->add_mos($2), 			"MOS",				$2->name); delete $2; }
		| netlist q { $$ = $1; check_add_element($$->add_bjt($2), 			"BJT",				$2->name); delete $2; }
		| v { $$ = netlist_gptr; $$->add_voltage_source($1); delete $1; }
		| i { $$ = netlist_gptr; $$->add_current_source($1); delete $1; }
		| r { $$ = netlist_gptr; $$->add_resistor($1);       delete $1; }
		| c { $$ = netlist_gptr; $$->add_capacitor($1);      delete $1; }
		| l { $$ = netlist_gptr; $$->add_inductor($1);       delete $1; }
		| d { $$ = netlist_gptr; $$->add_diode($1);          delete $1; }
		| m { $$ = netlist_gptr; $$->add_mos($1);            delete $1; }
		| q { $$ = netlist_gptr; $$->add_bjt($1);            delete $1; }

// Specifications for each element

v: T_V node node value { $$ = new spic::VoltageSource($1, $2, $3, $4); delete $1; }
i: T_I node node value { $$ = new spic::CurrentSource($1, $2, $3, $4); delete $1; }
r: T_R node node value { $$ = new spic::Resistor     ($1, $2, $3, $4); delete $1; }
c: T_C node node value { $$ = new spic::Capacitor    ($1, $2, $3, $4); delete $1; }
l: T_L node node value { $$ = new spic::Inductor     ($1, $2, $3, $4); delete $1; }

d: T_D node node T_NAME T_AREA value { $$ = new spic::Diode($1, $2, $3, $4, $6);  delete $1; delete $4; }
 | T_D node node T_NAME              { $$ = new spic::Diode($1, $2, $3, $4, 1.0); delete $1; delete $4; }

m: T_M node node node node T_NAME T_LENGTH value T_WIDTH value { $$ = new spic::MOS($1, $2, $3, $4, $5, $6, $8, $10); delete $1; delete $6; }

q: T_Q node node node T_NAME T_AREA value { $$ = new spic::BJT($1, $2, $3, $4, $5, $7); delete $1; delete $5; }
 | T_Q node node node T_NAME { $$ = new spic::BJT($1, $2, $3, $4, $5, 1.0);             delete $1; delete $5; }

node: T_INTEGER { $$ = find_or_append_node_int($1); }
	| T_NAME { $$ = find_or_append_node_str($1); delete $1; }

value: T_PLUS T_FLOAT { $$ = $2; }
	| T_MINUS T_FLOAT { $$ = -$2; }
	| T_FLOAT
	| T_PLUS T_INTEGER { $$ = (float) $2; }
	| T_MINUS T_INTEGER { $$ = (float) -$2; }
	| T_INTEGER { $$ = (float) $1; }

%%

/* Search for an int node in the NodeTable and if it doesn't exist append it */
spic::node_id_t find_or_append_node_int(int node)
{
	spic::node_id_t id = node_table_gptr->find_node(node);
	if (id < 0)
		id = node_table_gptr->append_node(node);
	return id;
}

/* Search for a string node in the NodeTable and if it doesn't exist append it */
spic::node_id_t find_or_append_node_str(std::string *node)
{
	spic::node_id_t id = node_table_gptr->find_node(node);
	if (id < 0)
		id = node_table_gptr->append_node(node);
	return id;
}

void check_add_element(bool res, const std::string &element_name, const std::string &name)
{
	if (!res) {
		yyerror(("Duplicate " + element_name + " name: '" + name + "'").c_str());
	}
}
