
#include <vector>
#include <string>


#include "commands.h"
#include "netlist.h"
#include "node_table.h"
#include "system.h"

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

	void DCSweep::sweep(Solver &solver, std::vector<std::string> &prints, std::vector<std::string> &plots) {
		Eigen::VectorXd b_new = solver.system.b;

		if (type == V) {
			int index_in_v_sources = netlist.voltage_sources.find_element_name(source_name);
			int index_in_b = (node_table.size()-1) + index_in_v_sources;
			std::cout << index_in_b << std::endl;
			b_new(index_in_b) = start_value;
			

			for (; b_new(index_in_b) <= end_value; b_new(index_in_b) += step) {
				solver.solve(b_new);
				
				// std::cout << solver.system;
				// std::cout << b_new << std::endl;
				for (auto &node : prints) {
					int index_in_node = node_table.find_node(&node);
					// std::cout << node << " -> " << index_in_node << std::endl;
					std::cout << "Node " << node << " = " << solver.system.x(index_in_node) << std::endl;
				}
			}
		}
	}

	void Commands::perform_dc_sweeps(Solver &solver) {
		for (auto &s : v_dc_sweeps) {
			s.sweep(solver, print_nodes, plot_nodes);
		}
	}
}

std::ostream& operator<<(std::ostream &out, const spic::options_t &options) {
	out << "\tCustom: " << (options.custom ? "Enabled" : "Disabled") << std::endl;
	out << "\tSPD: " << (options.spd ? "Enabled" : "Disabled") << std::endl;
	return out;
}

std::ostream& operator<<(std::ostream &out, const spic::DCSweep dc_sweep) {
	out << (dc_sweep.type == spic::DCSweep::V ? "V" : "I")  << dc_sweep.source_name << " "
				<< dc_sweep.start_value << " "
				<< dc_sweep.end_value   << " "
				<< dc_sweep.step		<< std::endl;
	return out;
}

std::ostream& operator<<(std::ostream &out, const std::vector<spic::DCSweep> dc_sweeps) {
	out << "\t" << (dc_sweeps[0].type == spic::DCSweep::V ? "Voltage" : "Current") << " DC Sweeps:" << std::endl;
	for (const auto &dc_sweep : dc_sweeps) {
		out << "\t\t * " << dc_sweep;
	}
	return out;
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