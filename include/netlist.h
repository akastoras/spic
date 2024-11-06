#pragma once

#include <string>
#include <list>

#include "node_table.h"

namespace spic {
	/* Class to be inherited from by elements with 2 nodes */
	class Element2nodes {
		public:
		std::string *name;
		node_id_t node_positive;
		node_id_t node_negative;
		float value;

		/* Voltage Source Constructor */
		Element2nodes(std::string *str, int node1, int node2, float val):
			name(str), node_positive(node1), node_negative(node2), value(val) { }
		~Element2nodes() {
			delete name;
		}
	};

	class VoltageSource : public Element2nodes {
		using Element2nodes::Element2nodes;
	};

	class CurrentSource : public Element2nodes {
		using Element2nodes::Element2nodes;
	};

	class Resistor : public Element2nodes {
		using Element2nodes::Element2nodes;
	};

	class Capacitor : public Element2nodes {
		using Element2nodes::Element2nodes;
	};
	
	class Inductor : public Element2nodes {
		using Element2nodes::Element2nodes;
	};

	class Diode {
		public:
		std::string *name;
		node_id_t node_positive;
		node_id_t node_negative;
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
		node_id_t drain;
		node_id_t gate;
		node_id_t source;
		node_id_t body;
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
		node_id_t collector;
		node_id_t base;
		node_id_t emitter;
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
	
	/* Netlist class contains a list of pointers to each element type */
	class Netlist {
		public:
		std::list<VoltageSource *> voltage_sources;
		std::list<CurrentSource *> current_sources;
		std::list<Resistor *> resistors;
		std::list<Capacitor *> capacitors;
		std::list<Inductor *> inductors;
		std::list<Diode *> diodes;
		std::list<MOS *> mos_transistors;
		std::list<BJT *> bj_transistors;

		Netlist() { }
		~Netlist() {
			for (auto it = voltage_sources.begin(); it != voltage_sources.end(); ++it) {
				delete *it;
			}
			for (auto it = current_sources.begin(); it != current_sources.end(); ++it) {
				delete *it;
			}
			for (auto it = resistors.begin(); it != resistors.end(); ++it) {
				delete *it;
			}
			for (auto it = capacitors.begin(); it != capacitors.end(); ++it) {
				delete *it;
			}
			for (auto it = inductors.begin(); it != inductors.end(); ++it) {
				delete *it;
			}
			for (auto it = diodes.begin(); it != diodes.end(); ++it) {
				delete *it;
			}
			for (auto it = mos_transistors.begin(); it != mos_transistors.end(); ++it) {
				delete *it;
			}
		}

		/* Functions to append an element to the relevant list */
		void add_voltage_source(VoltageSource *v);
		void add_current_source(CurrentSource *i);
		void add_resistor(Resistor *r);
		void add_capacitor(Capacitor *c);
		void add_inductor(Inductor *l);
		void add_diode(Diode *d);
		void add_mos(MOS *m);
		void add_bjt(BJT *q);
	};
}

/* Support of << operator for printing circuit elements */
std::ostream& operator<<(std::ostream &out, spic::VoltageSource &v);
std::ostream& operator<<(std::ostream &out, spic::CurrentSource &i);
std::ostream& operator<<(std::ostream &out, spic::Resistor &r);
std::ostream& operator<<(std::ostream &out, spic::Capacitor &c);
std::ostream& operator<<(std::ostream &out, spic::Inductor &l);
std::ostream& operator<<(std::ostream &out, spic::Diode &d);
std::ostream& operator<<(std::ostream &out, spic::MOS &m);
std::ostream& operator<<(std::ostream &out, spic::BJT &q);
std::ostream& operator<<(std::ostream &out, spic::Netlist &nl);