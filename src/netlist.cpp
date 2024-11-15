#include <ostream>
#include <algorithm>

#include "netlist.h"
#include "node_table.h"

extern spic::NodeTable *node_table;

namespace spic {
	template <class ElementType>
	element_id_t ElementList<ElementType>::find_element_name(std::string &name) {
		auto it = name_map.find(name);
		if (it != name_map.end()) {
			return it->second;
		}
		return -1; // Or some other invalid node_id_t value
	}

	template <class ElementType>
	element_id_t ElementList<ElementType>::append_element_name(std::string &name) {
		element_id_t eid = name_map.size();
		name_map[name] = eid;
		return eid;
	}

	template <class ElementType>
	bool ElementList<ElementType>::add_element(ElementType *e) {
		if (find_element_name(e->name) == -1) {
			elements.push_back(*e);
			append_element_name(e->name);
			return true;
		}
		return false; // Element with the same name already exists
	}

	bool Netlist::add_voltage_source(VoltageSource *v) {
		return voltage_sources.add_element(v);
	}
	bool Netlist::add_current_source(CurrentSource *c) {
		return current_sources.add_element(c);
	}
	bool Netlist::add_resistor(Resistor *r) {
		return resistors.add_element(r);
	}
	bool Netlist::add_capacitor(Capacitor *c) {
		return capacitors.add_element(c);
	}
	bool Netlist::add_inductor(Inductor *i) {
		return inductors.add_element(i);
	}
	bool Netlist::add_diode(Diode *d) {
		return diodes.add_element(d);
	}
	bool Netlist::add_mos(MOS *m) {
		return mos.add_element(m);
	}
	bool Netlist::add_bjt(BJT *q) {
		return bjt.add_element(q);
	}
}

template <class ElementType>
	std::ostream& operator<<(std::ostream &out, spic::ElementList<ElementType> &el_list) {
		for (auto it = el_list.elements.begin(); it != el_list.elements.end(); ++it) {
			out << *it << std::endl;
		}

		return out;
	}

std::ostream& operator<<(std::ostream &out, spic::VoltageSource &v) {
	out << "Voltage Source:\n\tName: \'" << v.name << "\'" << std::endl;
	out << "\t+ Node: "                  << v.node_positive << " (" << node_table->get_node_name(v.node_positive) << ")" << std::endl;
	out << "\t- Node: "                  << v.node_negative << " (" << node_table->get_node_name(v.node_negative) << ")" << std::endl;
	out << "\tValue: "                   << v.value << std::endl;
	return out;
}

/* Support of << operator for printing a Currect Source object*/
std::ostream& operator<<(std::ostream &out, spic::CurrentSource &i) {
	out << "Current Source:\n\tName: \'" << i.name << "\'" << std::endl;
	out << "\t+ Node: "                  << i.node_positive << " (" << node_table->get_node_name(i.node_positive) << ")" << std::endl;
	out << "\t- Node: "                  << i.node_negative << " (" << node_table->get_node_name(i.node_negative) << ")" << std::endl;
	out << "\tValue: "                   << i.value << std::endl;
	return out;
}

/* Support of << operator for printing a Resistor object*/
std::ostream& operator<<(std::ostream &out, spic::Resistor &r) {
	out << "Resistor:\n\tName: \'"  << r.name << "\'" << std::endl;
	out << "\t+ Node: "             << r.node_positive << " (" << node_table->get_node_name(r.node_positive) << ")" << std::endl;
	out << "\t- Node: "             << r.node_negative << " (" << node_table->get_node_name(r.node_negative) << ")" << std::endl;
	out << "\tValue: "              << r.value << std::endl;
	return out;
}

/* Support of << operator for printing a Capacitor object*/
std::ostream& operator<<(std::ostream &out, spic::Capacitor &c) {
	out << "Capacitor:\n\tName: \'" << c.name << "\'" << std::endl;
	out << "\t- Node: "             << c.node_negative << " (" << node_table->get_node_name(c.node_negative) << ")" << std::endl;
	out << "\t+ Node: "             << c.node_positive << " (" << node_table->get_node_name(c.node_positive) << ")" << std::endl;
	out << "\tValue: "              << c.value << std::endl;
	return out;
}

/* Support of << operator for printing a Inductor object*/
std::ostream& operator<<(std::ostream &out, spic::Inductor &l) {
	out << "Inductor:\n\tName: \'" << l.name << "\'" << std::endl;
	out << "\t+ Node: "            << l.node_positive << " (" << node_table->get_node_name(l.node_positive) << ")" << std::endl;
	out << "\t- Node: "            << l.node_negative << " (" << node_table->get_node_name(l.node_negative) << ")" << std::endl;
	out << "\tValue: "             << l.value << std::endl;
	return out;
}

/* Support of << operator for printing a Diode object*/
std::ostream& operator<<(std::ostream &out, spic::Diode &d) {
	out << "Diode:\n\tName: \'" << d.name         << "\'" << std::endl;
	out << "\t+ Node: "         << d.node_positive << " (" << node_table->get_node_name(d.node_positive) << ")" << std::endl;
	out << "\t- Node: "         << d.node_negative << " (" << node_table->get_node_name(d.node_negative) << ")" << std::endl;
	out << "\tModel: "          << d.model         << std::endl;
	out << "\tArea: "           << d.area_factor   << std::endl;
	return out;
}

/* Support of << operator for printing a MOS object*/
std::ostream& operator<<(std::ostream &out, spic::MOS &m) {
	out << "MOS:\n\tName: \'" << m.name << "\'" << std::endl;
	out << "\tDrain: "        << m.drain << " (" << node_table->get_node_name(m.drain) << ")" << std::endl;
	out << "\tGate: "         << m.gate << " (" << node_table->get_node_name(m.gate) << ")" << std::endl;
	out << "\tSource: "       << m.source << " (" << node_table->get_node_name(m.source) << ")" << std::endl;
	out << "\tBody: "         << m.body << " (" << node_table->get_node_name(m.body) << ")" << std::endl;
	out << "\tModel: "        << m.model << std::endl;
	out << "\tLength: "       << m.length << std::endl;
	out << "\tWidth: "        << m.width << std::endl;
	return out;
}

/* Support of << operator for printing a BJT object*/
std::ostream& operator<<(std::ostream &out, spic::BJT &q) {
	out << "BJT:\n\tName: \'" << q.name        << "\'" << std::endl;
	out << "\tCollector: "    << q.collector   << " (" << node_table->get_node_name(q.collector) << ")" << std::endl;
	out << "\tBase: "         << q.base        << " (" << node_table->get_node_name(q.base) << ")" << std::endl;
	out << "\tEmitter: "      << q.emitter     << " (" << node_table->get_node_name(q.emitter) << ")" << std::endl;
	out << "\tModel: "        << q.model       << std::endl;
	out << "\tArea: "         << q.area_factor << std::endl;
	return out;
}

/* Support of << operator for printing a NodeTable */
std::ostream& operator<<(std::ostream &out, spic::Netlist &nl) {
#if VERBOSE_NETLIST == 2
	out << nl.voltage_sources << std::endl;
	out << nl.current_sources << std::endl;
	out << nl.resistors       << std::endl;
	out << nl.capacitors      << std::endl;
	out << nl.inductors       << std::endl;
	out << nl.diodes          << std::endl;
	out << nl.mos             << std::endl;
	out << nl.bjt             << std::endl;
#endif
#if VERBOSE_NETLIST >= 1
	out << "Netlist consists of:" << std::endl;
	out << "\t" <<  nl.voltage_sources.elements.size() << " voltage sources" << std::endl;
	out << "\t" <<  nl.current_sources.elements.size() << " current sources" << std::endl;
	out << "\t" <<  nl.resistors.elements.size()       << " resistors" << std::endl;
	out << "\t" <<  nl.capacitors.elements.size()      << " capacitors" << std::endl;
	out << "\t" <<  nl.inductors.elements.size()       << " inductors" << std::endl;
	out << "\t" <<  nl.diodes.elements.size()          << " diode" << std::endl;
	out << "\t" <<  nl.mos.elements.size()             << " mosfet transistors" << std::endl;
	out << "\t" <<  nl.bjt.elements.size()             << " bipolar junction transistors" << std::endl;
#endif
	return out;
}