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

	class CurrentSource {
		public:
		std::string *name;
		int node_positive;
		int node_negative;
		float value;

		/* Current Source Constructor */
		CurrentSource(std::string *str, int node1, int node2, float val):
			name(str), node_positive(node1), node_negative(node2), value(val) { }
		~CurrentSource() {
			delete name;
		}
	};

	class Resistor {
		public:
		std::string *name;
		int node_positive;
		int node_negative;
		float value;

		/* Resistor Constructor */
		Resistor(std::string *str, int node1, int node2, float val):
			name(str), node_positive(node1), node_negative(node2), value(val) { }
		~Resistor() {
			delete name;
		}
	};

	class Capacitor {
		public:
		std::string *name;
		int node_positive;
		int node_negative;
		float value;

		/* Capacitor Constructor */
		Capacitor(std::string *str, int node1, int node2, float val):
			name(str), node_positive(node1), node_negative(node2), value(val) { }
		~Capacitor() {
			delete name;
		}
	};	
	
	class Load {
		public:
		std::string *name;
		int node_positive;
		int node_negative;
		float value;

		/* Load Constructor */
		Load(std::string *str, int node1, int node2, float val):
			name(str), node_positive(node1), node_negative(node2), value(val) { }
		~Load() {
			delete name;
		}
	};

	class Diode {
		public:
		std::string *name;
		int node_positive;
		int node_negative;
		std::string *model;
		float area_factor;

		/* Diode Constructor */
		Diode(std::string *str, int node1, int node2, std::string *file, float val):
			name(str), node_positive(node1), node_negative(node2), model(file), area_factor(val) { }
		~Diode() {
			delete name;
			delete model;
		}
	};
	
	class MOS {
		public:
		std::string *name;
		int drain;
		int gate;
		int source;
		int body;
		std::string *model;
		float length;
		float width;

		/* MOS Constructor */
		MOS(std::string *str, int node1, int node2, int node3, int node4, std::string *file, float val1, float val2):
			name(str), drain(node1), gate(node2), source(node3), body(node4), model(file), length(val1), width(val2) { }
		~MOS() {
			delete name;
			delete model;
		}
	};

	class BJT {
		public:
		std::string *name;
		int collector;
		int base;
		int emitter;
		std::string *model;
		float area_factor;

		/* BJT Constructor */
		BJT(std::string *str, int node1, int node2, int node3, std::string *file, float val):
			name(str), collector(node1), base(node2), emitter(node3), model(file), area_factor(val) { }
		~BJT() {
			delete name;
			delete model;
		}
	};
	
	
	class Netlist {
		public:
		std::list<VoltageSource *> voltage_sources;
		std::list<CurrentSource *> current_sources;
		std::list<Resistor *> resistors;
		std::list<Capacitor *> capacitors;
		std::list<Load *> loads;
		std::list<Diode *> diodes;
		std::list<MOS *> mos_transistors;
		std::list<BJT *> bj_transistors;

		Netlist() { }

		void add_voltage_source(VoltageSource *v);
		void add_current_source(CurrentSource *i);
		void add_resistor(Resistor *r);
		void add_capacitor(Capacitor *c);
		void add_load(Load *l);
		void add_diode(Diode *d);
		void add_mos(MOS *m);
		void add_bjt(BJT *q);

	};
}

std::ostream& operator<<(std::ostream &out, spic::VoltageSource &v);
std::ostream& operator<<(std::ostream &out, spic::CurrentSource &i);
std::ostream& operator<<(std::ostream &out, spic::Resistor &r);
std::ostream& operator<<(std::ostream &out, spic::Capacitor &c);
std::ostream& operator<<(std::ostream &out, spic::Load &l);
std::ostream& operator<<(std::ostream &out, spic::Diode &d);
std::ostream& operator<<(std::ostream &out, spic::MOS &m);
std::ostream& operator<<(std::ostream &out, spic::BJT &q);
std::ostream& operator<<(std::ostream &out, spic::Netlist &nl);