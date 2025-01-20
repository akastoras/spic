#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "solver.h"

namespace spic {
	class DCSweep {
		public:
		typedef enum {V,I} type_t;
		type_t type;
		std::string &source_name;
		double start_value; // Starting value of sweep
		double end_value;   // End value of sweep
		double step;

		std::string get_dc_sweep_name(std::string print_node);
		DCSweep(type_t type, std::string &source_name, double start_value, double end_value, double step)
			: type(type), source_name(source_name), start_value(start_value), end_value(end_value), step(step) {}

		void sweep(Solver                   *solver,
						std::vector<std::string> &prints,
						std::vector<std::string> &plots,
						std::filesystem::path    dc_sweeps_dir);

		private:
		void dump_dc_sweep_results(std::unordered_map<std::string, std::vector<double>> dc_sweep_data,
										std::vector<double>                                  dc_sweep_src,
										std::vector<std::string>                             unique_vector,
										std::filesystem::path                                dc_sweeps_dir);

		void plot_dc_sweep_results(std::vector<std::string> &plots,
										Logger                   &logger,
										std::filesystem::path    dc_sweeps_dir);
	};
}

std::ostream& operator<<(std::ostream &out, const spic::DCSweep dc_sweep);
std::ostream& operator<<(std::ostream &out, const std::vector<spic::DCSweep> dc_sweeps);