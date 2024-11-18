#include <iostream>
#include <Eigen/Core>

#include "netlist.h"

#include "mna.h"

namespace spic {
	MNASystemDC::MNASystemDC(Netlist &netlist, int total_nodes)
		: netlist(netlist), total_nodes(total_nodes)
	{
		int total_voltage_sources = netlist.voltage_sources.size();
		int total_inductors = netlist.inductors.size();
	}
}