#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <sstream> 

#include <Eigen/Core>

#include <boost/program_options.hpp>

#include "parser.h"
#include "lexer.h"


#include "commands.h"
#include "node_table.h"
#include "netlist.h"
#include "system.h"
#include "util.h"
#include "solver.h"

spic::Netlist   netlist;
spic::NodeTable node_table;
spic::Commands  commands;

extern int error_count;

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
	commands.options = {false, false};
	Logger logger = Logger(std::cout);

	// Define the supported options
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("cir_file", po::value<std::string>(), "Path to the circuit file")
		("output_dir", po::value<std::string>()->default_value(""), "Output directory (default: <filename>_golden)");

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
	} catch (const po::error &ex) {
		std::cerr << ex.what() << '\n';
		return 1;
	}

	if (vm.count("help")) {
		std::cout << desc << "\n";
		return 1;
	}

	if (!vm.count("cir_file")) {
		std::cerr << "Circuit file is required.\n";
		return 1;
	}

	std::string cir_file = vm["cir_file"].as<std::string>();
	std::string output_dir = vm["output_dir"].as<std::string>();

	// Your existing code to handle cir_file and output_dir
	std::cout << "Circuit file: " << cir_file << "\n";
	std::cout << "Output directory: " << output_dir << "\n";

	yyin = fopen(cir_file.c_str(), "r");
	if (yyin == NULL) {
		std::ostringstream strstream;
		strstream << "Error opening file " << argv[1];
		logger.log(ERROR, strstream);
		return -1;
	}

	// Initialize the structures
	logger.log(INFO, "Initializing netlist...");

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

	// std::cout << system << std::endl;

	spic::Solver slv = spic::Solver(system, commands.options, logger);

	Eigen::MatrixXd A_unchained = system.A;

	slv.decompose();
	slv.solve(system.b);

	std::cout << "Residual: " << (A_unchained * system.x - system.b).norm() << "\n";

	// Delete output_dir if it exists and then create it again
	if (std::filesystem::exists(output_dir)) {
		logger.log(WARNING, output_dir + " exists... removing it");
		std::filesystem::remove_all(output_dir);
	}
	logger.log(INFO, "Creating " + output_dir);
	std::filesystem::create_directories(output_dir);

	logger.log(INFO, "Copying " + cir_file +  " file to " + output_dir);
	std::filesystem::copy(cir_file, output_dir);

	// For each node in the node table:
	std::ofstream file;
	file.open(output_dir + "/dc_op.dat");
	for (auto &it : node_table.table) {
		if (it.first != "0")
			file << it.first << " " << std::setprecision(std::numeric_limits<double>::max_digits10) << system.x[it.second - 1] << std::endl;
	}
	// TODO: make output_dir a std::filesystem::path etc


	logger.log(INFO, "Simulator finished. Exiting...");
	fclose(yyin);

	return 0;
}