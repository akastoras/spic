#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include <Eigen/Dense>

#include "util.h"
#include "solver.h"
#include "netlist.h"


namespace spic {
	class DCSweep {
		public:
		using type_t = enum {V,I};
		type_t type;
		std::string &source_name;
		double start_value; // Starting value of sweep
		double end_value;   // End value of sweep
		double step;

		std::string get_dc_sweep_name(std::string print_node);
		DCSweep(type_t type, std::string &source_name, double start_value, double end_value, double step)
			: type(type), source_name(source_name), start_value(start_value), end_value(end_value), step(step) {}
	
		void sweep(Solver *solver, std::vector<std::string> &prints, std::vector<std::string> &plots);
	};
	
	class Commands {
		public:
		options_t options;
		std::vector<DCSweep> v_dc_sweeps;
		std::vector<DCSweep> i_dc_sweeps;
		std::filesystem::path dc_sweeps_dir;
		std::vector<std::string> print_nodes;
		std::vector<std::string> plot_nodes;


		Commands() { options = {0}; }
		bool add_v_dc_sweep(std::string &source_name, double start_value, double end_value, double step);
		bool add_i_dc_sweep(std::string &source_name, double start_value, double end_value, double step);
		void perform_dc_sweeps(Solver *solver, Logger &logger);
	};
}

std::ostream& operator<<(std::ostream &out, const spic::DCSweep dc_sweep);
std::ostream& operator<<(std::ostream &out, const std::vector<spic::DCSweep> dc_sweeps);
std::ostream& operator<<(std::ostream &out, const spic::Commands &commands);

extern spic::Commands commands;