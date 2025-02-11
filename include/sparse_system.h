#pragma once

#include <Eigen/SparseCore>


namespace spic {
	/* Forward Decleration */
	class Netlist;
	typedef int node_id_t;
	enum transient_method : unsigned int;
	typedef enum transient_method transient_method_t;

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
	
	class MNASparseSystem : public SparseSystem {
		public:
		MNASparseSystem(Netlist &netlist, int total_nodes);
		MNASparseSystem(Netlist &netlist, int total_nodes, int dim);

		int total_nodes;

		private:
		Netlist &netlist;

		void create_dc_sparse_system();

		void add_resistor_stamp(std::vector<Eigen::Triplet<double>> &triplets,
								node_id_t node_positive, node_id_t node_negative, float value);

		void add_current_source_stamp(node_id_t node_positive, node_id_t node_negative, float value);

		void add_voltage_source_stamp(std::vector<Eigen::Triplet<double>> &triplets,
									node_id_t node_positive, node_id_t node_negative,
									int voltage_src_id, float value);
	};

	class MNASparseSystemTransient {
		public:
		transient_method_t transient_method;
		MNASparseSystem &mna_sparse_system;
		Eigen::SparseMatrix<double> G;
		Eigen::SparseMatrix<double> C;
		Eigen::VectorXd dc_source_vector;

		// Constructor: Copy the A matrix to the G matrix to be kept
		// calculate the C matrix, A will be used to store the transient system 
		MNASparseSystemTransient(transient_method_t transient_method, MNASparseSystem &mna_sparse_system) :
						transient_method(transient_method),
						G(mna_sparse_system.A), C(mna_sparse_system.n, mna_sparse_system.n),
						mna_sparse_system(mna_sparse_system), dc_source_vector(mna_sparse_system.b)
		{
			create_initial_tran_system();
		}

		// Destructor: Copy the G matrix back to the A matrix to be used for DC analysis
		// also copy the dc_source_vector back to the b vector
		~MNASparseSystemTransient()
		{
			mna_sparse_system.A = G;
			mna_sparse_system.b = dc_source_vector;
		}

		void create_initial_tran_system();
		void create_tran_system(double time_step);

		void update_tran_system_tr(Eigen::VectorXd &e_new, Eigen::VectorXd &e_old, double time_step);
		void update_tran_system_be(Eigen::VectorXd &e, double time_step);

		void add_capacitor_stamp(std::vector<Eigen::Triplet<double>> &triplets,
								node_id_t node_positive, node_id_t node_negative, float value);

		void add_inductor_stamp(std::vector<Eigen::Triplet<double>> &triplets,
												int voltage_src_id, float value);
	};
}