#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <sstream>

#include <Eigen/Core>

#include <boost/program_options.hpp>

#include <omp.h>

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

/* Wrapper functions for spic stages of execution */
void parse_arguments(po::variables_map &vm, int argc, char** argv);
void parse_spice_file(std::filesystem::path cir_file, Logger &logger);
void solve_operating_point(spic::Solver &slv, spic::MNASystemDC &system,
	const std::filesystem::path &output_dir, const std::string &output_dir_str,
	const std::filesystem::path &cir_file, const std::string &cir_file_str,
	bool bypass_options, Logger &logger);

int main(int argc, char** argv)
{
	// and average of solve. Also the parsing and the MNA construction
	// Print all times and a total timer at the end of the program
	double g_timer_start = omp_get_wtime();
	Logger logger = Logger(std::cout);

	// Define the supported options
	po::variables_map vm;
	parse_arguments(vm, argc, argv);

	// Get the parsed arguments
	bool bypass_options = vm["bypass_options"].as<bool>();
	bool disable_dc_sweeps = vm["disable_dc_sweeps"].as<bool>();
	std::string cir_file_str = vm["cir_file"].as<std::string>();
	std::string output_dir_str = vm["output_dir"].as<std::string>();

	std::filesystem::path cir_file(cir_file_str);
	std::filesystem::path output_dir(output_dir_str);

	// Parse the spice circuit file that constructs the netlist
	// the node_table and the commands structures
	parse_spice_file(cir_file, logger);

	// Check if the user want to bypass the .cir options from spic
	if (bypass_options) {
		logger.log(INFO, "Bypassing .OPTIONS");
		commands.options.spd = vm["spd"].as<bool>();
		commands.options.custom = vm["custom"].as<bool>();
		commands.options.sparse = vm["sparse"].as<bool>();
		commands.options.iter = vm["iter"].as<bool>();
		commands.options.itol = vm["itol"].as<double>();
	}

	// Show final commands
	std::cout << commands;

	// Initialize Parallelism in Eigen
	int max_threads = omp_get_max_threads();
	logger.log(INFO, "Using " + std::to_string(max_threads) + " threads.");
	Eigen::setNbThreads(max_threads);

	// Construct MNA System
	logger.log(INFO, "Constructing MNA System for DC analysis.");
	spic::MNASystemDC system = spic::MNASystemDC(netlist, node_table.size());

	// Construct a Solver object
	spic::Solver slv = spic::Solver(system, commands.options, logger);

	// Solve on the operating point
	solve_operating_point(slv, system, output_dir, output_dir_str,
						cir_file, cir_file_str, bypass_options, logger);

	// Perform the dc sweeps (if there are any)
	if (!disable_dc_sweeps) {
		commands.dc_sweeps_dir = output_dir/"dc_sweeps";
		commands.perform_dc_sweeps(slv, logger);
	}

	// Performance Counters
	logger.log(INFO, "Dumping performance report.");
	std::filesystem::path perf_rpt = output_dir/"spic_performance.rpt";
	double g_total_time = omp_get_wtime() - g_timer_start;
	logger.log(INFO, "spic execution time was " + std::to_string(g_total_time));
	slv.dump_perf_counters(perf_rpt, g_total_time);

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
							"Output directory (default: <filename>_golden)")
		("bypass_options", po::bool_switch()->default_value(false), "Bypass .cir file options")
		("disable_dc_sweeps", po::bool_switch()->default_value(false), "Disable DC Sweeps")
		("spd", po::bool_switch()->default_value(false), "Enable SPD option")
		("custom", po::bool_switch()->default_value(false), "Enable custom solver option")
		("sparse", po::bool_switch()->default_value(false), "Enable sparse solver option")
		("iter", po::bool_switch()->default_value(false), "Enable iterative solver option")
		("itol", po::value<double>()->default_value(1e-3), "Set iteration tolerance");
		
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
}

void solve_operating_point(spic::Solver &slv, spic::MNASystemDC &system,
	const std::filesystem::path &output_dir, const std::string &output_dir_str,
	const std::filesystem::path &cir_file, const std::string &cir_file_str,
	bool bypass_options, Logger &logger)
{
	// Solve MNA system on the operating point
	slv.solve(system.b);

	// Delete output_dir if it exists and then create it again
	if (std::filesystem::exists(output_dir)) {
		logger.log(WARNING, output_dir_str + " exists... removing it");
		std::filesystem::remove_all(output_dir);
	}
	logger.log(INFO, "Creating " + output_dir_str);
	std::filesystem::create_directories(output_dir);

	// Copy the circuit file used to the output directory
	logger.log(INFO, "Copying " + cir_file_str +  " file to " + output_dir_str);
	
	if (!bypass_options) {
		std::filesystem::copy(cir_file, output_dir);
	} else {
		// Get all lines apart from the .OPTIONS lines
		std::ifstream input_file(cir_file);
		if (!input_file.is_open()) {
			throw std::runtime_error("Unable to open file: " + cir_file_str);
		}

		std::vector<std::string> lines;
		std::string line;
		while (std::getline(input_file, line)) {
			if (line.find(".OPTIONS") == std::string::npos) {
				lines.push_back(line);
			}
		}
		input_file.close();

		// Write the modified content to a new file in the output directory
		std::filesystem::path new_cir_file = output_dir / cir_file.filename();
		std::ofstream out_file(new_cir_file);
		if (!out_file.is_open()) {
			throw std::runtime_error("Unable to open file for writing: " + new_cir_file.string());
		}

		for (const auto& line : lines) {
			out_file << line << std::endl;
		}

		// Append the user's options to the end of the new file
		std::string user_options = std::string(".OPTIONS")
								+ std::string((commands.options.spd ? " SPD" : ""))
								+ std::string(commands.options.custom ? " CUSTOM" : "")
								+ std::string(commands.options.iter ? " ITER" : "")
								+ std::string(" ITOL=") + std::to_string(commands.options.itol);
		out_file << user_options << std::endl;
		out_file.close();
	}

	// Create the dc_op.dat file in the output directory
	std::ofstream file;
	file.open(output_dir/"dc_op.dat");

	file << "Node Voltage" << std::endl;
	for (auto &it : node_table.table) {
		if (it.first != "0") {
			file << it.first << " "
				 << std::setprecision(std::numeric_limits<double>::max_digits10)
				 << system.x[it.second - 1] << std::endl;
		}
	}

	int total_voltage_sources = netlist.voltage_sources.size();
	int total_inductors = netlist.inductors.size();
	int total_nodes = node_table.size();

	file << std::endl << "Source Current" << std::endl;
	for (int i = 0; i < total_voltage_sources; i++) {
		file << "V" << netlist.voltage_sources.elements[i].name
			 << " " << system.x[total_nodes - 1 + i] << std::endl;
	}

	for (int i = 0; i < total_inductors; i++) {
		file << "L" << netlist.inductors.elements[i].name
			 << " " << system.x[total_nodes - 1 + total_voltage_sources + i] << std::endl;
	}

	file.close();
}