#include <ostream>

#include "netlist.h"
#include "node_table.h"

extern spic::NodeTable *node_table;

namespace spic {
	void Netlist::add_voltage_source(VoltageSource *v) {
		voltage_sources.push_back(v);
	}
}

/* Support of << operator for printing a NodeTable */
std::ostream& operator<<(std::ostream &out, spic::Netlist &nl) {
	for (auto it = nl.voltage_sources.begin(); it != nl.voltage_sources.end(); ++it) {
		out << **it << std::endl;
	}
	// and so on... 
	return out;
}

std::ostream& operator<<(std::ostream &out, spic::VoltageSource &v) {
	out << "Voltage Source:\n\tName: \'" << *v.name << "\'" << std::endl;
	out << "\t+ Node: " << v.node_positive << " (" << node_table->get_node_name(v.node_positive) << ")" << std::endl;
	out << "\t- Node: " << v.node_negative << " (" << node_table->get_node_name(v.node_negative) << ")" << std::endl;
	out << "\tValue: " << v.value << std::endl;
	return out;
}
