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

void parse_arguments(po::variables_map &vm, int argc, char** argv);
void parse_spice_file(std::filesystem::path cir_file, Logger &logger);
void solve_operating_point(spic::Solver &slv, spic::MNASystemDC &system,
	const std::filesystem::path &output_dir, const std::string &output_dir_str,
	const std::filesystem::path &cir_file, const std::string &cir_file_str, Logger &logger);

int main(int argc, char** argv)
{
	Logger logger = Logger(std::cout);

	// Define the supported options
	po::variables_map vm;
	parse_arguments(vm, argc, argv);
	
	std::string cir_file_str = vm["cir_file"].as<std::string>();
	std::string output_dir_str = vm["output_dir"].as<std::string>();
	std::filesystem::path cir_file(cir_file_str);
	std::filesystem::path output_dir(output_dir_str);

	// Parse the spice circuit file that constructs the netlist
	// the node_table and the commands structures
	parse_spice_file(cir_file, logger);

	// Construct MNA System
	logger.log(INFO, "Constructing MNA System for DC analysis.");
	spic::MNASystemDC system = spic::MNASystemDC(netlist, node_table.size());

	// Construct a Solver object
	spic::Solver slv = spic::Solver(system, commands.options.spd,
									commands.options.custom, logger);

	// Solve on the operating point
	solve_operating_point(slv, system, output_dir, output_dir_str,
									cir_file, cir_file_str, logger);

	// Perform the dc sweeps (if there are any)
	commands.dc_sweeps_dir = output_dir/"dc_sweeps";
	commands.perform_dc_sweeps(slv, logger);

	logger.log(INFO, "Simulator finished. Exiting...");
	return 0;
}


/* Function that parses the program arguments of spic */
void parse_arguments(po::variables_map &vm, int argc, char **argv) {
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("cir_file", po::value<std::string>(), "Path to the circuit file")
		("output_dir", po::value<std::string>()->default_value(""),
							"Output directory (default: <filename>_golden)");

	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
	} catch (const po::error &ex) {
		std::cerr << ex.what() << '\n';
		exit(1);
	}

	if (vm.count("help")) {
		std::cout << desc << "\n";
		exit(1);
	}

	if (!vm.count("cir_file")) {
		std::cerr << "Circuit file is required.\n";
		exit(1);
	}
}

// Function that calls the opens the spice file, calls the parser and checks for errors
void parse_spice_file(std::filesystem::path cir_file, Logger &logger) {
	yyin = fopen(cir_file.c_str(), "r");
	if (yyin == NULL) {
		logger.log(ERROR, "Error opening file " + cir_file.string());
		exit(1);
	}

	// Call the parser
	logger.log(INFO, "Calling parser...");
	yyparse();

	// Delete file pointer
	fclose(yyin);

	// Check for errors
	if (error_count > 0) {
		logger.log(ERROR, "Finished parsing with errors.");
		exit(1);
	} else {
		logger.log(INFO, "Parsing finished successfully.");
	}

	// Show the node table and the netlist
	std::cout << node_table;
	std::cout << netlist;
	std::cout << commands;
}

void solve_operating_point(spic::Solver &slv, spic::MNASystemDC &system,
	const std::filesystem::path &output_dir, const std::string &output_dir_str,
	const std::filesystem::path &cir_file, const std::string &cir_file_str,
	Logger &logger)
{
	// Keep a copy of the MNA matrix to check the residual
	Eigen::MatrixXd A_cpy = system.A;

	// SOlve MNA system on the operating point
	if (!slv.decompose()) {
		logger.log(ERROR, "Exiting due to non-SPD MNA system");
		exit(EXIT_FAILURE);
	}
	slv.solve(system.b);

	/*TODO: Since we keep a copy for the residual maybe we should do it internally
	 * and use it for making decompose perform LU after failed cholesky
	 * (maybe also consider out of place)
	 */
	logger.log(INFO, "Residual: " + std::to_string((A_cpy*system.x - system.b).norm()));

	// Delete output_dir if it exists and then create it again
	if (std::filesystem::exists(output_dir)) {
		logger.log(WARNING, output_dir_str + " exists... removing it");
		std::filesystem::remove_all(output_dir);
	}
	logger.log(INFO, "Creating " + output_dir_str);
	std::filesystem::create_directories(output_dir);

	// Copy the circuit file used to the output directory
	logger.log(INFO, "Copying " + cir_file_str +  " file to " + output_dir_str);
	std::filesystem::copy(cir_file, output_dir);

	// Create the dc_op.dat file in the output directory
	std::ofstream file;
	file.open(output_dir/"dc_op.dat");
	for (auto &it : node_table.table) {
		if (it.first != "0") {
			file << it.first << " "
				 << std::setprecision(std::numeric_limits<double>::max_digits10)
				 << system.x[it.second - 1] << std::endl;
		}
	}
}