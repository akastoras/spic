
#include <vector>
#include <string>
#include <filesystem>
#include <cstdio>
#include <set>

#include "commands.h"
#include "solver.h"
#include "netlist.h"
#include "node_table.h"
#include "system.h"
#include "sparse_system.h"
#include "util.h"

namespace spic {
	// Commands::add_v_dc_sweep function creates a new dc_sweep for a voltage source
	bool Commands::add_v_dc_sweep(std::string &source_name, double start_value, double end_value, double step)
	{
		if (netlist.voltage_sources.find_element_name(source_name) != -1) {
			v_dc_sweeps.push_back(DCSweep(spic::DCSweep::V, source_name, start_value, end_value, step));
			return true;
		}
		return false;
	}

	// Commands::add_i_dc_sweep function creates a new dc_sweep for a current source
	bool Commands::add_i_dc_sweep(std::string &source_name, double start_value, double end_value, double step)
	{
		if (netlist.current_sources.find_element_name(source_name) != -1) {
			i_dc_sweeps.push_back(DCSweep(spic::DCSweep::I, source_name, start_value, end_value, step));
			return true;
		}
		return false;
	}

	// Commands::perform_dc_sweeps function performs all the DC Sweeps
	void Commands::perform_dc_sweeps(Solver *solver, Logger &logger) {
		logger.log(INFO, "Performing DC Sweeps");
		
		// Create it the dc_sweeps_dir
		logger.log(INFO, "Creating " + std::string(dc_sweeps_dir));
		if (!std::filesystem::create_directories(dc_sweeps_dir)) {
			logger.log(ERROR, "Unable to create " + std::string(dc_sweeps_dir));
		}

		// Perform the DC Voltage Sweeps
		for (auto &s : v_dc_sweeps) {
			s.sweep(solver, print_nodes, plot_nodes, dc_sweeps_dir);
		}
		// Perform the DC Current Sweeps
		for (auto &s : i_dc_sweeps) {
			s.sweep(solver, print_nodes, plot_nodes, dc_sweeps_dir);
		}
	}

	// Commands::perform_dc_sweeps function performs all the Transient Analysises
	void Commands::perform_transients(Solver &solver, MNASystem &mna_system, Logger &logger)
	{
		logger.log(INFO, "Performing Transient Analysises");

		logger.log(INFO, "Creating " + std::string(transient_dir));
		if (!std::filesystem::create_directories(transient_dir)) {
			logger.log(ERROR, "Unable to create " + std::string(transient_dir));
		}

		MNASystemTransient tran_mna_system(commands.options.transient_method, mna_system);

		// Perform the Transient Analysises
		for (auto &t : transient_list) {
			t.load_system(&tran_mna_system);
			t.run(solver, print_nodes, plot_nodes, transient_dir, logger);
		}
	}

	// Commands::perform_dc_sweeps function performs all the Transient Analysises
	void Commands::perform_transients(Solver &solver, MNASparseSystem &mna_sparse_system, Logger &logger)
	{
		logger.log(INFO, "Performing Transient Analysises");

		// Create empty directory to store the transient analysis results
		logger.log(INFO, "Creating " + std::string(transient_dir));
		if (!std::filesystem::create_directories(transient_dir)) {
			logger.log(ERROR, "Unable to create " + std::string(transient_dir));
		}

		MNASparseSystemTransient tran_mna_sparse_system(commands.options.transient_method, mna_sparse_system);

		// Perform the Transient Analysises
		for (auto &t : transient_list) {
			t.load_system(&tran_mna_sparse_system);
			t.run(solver, print_nodes, plot_nodes, transient_dir, logger);
		}
	}
}

std::ostream& operator<<(std::ostream &out, const spic::Commands &commands) {
	out << "The following spic Commands are specified:\n";
	out << commands.options;
	
	if (!commands.v_dc_sweeps.empty()) {
		out << commands.v_dc_sweeps;
	}
	if (!commands.i_dc_sweeps.empty()) {
		out << commands.i_dc_sweeps;
	}

	if (!commands.transient_list.empty()) {
		out << commands.transient_list;
	}

	if (!commands.print_nodes.empty()) {
		out << "\tPrint Nodes:\n";
		for (const auto &node : commands.print_nodes) {
			out << "\t\t * V(" << node << ")" << std::endl;
		}
	}
	
	if (!commands.plot_nodes.empty()) {
		out << "\tPlot Nodes:\n";
		for (const auto &node : commands.plot_nodes) {
			out << "\t\t * V(" << node << ")" << std::endl;
		}
	}

	return out;
}