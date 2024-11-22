#pragma once

#include <Eigen/Core>

#include "netlist.h"

namespace spic {
	class MNASystemDC {
		public:
		MNASystemDC(Netlist &netlist, int total_nodes);
		MNASystemDC(Netlist &netlist, int total_nodes, int dim);

		// Matrices of system
		Eigen::MatrixXd static_matrix; // DC Coefficient matrix
		Eigen::VectorXd node_VC_vector; // Unknown Variable matrix
		Eigen::VectorXd source_vector; // Matrix with source values
		
		private:
		Netlist &netlist;
		int total_nodes;

		void add_resistor_stamp(node_id_t node_positive, node_id_t node_negative, float value);
		void add_current_source_stamp(node_id_t node_positive, node_id_t node_negative, float value);
		void add_voltage_source_stamp(node_id_t node_positive, node_id_t node_negative, int voltage_src_id, float value);
	};
}

std::ostream& operator<<(std::ostream &out, spic::MNASystemDC &system);