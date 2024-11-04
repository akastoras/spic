#pragma once

#include <string>
#include <list>

namespace spic {
	class VoltageSource {
		public:
		std::string *name;
		int node_positive;
		int node_negative;
		float value;

		/* Voltage Source Constructor */
		VoltageSource(std::string *str, int node1, int node2, float val):
			name(str), node_positive(node1), node_negative(node2), value(val) { }
		~VoltageSource() {
			delete name;
		}

	};
	// More classes
	
	class Netlist {
		public:
		std::list<VoltageSource *> voltage_sources;
		// More lists

		Netlist() { }
		void add_voltage_source(VoltageSource *v);
	};
}

std::ostream& operator<<(std::ostream &out, spic::VoltageSource &v);
std::ostream& operator<<(std::ostream &out, spic::Netlist &nl);