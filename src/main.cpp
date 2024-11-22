#include <iostream>
#include <unordered_map>

#include <Eigen/Core>

#include "parser.h"
#include "lexer.h"

#include "node_table.h"
#include "netlist.h"
#include "mna.h"
#include "util.h"

spic::Netlist *netlist_gptr;
spic::NodeTable *node_table_gptr;
extern int error_count;

int main(int argc, char **argv)
{
	Logger logger = Logger(std::cout);
	
	// Get input file.
	if (argc > 1) {
		// Declare which file to parse
		logger.log(INFO, "Opening file...");
		yyin = fopen(argv[1], "r");
		if (yyin == NULL) {
			std::ostringstream strstream;
			strstream << "Error opening file " << argv[1];
			logger.log(ERROR, strstream);
			return -1;
		}
	}

	// Initialize the structures
	logger.log(INFO, "Initializing netlist...");
	spic::NodeTable node_table = spic::NodeTable();
	spic::Netlist netlist = spic::Netlist();

	node_table_gptr = &node_table;
	netlist_gptr = &netlist;

	// Call the parser
	logger.log(INFO, "Calling parser...");
	yyparse();
	
	if (error_count > 0) {
		logger.log(ERROR, "Finished parsing with errors.");
		exit(1);
	} else {
		logger.log(INFO, "Parsing finished successfully.");
	}
	
	// Show the node table and the netlist
	std::cout << node_table;
	std::cout << netlist;

	// Construct MNA System
	logger.log(INFO, "Constructing MNA System for DC analysis.");
	spic::MNASystemDC system = spic::MNASystemDC(netlist, node_table.size());
	// logger.increaseTabs();
	// std::ostringstream strstream;
	// strstream << system << std::endl;
	std::cout << system << std::endl;
	// logger.log(INFO, strstream);
	// logger.decreaseTabs();

	logger.log(INFO, "Simulator finished. Exiting...");
	fclose(yyin);

	return 0;
}
