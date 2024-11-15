#pragma once

#include <string>
#include <vector>

#include "node_table.h"

namespace spic {
	using element_id_t = int;

	class Element {
		public:
		std::string name;

		// Define == operator
		bool operator==(const Element &other) const {
			return name == other.name;
		}

		Element(std::string *str) : name(*str) {}
		~Element() {}
	};

	/* Class to be inherited from by elements with 2 nodes */
	class Element2nodes : public Element {
		public:
		node_id_t node_positive;
		node_id_t node_negative;
		float value;

		/* Voltage Source Constructor */
		Element2nodes(std::string *str, int node1, int node2, float val):
			Element(str), node_positive(node1), node_negative(node2), value(val) { }
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

	class Diode : public Element {
		public:
		node_id_t node_positive;
		node_id_t node_negative;
		std::string model;
		float area_factor;

		/* Diode Constructor */
		Diode(std::string *str, int node1, int node2, std::string *file, float val):
			Element(str), node_positive(node1), node_negative(node2), model(*file), area_factor(val) { }
	};

	class MOS : public Element {
		public:
		node_id_t drain;
		node_id_t gate;
		node_id_t source;
		node_id_t body;
		std::string model;
		float length;
		float width;

		/* MOS Constructor */
		MOS(std::string *str, int node1, int node2, int node3, int node4, std::string *file, float val1, float val2):
			Element(str), drain(node1), gate(node2), source(node3), body(node4), model(*file), length(val1), width(val2) { }
	};

	class BJT : public Element {
		public:
		node_id_t collector;
		node_id_t base;
		node_id_t emitter;
		std::string model;
		float area_factor;

		/* BJT Constructor */
		BJT(std::string *str, int node1, int node2, int node3, std::string *file, float val):
			Element(str), collector(node1), base(node2), emitter(node3), model(*file), area_factor(val) { }
	};
	
	/* Template class to store elements of netlist */
	template <class ElementType>
		class ElementList {
			public:
			std::vector<ElementType> elements;
			std::unordered_map<std::string, element_id_t> name_map;
			element_id_t find_element_name(std::string &name);
			element_id_t append_element_name(std::string &name);
			bool add_element(ElementType *e);
		};

	/* Netlist class contains a list of pointers to each element type */
	class Netlist {
		public:
		spic::ElementList<VoltageSource> voltage_sources;
		spic::ElementList<CurrentSource> current_sources;
		spic::ElementList<Resistor>      resistors;
		spic::ElementList<Capacitor>     capacitors;
		spic::ElementList<Inductor>      inductors;
		spic::ElementList<Diode>         diodes;
		spic::ElementList<MOS>           mos;
		spic::ElementList<BJT>           bjt;

		bool add_voltage_source(VoltageSource *v);
		bool add_current_source(CurrentSource *c);
		bool add_resistor(Resistor *r);
		bool add_capacitor(Capacitor *c);
		bool add_inductor(Inductor *i);
		bool add_diode(Diode *d);
		bool add_mos(MOS *m);
		bool add_bjt(BJT *q);
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