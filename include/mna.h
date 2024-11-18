#pragma once

#include <Eigen/Core>

#include "netlist.h"

namespace spic {
	class MNASystemDC {
		public:
		MNASystemDC(Netlist &netlist, int total_nodes);

		private:
		Netlist &netlist;
		int total_nodes;

		// Matrices of system
		Eigen::MatrixXf static_matrix; // DC Coefficient matrix
		Eigen::VectorXf node_VC_vector; // Unknown Variable matrix
		Eigen::VectorXf source_vector; // Matrix with source values
	};
}