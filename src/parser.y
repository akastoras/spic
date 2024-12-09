%code requires {
	#include <string>
	#include "node_table.h"
	#include "netlist.h"
	#include "commands.h"
}

%{
	#include "parser.h"

	std::vector<std::string> *global_node_list_ptr;

	extern FILE *yyin;
	extern int error_count;
	extern int yylex();
	extern void yyerror(const char *err);
	void spic_parser_init();
	spic::node_id_t find_or_append_node_int(int node);
	spic::node_id_t find_or_append_node_str(std::string *node);
	void check_add_element(bool res, const std::string &element_name, const std::string &name);
	void check_dc_sweep(bool res, const std::string &element_name, const std::string &name);
	void add_node_to_list(std::string *node_name);
	void check_commands();
%}

%define parse.error verbose

%union	{
	int						 intval;
	float					 floatval;
	std::string				 *strval;
	spic::VoltageSource		 *voltage_source;
	spic::CurrentSource 	 *current_source;
	spic::Resistor 			 *resistor;
	spic::Capacitor 		 *capacitor;
	spic::Inductor 			 *Inductor;
	spic::Diode				 *diode;
	spic::MOS				 *mos_transistor;
	spic::BJT				 *bj_transistor;
}

//  optional required  optional  optional
// %token    <type>    <name>    <number>  "description"

%token	<strval> T_V	"Voltage Source"
%token	<strval> T_I	"Current Source"
%token	<strval> T_R	"Resistor"
%token	<strval> T_C	"Capacitor"
%token	<strval> T_L	"Inductor"
%token	<strval> T_D	"Diode"
%token	<strval> T_M	"MOS Transistor"
%token	<strval> T_Q	"BJT Transistor"

%token	T_LENGTH	"MOS Length"
%token	T_WIDTH		"MOS Width"
%token	T_AREA		"Area Factor of BJT/Diode"
%token	T_MINUS		"Minus Operator"
%token	T_PLUS		"Plus Operator"
%token	T_OPTIONS	".OPTIONS"
%token	T_SPD		"MNA static matrix is SPD"
%token	T_CUSTOM	"MNA system should be solved with custom solver"
%token	T_DC		".DC"
%token	T_PRINT		".PRINT"
%token	T_PLOT		".PLOT"

%token	<intval>	T_INTEGER	"Integer Number"
%token	<floatval>	T_FLOAT		"Floating Point Number"
%token	<strval>	T_NAME 		"String Name"
%token	<strval>	T_VNODE		"V(node) used in .PRINT/.PLOT"

%token	T_EOF	0	"EOF"

%type <floatval> value
%type <intval> node
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
spicefile: netlist commands { check_commands(); }

// Netlist can contain multiple elements
netlist:  netlist v { check_add_element(netlist.add_voltage_source($2),	"Voltage Source",	$2->name); delete $2; }
		| netlist i { check_add_element(netlist.add_current_source($2),	"Current Source",	$2->name); delete $2; }
		| netlist r { check_add_element(netlist.add_resistor($2), 		"Resistor",			$2->name); delete $2; }
		| netlist c { check_add_element(netlist.add_capacitor($2), 		"Capacitor",		$2->name); delete $2; }
		| netlist l { check_add_element(netlist.add_inductor($2), 		"Inductor",			$2->name); delete $2; }
		| netlist d { check_add_element(netlist.add_diode($2), 			"Diode",			$2->name); delete $2; }
		| netlist m { check_add_element(netlist.add_mos($2), 			"MOS",				$2->name); delete $2; }
		| netlist q { check_add_element(netlist.add_bjt($2), 			"BJT",				$2->name); delete $2; }
		| /* empty */

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

commands: command commands
		| /* empty */

// Options for the simulation
command: T_OPTIONS options  
		| T_DC T_V value value value { check_dc_sweep(commands.add_v_dc_sweep(*$2, $3, $4, $5), "Voltage Source", *$2); }
		| T_DC T_I value value value { check_dc_sweep(commands.add_i_dc_sweep(*$2, $3, $4, $5), "Current Source", *$2); }
		| T_PRINT { global_node_list_ptr = &commands.print_nodes; } v_nodes
		| T_PLOT { global_node_list_ptr = &commands.plot_nodes; } v_nodes

options: options T_SPD { commands.options.spd = true; }
		| options T_CUSTOM { commands.options.custom = true; }
		| T_SPD { commands.options.spd = true; }
		| T_CUSTOM { commands.options.custom = true; }

v_nodes: v_nodes T_VNODE { add_node_to_list($2); delete $2; }
	| T_VNODE { add_node_to_list($1); delete $1; }

%%

/* Search for an int node in the NodeTable and if it doesn't exist append it */
spic::node_id_t find_or_append_node_int(int node)
{
	spic::node_id_t id = node_table.find_node(node);
	if (id < 0)
		id = node_table.append_node(node);
	return id;
}

/* Search for a string node in the NodeTable and if it doesn't exist append it */
spic::node_id_t find_or_append_node_str(std::string *node)
{
	spic::node_id_t id = node_table.find_node(node);
	if (id < 0)
		id = node_table.append_node(node);
	return id;
}

/* Checks the return value of add_element function and prints error message if needed */
void check_add_element(bool res, const std::string &element_name, const std::string &name)
{
	if (!res) {
		yyerror(("Duplicate " + element_name + " name: '" + name + "'").c_str());
	}
}

/* Checks the return value of dc_sweep creation function and prints error message if needed */
void check_dc_sweep(bool res, const std::string &element_name, const std::string &name)
{
	if (!res) {
		yyerror(("DC Sweep on non-existent " + element_name + " name: '" + name + "'").c_str());
	}
}

/* Searched for a node in a list and  */
void add_node_to_list(std::string *node_name)
{
	if (node_table.find_node(node_name) == -1) {
		yyerror(("Node " + *node_name + " used in print/plot does not exist").c_str());
	} else if (std::find(global_node_list_ptr->begin(), global_node_list_ptr->end(), *node_name) == global_node_list_ptr->end()) {
		global_node_list_ptr->push_back(*node_name);
	}
	/* else {
		yyerror(("Node" + *node_name + "used in print/plot more than once").c_str());
	} */
}

void check_commands()
{
	bool no_node_printed = (commands.print_nodes.empty() && commands.plot_nodes.empty());
	bool no_sweeps = (commands.v_dc_sweeps.empty() && commands.i_dc_sweeps.empty());

	if (no_sweeps ^ no_node_printed) {
		yyerror("DC Sweep and Print/Plot commands must be used together");
	}
}