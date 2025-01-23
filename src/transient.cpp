#include <vector>
#include <cmath>
#include <cassert>
#include <ostream>
#include <set>

#include "transient.h"
#include "commands.h"
#include "solver.h"
#include "netlist.h"
#include "node_table.h"

namespace spic {
	/*******************************************************************/
	/*                 Routines for TransientSpec class               */
	/*******************************************************************/

	double TransientSpecs::eval(double t) {
		switch (type)
		{
		case EXP:
			return exp_eval(t);
		case SIN:
			return sin_eval(t);
		case PULSE:
			return pulse_eval(t);
		case PWL:
			return pwl_eval(t);
		}
		assert(0);
	}

	double TransientSpecs::exp_eval(double t)
	{
		if (t <= exp.td1){
			return exp.i1;
		} else if (t <= exp.td2) {
			return exp.i1 + exp.idiff * (1.0 - std::exp(-(t - exp.td1) / exp.tc1));
		} else {
			return exp.i1 + exp.idiff * (std::exp(-(t - exp.td2) / exp.tc2) - std::exp(-(t - exp.td1) / exp.tc1));
		}
	}

	double TransientSpecs::sin_eval(double t)
	{
		if (t <= sin.td) {
			return sin.i1 + sin.ia * std::sin(sin.initial_phase);
		} else {
			return sin.i1 + sin.ia * std::sin(sin.omega * (t - sin.td) + sin.initial_phase) * std::exp(-(t - sin.td) * sin.df);
		}
	}

	double TransientSpecs::pulse_eval(double t)
	{
		// Get modulo to think only in terms of a single period
		double t_rem = std::fmod(t, pulse.per);

		if (t_rem <= pulse.td) {
			return pulse.i1;
		} else if (t_rem <= pulse.peak) {
			return pulse.i1 + (pulse.diff * (t_rem - pulse.td)) / pulse.tr;
		} else if (t_rem <= pulse.fall_start) {
			return pulse.i2;
		} else if (t_rem <= pulse.fall_end) {
			return pulse.i2 - (pulse.diff * (t_rem - pulse.fall_start)) / pulse.tf;
		} else {
			return pulse.i1;
		}
	}

	double TransientSpecs::pwl_eval(double t) {
		// Case of t before first point
		if (t < (*pwl.points)[0].first) {
			return (*pwl.points)[0].second;
		}

		// Case of t between points
		for (int i = 0; i < pwl.points->size() - 1; i++) {
			if (t >= (*pwl.points)[i].first && t < (*pwl.points)[i + 1].first) {
				return (*pwl.points)[i].second + pwl.slopes[i] * (t - (*pwl.points)[i].first);
			}
		}

		// Case of t after the last point (no need to check)
		return (*pwl.points)[pwl.points->size() - 1].second;
	}

	/******************************************************************/
	/*              Routines for TransientAnalysis class              */
	/******************************************************************/

	/* Main routine for executing a Transient Analysis */
	void TransientAnalysis::run(Solver &solver,
								MNASystemTransient &tran_mna_system,
								std::vector<std::string> &prints,
								std::vector<std::string> &plots,
								std::filesystem::path transient_dir,
								Logger &logger)
	{
		logger.log(INFO, "Running Transient Analysis for " + std::to_string(fin_time) + " seconds with time step " + std::to_string(time_step) + " seconds.");

		/* Initialize variables */

		int steps = fin_time / time_step;
		Eigen::VectorXd *curr_source_vector_ptr = new Eigen::VectorXd(tran_mna_system.mna_system.n);
		curr_source_vector_ptr->setZero();

		/* Create vectors for keeping the output data */

		// Create a set to store unique elements from prints and plots
		std::set<std::string> unique_elements(prints.begin(), prints.end());
		unique_elements.insert(plots.begin(), plots.end());
		std::vector<std::string> unique_vector(unique_elements.begin(), unique_elements.end());

		std::unordered_map<std::string, std::vector<double>> transient_data;
		std::vector<double> transient_times;

		// Init vector of vectors
		for (auto &print_node : unique_vector) {
			transient_data[print_node] = std::vector<double>();
		}

		/* Start the Transient Analysis */

		// Find the transient source vector for the initial time which is 0.0
		calculate_source_vector(*curr_source_vector_ptr, tran_mna_system.mna_system.total_nodes, 0.0);

		// Solve the MNA system for the initial time
		solver.analyze();
		solver.solve(*curr_source_vector_ptr);

		Eigen::VectorXd *prev_source_vector_ptr = nullptr;
		if (commands.options.transient_method == TR) {
			prev_source_vector_ptr = new Eigen::VectorXd(*curr_source_vector_ptr);
		}

		// Create the new transient A matrix and analyze it
		tran_mna_system.create_tran_system(time_step);
		solver.analyze();

		// Run the transient analysis
		for (int k = 1; k <= steps; k++) {
			transient_times.push_back(k * time_step);

			// Calculate the source vector for the current time
			calculate_source_vector(*curr_source_vector_ptr, tran_mna_system.mna_system.total_nodes, transient_times[k-1]);


			// Update the system's b vector
			if (commands.options.transient_method == BE) {
				tran_mna_system.update_tran_system_be(*curr_source_vector_ptr, time_step);
			} else {
				tran_mna_system.update_tran_system_tr(*curr_source_vector_ptr, *prev_source_vector_ptr, time_step);
				std::swap<Eigen::VectorXd*>(curr_source_vector_ptr, prev_source_vector_ptr);
			}

			// Solve the system
			solver.solve(tran_mna_system.mna_system.b);

			// Store the results for the print nodes
			for (auto &print_node : unique_vector) {
				int node_id = node_table.find_node(&print_node) - 1;
				transient_data[print_node].push_back(tran_mna_system.mna_system.x(node_id));
			}
		}

		// Dump the Transient Analysis results to files
		dump_results(transient_data, transient_times, unique_vector, transient_dir);

		// Use gnuplot to plot the Transient Analysis results for the plot nodes
		plot_results(plots, logger, transient_dir);

		delete prev_source_vector_ptr;
		delete curr_source_vector_ptr;
	}

	/* Calculate the source vector for the current time */
	void TransientAnalysis::calculate_source_vector(Eigen::VectorXd &source_vector, int total_nodes, double time)
	{
		// Set Source Vector to zero
		source_vector.setZero();

		// Add the current sources transient stamp to the source vector
		for (auto &source : netlist.current_sources.elements) {
			double value = source.eval(time);
			int node_positive = source.node_positive;
			int node_negative = source.node_negative;

			if (node_positive > 0) {
				source_vector[node_positive - 1] -= value;
			}
			if (node_negative > 0) {
				source_vector[node_negative - 1] += value;
			}
		}

		// Add the voltage sources transient stamp to the source vector
		int total_voltage_sources = netlist.voltage_sources.size();
		for (int i = 0; i < total_voltage_sources; i++) {
			auto &source = netlist.voltage_sources.elements[i];
			double value = source.eval(time);
			source_vector[total_nodes - 1 + i] = value;
		}
	}

	std::string TransientAnalysis::get_transient_name(std::string print_node)
	{
		std::string step_str = std::to_string(time_step);
		step_str.erase(step_str.find_last_not_of('0') + 1, std::string::npos);
		step_str.erase(step_str.find_last_not_of('.') + 1, std::string::npos);

		std::string fin_str = std::to_string(fin_time);
		fin_str.erase(fin_str.find_last_not_of('0') + 1, std::string::npos);
		fin_str.erase(fin_str.find_last_not_of('.') + 1, std::string::npos);

		return "tran_" + step_str + "_" + fin_str + "_V(" + print_node + ").dat";
	}

		/* Routine that prints dc_sweep results */
	void TransientAnalysis::dump_results(std::unordered_map<std::string, std::vector<double>> transient_data,
										std::vector<double>                                  transient_times,
										std::vector<std::string>                             unique_vector,
										std::filesystem::path                                transient_dir)
	{
		std::ofstream file;
		for (auto &print_node : unique_vector) {
			file.open(transient_dir/get_transient_name(print_node));
			if (!file.is_open()) {
				throw std::runtime_error("Unable to open file for writing: " + (transient_dir/get_transient_name(print_node)).string());
			}
			for (int i = 0; i < transient_times.size(); i++) {
				file << transient_times[i] << " " << transient_data[print_node][i] << std::endl;
			}
			file.close();
		}
	}

	/* Routine that plots dc_sweep results */
	void TransientAnalysis::plot_results(std::vector<std::string> &plots,
										Logger                   &logger,
										std::filesystem::path    transient_dir)
	{
		for (auto &plot_node : plots) {
			std::string plot_file = transient_dir/get_transient_name(plot_node);
			std::string image_file = plot_file;
			image_file.erase(plot_file.find(".dat"), std::string::npos);
			std::string plot_command = "gnuplot -e \"set terminal png; set output '" + image_file + ".png'; plot '" + plot_file + "' with lines title 'V(" + plot_node + ") vs time\"";
			logger.log(INFO, plot_command);
			std::system(plot_command.c_str());
		}
	}
}


std::ostream& operator<<(std::ostream &out, const spic::TransientSpecs &transient_specs)
{
	switch (transient_specs.type) {
		case spic::TransientSpecs::EXP:
			out << "EXP: i1=" << transient_specs.exp.i1 << ", i2=" << transient_specs.exp.i2
				<< ", td1=" << transient_specs.exp.td1 << ", tc1=" << transient_specs.exp.tc1
				<< ", td2=" << transient_specs.exp.td1 << ", tc2=" << transient_specs.exp.tc2;
			break;
		case spic::TransientSpecs::SIN:
			out << "SIN: i1=" << transient_specs.sin.i1 << ", ia=" << transient_specs.sin.ia
				<< ", fr=" << transient_specs.sin.fr << ", td=" << transient_specs.sin.td
				<< ", df=" << transient_specs.sin.df << ", ph=" << transient_specs.sin.ph;
			break;
		case spic::TransientSpecs::PULSE:
			out << "PULSE: i1=" << transient_specs.pulse.i1 << ", i2=" << transient_specs.pulse.i2
				<< ", td=" << transient_specs.pulse.td << ", tr=" << transient_specs.pulse.tr
				<< ", tf=" << transient_specs.pulse.tf << ", pw=" << transient_specs.pulse.pw
				<< ", per=" << transient_specs.pulse.per;
			break;
		case spic::TransientSpecs::PWL:
			out << "PWL: points=";
			for (const auto &point : *transient_specs.pwl.points) {
				out << "(" << point.first << ", " << point.second << ") ";
			}
			break;
		default:
			out << "Unknown transient type";
			break;
	}
	return out;
}

std::ostream& operator<<(std::ostream &out, const spic::TransientAnalysis &transient_analysis)
{
	out << transient_analysis.time_step << " " << transient_analysis.fin_time << std::endl;
	return out;
}

std::ostream& operator<<(std::ostream &out, const std::vector<spic::TransientAnalysis> &transient_list)
{
	out << "\t" << "Transient Analysis List (step, finish):" << std::endl;
	for (const auto &ta : transient_list) {
		out << "\t\t * " << ta << std::endl;
	}
	return out;
}