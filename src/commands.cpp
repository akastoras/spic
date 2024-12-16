
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

	// DCSweep::get_dc_sweep_name function creates the DC Sweep filename for a print node
	std::string DCSweep::get_dc_sweep_name(std::string print_node)
	{
		std::string start_value_str = std::to_string(start_value);
		start_value_str.erase(start_value_str.find_last_not_of('0') + 1, std::string::npos);
		start_value_str.erase(start_value_str.find_last_not_of('.') + 1, std::string::npos);

		std::string end_value_str = std::to_string(end_value);
		end_value_str.erase(end_value_str.find_last_not_of('0') + 1, std::string::npos);
		end_value_str.erase(end_value_str.find_last_not_of('.') + 1, std::string::npos);

		std::string step_str = std::to_string(step);
		step_str.erase(step_str.find_last_not_of('0') + 1, std::string::npos);
		step_str.erase(step_str.find_last_not_of('.') + 1, std::string::npos);
		
		return ((type == V) ? "V" : "I")
				+ source_name + "_"
				+ start_value_str + "_"
				+ end_value_str + "_"
				+ step_str
				+ "_V(" + print_node + ").dat";
	}

	// DCSweep::sweep function performs all the DC Sweeps, gets the results for the print nodes
	// and stores them in corresponding files
	void DCSweep::sweep(Solver &solver, std::vector<std::string> &prints, std::vector<std::string> &plots) {
		Eigen::VectorXd b_new = solver.system.b;
		int voltage_src_id, matrix_src_id, current_src_id, current_pos_node, current_neg_node;
		double current_src_value;

		// Create a set to store unique elements from prints and plots
		std::set<std::string> unique_elements(prints.begin(), prints.end());
		unique_elements.insert(plots.begin(), plots.end());

		// Convert the set back to a vector
		std::vector<std::string> unique_vector(unique_elements.begin(), unique_elements.end());

		// Create updated version of b with the DC Sweep initial parameters
		if (type == V) {
			voltage_src_id = netlist.voltage_sources.find_element_name(source_name);
			matrix_src_id = (node_table.size()-1) + voltage_src_id;
			b_new(matrix_src_id) = start_value;
		} else { // type == I
			current_src_id = netlist.current_sources.find_element_name(source_name);
			current_pos_node = netlist.current_sources.elements[current_src_id].node_positive;
			current_neg_node = netlist.current_sources.elements[current_src_id].node_negative;
			current_src_value = netlist.current_sources.elements[current_src_id].value;
			
			// Remove old current source stamp from b_new
			if (current_pos_node > 0) {
				b_new(current_pos_node - 1) += current_src_value;
			}
			if (current_neg_node > 0) {
				b_new(current_neg_node - 1) -= current_src_value;
			}
			
			// Add new current source stamp to b_new
			if (current_pos_node > 0) {
				b_new(current_pos_node - 1) -= start_value;
			}
			if (current_neg_node > 0) {
				b_new(current_neg_node - 1) += start_value;
			}
		}

		// Init vector of vectors
		std::unordered_map<std::string, std::vector<double>> dc_sweep_data;
		for (auto &print_node : unique_vector) {
			dc_sweep_data[print_node] = std::vector<double>();
		}
		std::vector<double> dc_sweep_src;

		// Produce the DC Sweep results for all print nodes
		double src_value = start_value;

		const auto relative_difference_factor = 0.0001;    // 0.01%
		const auto greater_magnitude = std::max(std::abs(src_value),std::abs(end_value));
		const auto tolerance = greater_magnitude * relative_difference_factor;

		while (src_value < end_value || std::abs(src_value - end_value) < tolerance) {
			// Store the source value for the current iteration
			dc_sweep_src.push_back(src_value);

			// Solve the system and keep the results for the print nodes
			solver.solve(b_new);
			for (auto &print_node : unique_vector) {
				int node_id = node_table.find_node(&print_node) - 1;
				dc_sweep_data[print_node].push_back(solver.system.x(node_id));
			}

			// Update the source value for the next iteration
			if (type == V) {
				b_new(matrix_src_id) += step;
			} else { // type == I
				if (current_pos_node > 0) {
					b_new(current_pos_node - 1) -= step;
				}
				if (current_neg_node > 0) {
					b_new(current_neg_node - 1) += step;
				}
			}
			src_value += step;
		}

		// Write the DC Sweep results to files
		std::ofstream file;
		for (auto &print_node : unique_vector) {
			file.open(commands.dc_sweeps_dir/get_dc_sweep_name(print_node));
			for (int i = 0; i < dc_sweep_src.size(); i++) {
				file << dc_sweep_src[i] << " " << dc_sweep_data[print_node][i] << std::endl;
			}
			file.close();
		}

		// Use gnuplot to plot the DC Sweep results for the plot nodes
		for (auto &plot_node : plots) {
			std::string plot_file = commands.dc_sweeps_dir/get_dc_sweep_name(plot_node);
			std::string image_file = plot_file;
			image_file.erase(plot_file.find(".dat"), std::string::npos);
			std::string plot_command = "gnuplot -e \"set terminal png; set output '" + image_file + ".png'; plot '" + plot_file + "' with lines title 'V(" + plot_node + ") vs " + source_name + "\"";
			solver.logger.log(INFO, plot_command);
			std::system(plot_command.c_str());
		}
	}

	// Commands::perform_dc_sweeps function performs all the DC Sweeps
	void Commands::perform_dc_sweeps(Solver &solver, Logger &logger) {
		// Delete dc_sweeps_dir if it exists and then create it again
		if (std::filesystem::exists(dc_sweeps_dir)) {
			logger.log(WARNING, std::string(dc_sweeps_dir) + " exists... removing it");
			std::filesystem::remove_all(dc_sweeps_dir);
		}
		logger.log(INFO, "Creating " + std::string(dc_sweeps_dir));
		std::filesystem::create_directories(dc_sweeps_dir);

		// Perform the DC Voltage Sweeps
		for (auto &s : v_dc_sweeps) {
			s.sweep(solver, print_nodes, plot_nodes);
		}
		// Perform the DC Current Sweeps
		for (auto &s : i_dc_sweeps) {
			s.sweep(solver, print_nodes, plot_nodes);
		}
	}
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