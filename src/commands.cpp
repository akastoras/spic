#include "commands.h"
#include "netlist.h"

#include <vector>
#include <string>


namespace spic {
	// Commands::add_v_dc_sweep function creates a new dc_sweep for a voltage source
	bool Commands::add_v_dc_sweep(std::string &source_name, float start_value, float end_value, float step)
	{
		if (netlist.voltage_sources.find_element_name(source_name) != -1) {
			v_dc_sweeps.push_back(DCSweep(source_name, start_value, end_value, step));
			return true;
		}
		return false;
	}

	// Commands::add_i_dc_sweep function creates a new dc_sweep for a current source
	bool Commands::add_i_dc_sweep(std::string &source_name, float start_value, float end_value, float step)
	{
		if (netlist.current_sources.find_element_name(source_name) != -1) {
			i_dc_sweeps.push_back(DCSweep(source_name, start_value, end_value, step));
			return true;
		}
		return false;
	}
}