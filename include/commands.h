#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include <Eigen/Dense>

#include "util.h"
#include "solver.h"
#include "netlist.h"
#include "dc_sweeps.h"
#include "transient.h"

namespace spic {
	class Commands {
		public:
		options_t options;
		std::vector<DCSweep> v_dc_sweeps;
		std::vector<DCSweep> i_dc_sweeps;
		std::filesystem::path dc_sweeps_dir;
		std::vector<TransientAnalysis> transient_list;
		std::filesystem::path transient_dir;
		std::vector<std::string> print_nodes;
		std::vector<std::string> plot_nodes;


		Commands() { options = {0}; }
		bool add_v_dc_sweep(std::string &source_name, double start_value, double end_value, double step);
		bool add_i_dc_sweep(std::string &source_name, double start_value, double end_value, double step);
		void perform_dc_sweeps(Solver *solver, Logger &logger);
		void perform_transients(Solver &solver, MNASystem &mna_system, Logger &logger);
		void perform_transients(Solver &solver, MNASparseSystem &mna_sp_system, Logger &logger);
	};
}
std::ostream& operator<<(std::ostream &out, const spic::Commands &commands);

extern spic::Commands commands;