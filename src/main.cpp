#include <iostream>
#include <unordered_map>

#include <Eigen/Core>

#include "parser.h"
#include "lexer.h"

#include "node_table.h"
#include "netlist.h"
#include "mna.h"

spic::Netlist *netlist_gptr;
spic::NodeTable *node_table_gptr;
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
	spic::NodeTable node_table = spic::NodeTable();
	spic::Netlist netlist = spic::Netlist();

	node_table_gptr = &node_table;
	netlist_gptr = &netlist;

	// Call the parser
	yyparse();
	
	if (error_count > 0) {
		exit(1);
	}
	
	std::cout << " * Finished parsing...\n" << std::endl;
	std::cout << node_table;
	std::cout << netlist;
	std::cout << std::endl << " * Generating MNA System...\n" << std::endl;

	spic::MNASystemDC system = spic::MNASystemDC(netlist, node_table.size());

	std::cout << system << std::endl;

	fclose(yyin);

	return 0;
}
