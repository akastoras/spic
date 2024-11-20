#pragma once

#include <Eigen/Core>

#include "netlist.h"

namespace spic {
	class MNASystemDC {
		public:
		MNASystemDC(Netlist &netlist, int total_nodes);
		MNASystemDC(Netlist &netlist, int total_nodes, int dim);

		// Matrices of system
		Eigen::MatrixXf static_matrix; // DC Coefficient matrix
		Eigen::VectorXf node_VC_vector; // Unknown Variable matrix
		Eigen::VectorXf source_vector; // Matrix with source values
		
		private:
		Netlist &netlist;
		int total_nodes;
	};
}

std::ostream& operator<<(std::ostream &out, spic::MNASystemDC &system);