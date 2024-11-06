#include <ostream>

#include "netlist.h"
#include "node_table.h"

extern spic::NodeTable *node_table;

namespace spic {
	void Netlist::add_voltage_source(VoltageSource *v) {
		voltage_sources.push_back(v);
	}

	void Netlist::add_current_source(CurrentSource *i) {
		current_sources.push_back(i);
	}

	void Netlist::add_resistor(Resistor *r) {
		resistors.push_back(r);
	}

	void Netlist::add_capacitor(Capacitor *c) {
		capacitors.push_back(c);
	}

	void Netlist::add_inductor(Inductor *l) {
		inductors.push_back(l);
	}

	void Netlist::add_diode(Diode *d) {
		diodes.push_back(d);
	}

	void Netlist::add_bjt(BJT *q) {
		bj_transistors.push_back(q);
	}

	void Netlist::add_mos(MOS *m) {
		mos_transistors.push_back(m);
	}
}

/* Support of << operator for printing a NodeTable */
std::ostream& operator<<(std::ostream &out, spic::Netlist &nl) {
#if VERBOSE_NETLIST == 2
	for (auto it = nl.voltage_sources.begin(); it != nl.voltage_sources.end(); ++it) {
		out << **it << std::endl;
	}

	for (auto it = nl.current_sources.begin(); it != nl.current_sources.end(); ++it) {
		out << **it << std::endl;
	}

	for (auto it = nl.resistors.begin(); it != nl.resistors.end(); ++it) {
		out << **it << std::endl;
	}

	for (auto it = nl.capacitors.begin(); it != nl.capacitors.end(); ++it) {
		out << **it << std::endl;
	}

	for (auto it = nl.inductors.begin(); it != nl.inductors.end(); ++it) {
		out << **it << std::endl;
	}

	for (auto it = nl.diodes.begin(); it != nl.diodes.end(); ++it) {
		out << **it << std::endl;
	}

	for (auto it = nl.mos_transistors.begin(); it != nl.mos_transistors.end(); ++it) {
		out << **it << std::endl;
	}

	for (auto it = nl.bj_transistors.begin(); it != nl.bj_transistors.end(); ++it) {
		out << **it << std::endl;
	}
#endif
#if VERBOSE_NETLIST >= 1
	out << "Netlist consists of:" << std::endl;
	out << "\t" <<  nl.voltage_sources.size() << " voltage sources" << std::endl;
	out << "\t" <<  nl.current_sources.size() << " current sources" << std::endl;
	out << "\t" <<  nl.resistors.size() << " resistors" << std::endl;
	out << "\t" <<  nl.capacitors.size() << " capacitors" << std::endl;
	out << "\t" <<  nl.inductors.size() << " inductors" << std::endl;
	out << "\t" <<  nl.diodes.size() << " diode" << std::endl;
	out << "\t" <<  nl.mos_transistors.size() << " mosfet transistors" << std::endl;
	out << "\t" <<  nl.bj_transistors.size() << " bipolar junction transistors" << std::endl;
#endif
	return out;
}

std::ostream& operator<<(std::ostream &out, spic::VoltageSource &v) {
	out << "Voltage Source:\n\tName: \'" << *v.name << "\'" << std::endl;
	out << "\t+ Node: " << v.node_positive << " (" << node_table->get_node_name(v.node_positive) << ")" << std::endl;
	out << "\t- Node: " << v.node_negative << " (" << node_table->get_node_name(v.node_negative) << ")" << std::endl;
	out << "\tValue: " << v.value << std::endl;
	return out;
}

/* Support of << operator for printing a Currect Source object*/
std::ostream& operator<<(std::ostream &out, spic::CurrentSource &i) {
	out << "Current Source:\n\tName: \'" << *i.name << "\'" << std::endl;
	out << "\t+ Node: " << i.node_positive << " (" << node_table->get_node_name(i.node_positive) << ")" << std::endl;
	out << "\t- Node: " << i.node_negative << " (" << node_table->get_node_name(i.node_negative) << ")" << std::endl;
	out << "\tValue: " << i.value << std::endl;
	return out;
}

/* Support of << operator for printing a Resistor object*/
std::ostream& operator<<(std::ostream &out, spic::Resistor &r) {
	out << "Resistor:\n\tName: \'" << *r.name << "\'" << std::endl;
	out << "\t+ Node: " << r.node_positive << " (" << node_table->get_node_name(r.node_positive) << ")" << std::endl;
	out << "\t- Node: " << r.node_negative << " (" << node_table->get_node_name(r.node_negative) << ")" << std::endl;
	out << "\tValue: " << r.value << std::endl;
	return out;
}

/* Support of << operator for printing a Capacitor object*/
std::ostream& operator<<(std::ostream &out, spic::Capacitor &c) {
	out << "Capacitor:\n\tName: \'" << *c.name << "\'" << std::endl;
	out << "\t- Node: " << c.node_negative << " (" << node_table->get_node_name(c.node_negative) << ")" << std::endl;
	out << "\t+ Node: " << c.node_positive << " (" << node_table->get_node_name(c.node_positive) << ")" << std::endl;
	out << "\tValue: " << c.value << std::endl;
	return out;
}

/* Support of << operator for printing a Inductor object*/
std::ostream& operator<<(std::ostream &out, spic::Inductor &l) {
	out << "Inductor:\n\tName: \'" << *l.name << "\'" << std::endl;
	out << "\t+ Node: " << l.node_positive << " (" << node_table->get_node_name(l.node_positive) << ")" << std::endl;
	out << "\t- Node: " << l.node_negative << " (" << node_table->get_node_name(l.node_negative) << ")" << std::endl;
	out << "\tValue: " << l.value << std::endl;
	return out;
}

std::ostream& operator<<(std::ostream &out, spic::Diode &d) {
	out << "Diode:\n\tName: \'" << *d.name << "\'" << std::endl;
	out << "\t+ Node: " << d.node_positive << " (" << node_table->get_node_name(d.node_positive) << ")" << std::endl;
	out << "\t- Node: " << d.node_negative << " (" << node_table->get_node_name(d.node_negative) << ")" << std::endl;
	out << "\tModel: " << *d.model << std::endl;
	out << "\tArea: " << d.area_factor << std::endl;
	return out;
}

std::ostream& operator<<(std::ostream &out, spic::MOS &m) {
	out << "MOS:\n\tName: \'" << *m.name << "\'" << std::endl;
	out << "\tDrain: " << m.drain << " (" << node_table->get_node_name(m.drain) << ")" << std::endl;
	out << "\tGate: " << m.gate << " (" << node_table->get_node_name(m.gate) << ")" << std::endl;
	out << "\tSource: " << m.source << " (" << node_table->get_node_name(m.source) << ")" << std::endl;
	out << "\tBody: " << m.body << " (" << node_table->get_node_name(m.body) << ")" << std::endl;
	out << "\tModel: " << *m.model << std::endl;
	out << "\tLength: " << m.length << std::endl;
	out << "\tWidth: " << m.width << std::endl;
	return out;
}

std::ostream& operator<<(std::ostream &out, spic::BJT &q) {
	out << "Diode:\n\tName: \'" << *q.name << "\'" << std::endl;
	out << "\tCollector: " << q.collector << " (" << node_table->get_node_name(q.collector) << ")" << std::endl;
	out << "\tBase: " << q.base << " (" << node_table->get_node_name(q.base) << ")" << std::endl;
	out << "\tEmitter: " << q.emitter << " (" << node_table->get_node_name(q.emitter) << ")" << std::endl;
	out << "\tModel: " << *q.model << std::endl;
	out << "\tArea: " << q.area_factor << std::endl;
	return out;
}
