#pragma once

#include <Eigen/Core>

// TODO: Maybe move System in a system.cpp and rename system.cpp to mna_system.cpp

namespace spic {
	/* Forward Decleration */
	class Netlist;
	typedef int node_id_t;
	enum transient_method : unsigned int;
	typedef enum transient_method transient_method_t;
	
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

	class MNASystem : public System {
		public:
		MNASystem(Netlist &netlist, int total_nodes);
		MNASystem(Netlist &netlist, int total_nodes, int dim);

		int total_nodes;

		private:
		Netlist &netlist;

		void create_dc_system();
		void add_resistor_stamp(node_id_t node_positive, node_id_t node_negative, float value);
		void add_current_source_stamp(node_id_t node_positive, node_id_t node_negative, float value);
		void add_voltage_source_stamp(node_id_t node_positive, node_id_t node_negative,
									int voltage_src_id, float value);

	};

	class MNASystemTransient {
		public:
		transient_method_t transient_method;
		MNASystem &mna_system;
		Eigen::MatrixXd G;
		Eigen::MatrixXd C;
		Eigen::VectorXd dc_source_vector;

		// Constructor: Copy the A matrix to the G matrix to be kept
		// calculate the C matrix, A will be used to store the transient system 
		MNASystemTransient(transient_method_t transient_method, MNASystem &mna_system) :
						transient_method(transient_method),
						G(mna_system.A), C(mna_system.n, mna_system.n), 
						mna_system(mna_system), dc_source_vector(mna_system.b)
		{
			C.setZero();
			create_initial_tran_system();
		}

		// Destructor: Copy the G matrix back to the A matrix to be used for DC analysis
		// also copy the dc_source_vector back to the b vector
		~MNASystemTransient()
		{
			mna_system.A = G;
			mna_system.b = dc_source_vector;
		}

		void create_initial_tran_system();
		void create_tran_system(double time_step);

		void update_tran_system_tr(Eigen::VectorXd &e_new, Eigen::VectorXd &e_old, double time_step);
		void update_tran_system_be(Eigen::VectorXd &e, double time_step);

		void add_capacitor_stamp(node_id_t node_positive, node_id_t node_negative, float value);
		void add_inductor_stamp(int voltage_src_id, float value);
	};
}

std::ostream& operator<<(std::ostream &out, spic::System &system);