#include <iostream>
#include <unordered_map>

#include "parser.h"
#include "lexer.h"

#include "node_table.h"
#include "netlist.h"

spic::Netlist *netlist;
spic::NodeTable *node_table;

int main(int argc, char **argv)
{
	// Get input file.
	if (argc > 1) {
		// Declare which file to parse
		yyin = fopen(argv[1], "r");
		if (yyin == NULL) {
			char buff[20];
			sprintf(buff, "Error opening file %s", argv[1]);
			perror(buff);
			return -1;
		}
	}

	// Initialize the structures
	node_table = new spic::NodeTable();
	netlist = new spic::Netlist(); 

	// Call the parser
	yyparse();

	// Print the basic structures
	std::cout << *node_table;
	std::cout << *netlist; 

	// Free memory & Cleanup
	delete node_table;
	delete netlist;
	fclose(yyin);

	return 0;
}
