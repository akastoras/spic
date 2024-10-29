#include <string>
#include <list>

namespace spic {
	class VoltageSource {
		std::string *name;
		int node_positive;
		int node_negative;
		float value;
	};
	
	class Netlist {
		std::list<VoltageSource *> voltage_sources;
	};

	struct Element {
		enum { VSOURCE } type;
		
		union {
			VoltageSource *v;
		};
	};
}