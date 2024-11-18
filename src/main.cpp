#include <iostream>
#include <unordered_map>

#include <Eigen/Core>

#include "parser.h"
#include "lexer.h"

#include "node_table.h"
#include "netlist.h"
#include "mna.h"

spic::Netlist *netlist;
spic::NodeTable *node_table;
extern int error_count;

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
	
	if (error_count > 0) {
		exit(1);
	}

	int no_nodes = node_table->size();

	spic::MNASystemDC system = spic::MNASystemDC(*netlist, no_nodes);

	// Print the basic structures
	std::cout << *node_table;
	std::cout << *netlist;

	// Free memory & Cleanup
	delete node_table;
	delete netlist;
	fclose(yyin);

	return 0;
}
