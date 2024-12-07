#pragma once

#include <Eigen/Core>

#include "netlist.h"

namespace spic {
	class System {
		public:
		// Matrices of system
		int n;
		Eigen::MatrixXd A; // DC Coefficient matrix
		Eigen::VectorXd x; // Unknown Variable matrix
		Eigen::VectorXd b; // Matrix with source values

		System(int n) : n(n), A(n, n), b(n), x(n) {}
	};

	class MNASystemDC : public System {
		public:
		MNASystemDC(Netlist &netlist, int total_nodes);
		MNASystemDC(Netlist &netlist, int total_nodes, int dim);

		private:
		Netlist &netlist;
		int total_nodes;

		void add_resistor_stamp(node_id_t node_positive, node_id_t node_negative, float value);
		void add_current_source_stamp(node_id_t node_positive, node_id_t node_negative, float value);
		void add_voltage_source_stamp(node_id_t node_positive, node_id_t node_negative, int voltage_src_id, float value);
	};
}

std::ostream& operator<<(std::ostream &out, spic::System &system);