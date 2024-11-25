#pragma once

#include <string>
#include <vector>

namespace spic {
	typedef struct {
		bool custom; // Enable usage of custom implementations
		bool spd; // Enable cholesky decomp
	} options_t;

	class DCSweep {
		public:
		std::string &source_name;
		float start_value; // Starting value of sweep
		float end_value;   // End value of sweep
		float step;
	};

	class Commands {
		public:
		options_t options;
		std::vector<DCSweep> v_dc_sweeps;
		std::vector<DCSweep> i_dc_sweeps;
		
		Commands() { options = {0}; }
		bool add_v_dc_sweep(std::string &source_name, float start_value, float end_value, float step);
		bool add_i_dc_sweep(std::string &source_name, float start_value, float end_value, float step);
	};
}

extern spic::Commands commands;