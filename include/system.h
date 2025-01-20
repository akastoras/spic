#pragma once

#include <Eigen/Core>
#include <Eigen/SparseCore>

#include "netlist.h"

namespace spic {
	class System {
		public:
		// Matrices of system
		int n;
		Eigen::MatrixXd A; // DC Coefficient matrix
		Eigen::VectorXd x; // Unknown Variable matrix
		Eigen::VectorXd b; // Matrix with source values

		System(int n) : n(n), A(n, n), b(n), x(n)
		{
			A.setZero();
			b.setZero();
			x.setZero();
		}
	};

	class SparseSystem {
		public:
		// Matrices of system
		int n;
		Eigen::SparseMatrix<double> A; // DC Coefficient matrix
		Eigen::VectorXd x; // Unknown Variable matrix
		Eigen::VectorXd b; // Matrix with source values

		SparseSystem(int n) : n(n), A(n, n), b(n), x(n)
		{
			b.setZero();
			x.setZero();
		}
	};

	class MNASystem : public System {
		public:
		MNASystem(Netlist &netlist, int total_nodes);
		MNASystem(Netlist &netlist, int total_nodes, int dim);

		private:
		Netlist &netlist;
		int total_nodes;

		void create_dc_system();
		void add_resistor_stamp(node_id_t node_positive, node_id_t node_negative, float value);
		void add_current_source_stamp(node_id_t node_positive, node_id_t node_negative, float value);
		void add_voltage_source_stamp(node_id_t node_positive, node_id_t node_negative, int voltage_src_id, float value);

		void create_trans_system();
	};

	class MNASparseSystem : public SparseSystem {
		public:
		MNASparseSystem(Netlist &netlist, int total_nodes);
		MNASparseSystem(Netlist &netlist, int total_nodes, int dim);

		private:
		Netlist &netlist;
		int total_nodes;

		void create_dc_sparse_system();
		void add_resistor_stamp(std::vector<Eigen::Triplet<double>> &triplets, node_id_t node_positive, node_id_t node_negative, float value);
		void add_current_source_stamp(node_id_t node_positive, node_id_t node_negative, float value);
		void add_voltage_source_stamp(std::vector<Eigen::Triplet<double>> &triplets, node_id_t node_positive, node_id_t node_negative, int voltage_src_id, float value);
	};
}

std::ostream& operator<<(std::ostream &out, spic::System &system);