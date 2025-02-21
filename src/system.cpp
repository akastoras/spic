#include <iomanip>
#include <iostream>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Dense>

#include "netlist.h"

#include "system.h"

namespace spic {
	/* Implemantation of the Dense MNA System */
	MNASystem::MNASystem(Netlist &netlist, int total_nodes)
	: MNASystem(netlist, total_nodes, total_nodes - 1 + netlist.voltage_sources.size() + netlist.inductors.size()) {}

	MNASystem::MNASystem(Netlist &netlist, int total_nodes, int MNA_matrix_dim)
		: netlist(netlist), total_nodes(total_nodes), System(MNA_matrix_dim)
	{
		create_dc_system();
	}

	/* Add stamps for the G part of the MNA system */
	void MNASystem::create_dc_system()
	{
		int total_voltage_sources = netlist.voltage_sources.size();
		int total_inductors = netlist.inductors.size();
		int node_pos, node_neg, matrix_inductor_idx, matrix_voltage_idx;

		// Fill the matrix with the stamps of the resistors
		for (auto it = netlist.resistors.elements.begin(); it != netlist.resistors.elements.end(); ++it) {
			add_resistor_stamp(it->node_positive, it->node_negative, it->value);
		}

		// Fill the source vector with the stamps of the current sources
		for (auto it = netlist.current_sources.elements.begin(); it != netlist.current_sources.elements.end(); ++it) {
			add_current_source_stamp(it->node_positive, it->node_negative, it->value);
		}

		// Fill the matrix with the stamps of the voltage sources
		for (int i = 0; i < total_voltage_sources; i++) {
			node_pos = netlist.voltage_sources.elements[i].node_positive;
			node_neg = netlist.voltage_sources.elements[i].node_negative;
			add_voltage_source_stamp(node_pos, node_neg, i, netlist.voltage_sources.elements[i].value);
		}

		// Fill the matrix with the stamps of the inductors
		for (int i = 0; i < total_inductors; i++) {
			node_pos = netlist.inductors.elements[i].node_positive;
			node_neg = netlist.inductors.elements[i].node_negative;
			add_voltage_source_stamp(node_pos, node_neg, total_voltage_sources + i, 0);
		}
	}

	/* For a resistor, do the following adjustments to the static_matrix (A):
	 * A(<->,<+>) -= 1/resistance
	 * A(<+>,<->) -= 1/resistance
	 * A(<+>,<+>) += 1/resistance
	 * A(<->,<->) += 1/resistance
	 * Where <+>,<-> are the positive and negative nodes of the resistor
	 */
	void MNASystem::add_resistor_stamp(node_id_t node_positive, node_id_t node_negative, float value) {
		double conductance = 1 / value;
		if (node_positive > 0 && node_negative > 0) {
			A(node_positive-1, node_negative-1) -= conductance;
			A(node_negative-1, node_positive-1) -= conductance;
		}
		if (node_positive > 0) {
			A(node_positive-1, node_positive-1) += conductance;
		}
		if (node_negative > 0) {
			A(node_negative-1, node_negative-1) += conductance;
		}
	}

	/* For a current source, do the following adjustments to the source vector (b):
	 * b(<+>) -= value
	 * b(<->) += value
	 * Where <+>,<-> are the positive and negative nodes of the current source
	 */
	void MNASystem::add_current_source_stamp(node_id_t node_positive, node_id_t node_negative, float value) {
		if (node_positive > 0) {
			b(node_positive - 1) -= value;
		}
		if (node_negative > 0) {
			b(node_negative - 1) += value;
		}
	}

	/* For a voltage source, do the following adjustments to the static matrix (A):
	 * A(n-1+k,<+>) += 1
	 * A(<+>,n-1+k) += 1
	 * A(n-1+k,<->) -= 1
	 * A(<->,n-1+k) -= 1
	 * and rhe following adjustment to the source vector (b):
	 * b(n-1+k) = value
	 * Where <+>,<-> are the positive and negative nodes of the voltage source
	 * n is the total number of nodes and k is the index of the voltage source
	 * Note that in DC analysis the inductors are considered as voltage sources with 0 value
	 */
	void MNASystem::add_voltage_source_stamp(node_id_t node_positive, node_id_t node_negative, int voltage_src_id, float value) {
		int matrix_voltage_idx = total_nodes - 1 + voltage_src_id;
		b(matrix_voltage_idx) = value;
		if (node_positive > 0) {
			A(matrix_voltage_idx, node_positive - 1) += 1;
			A(node_positive - 1, matrix_voltage_idx) += 1;
		}
		if (node_negative > 0) {
			A(matrix_voltage_idx, node_negative - 1) -= 1;
			A(node_negative - 1, matrix_voltage_idx) -= 1;
		}
	}

	/* 
	 * Add stamps for the transient part (C) of the system depending
	 * on the derivative calculation method
	 */
	void MNASystemTransient::create_initial_tran_system()
	{
		int total_inductors = netlist.inductors.size();
		int total_capacitors = netlist.capacitors.size();
		int total_voltage_sources = netlist.voltage_sources.size();
		int node_pos, node_neg;

		// Fill the matrix with the stamps of the capacitors
		for (auto it = netlist.capacitors.elements.begin(); it != netlist.capacitors.elements.end(); ++it) {
			add_capacitor_stamp(it->node_positive, it->node_negative, it->value);
		}

		// Fill the matrix with the stamps of the inductors
		for (int i = 0; i < total_inductors; i++) {
			add_inductor_stamp(total_voltage_sources + i, netlist.inductors.elements[i].value);
		}
	}

	/*
	 * Calculates the A = G + C / time_step matrix for a Transient Analysis run
	 */
	void MNASystemTransient::create_tran_system(double time_step)
	{
		if (transient_method == BE) {
			mna_system.A = G + C / time_step;
		} else if (transient_method == TR) {
			mna_system.A = G + C * (2 / time_step);
		}
	}

	/*
	 * Update the b part of the Transient Equation for the Backward Euler method
	 */
	void MNASystemTransient::update_tran_system_be(Eigen::VectorXd &e, double time_step)
	{
		mna_system.b = e + (C * mna_system.x) / time_step;
	}

	/*
	 * Update the b part of the Transient Equation for the Trapezoidal method
	 */
	void MNASystemTransient::update_tran_system_tr(Eigen::VectorXd &e_new, Eigen::VectorXd &e_old, double time_step)
	{
		mna_system.b = e_new + e_old - (G - C * (2.0 / time_step)) * mna_system.x;
	}

	/* Adds capacitor stamps for the transient part (C) of the MNA system */
	void MNASystemTransient::add_capacitor_stamp(node_id_t node_positive, node_id_t node_negative, float value)
	{
		if (node_positive > 0 && node_negative > 0) {
			C(node_positive-1, node_negative-1) -= value;
			C(node_negative-1, node_positive-1) -= value;

		}
		if (node_positive > 0) {
			C(node_positive-1, node_positive-1) += value;
		}
		if (node_negative > 0) {
			C(node_negative-1, node_negative-1) += value;
		}
	}

	/* Adds inductor stamps for the transient part (C) of the MNA system */
	void MNASystemTransient::add_inductor_stamp(int voltage_src_id, float value)
	{
		int matrix_voltage_idx = mna_system.total_nodes - 1 + voltage_src_id;
		C(matrix_voltage_idx, matrix_voltage_idx) -= value;
	}
}


/* Finds the maximum width per array column to help beautify the printing of the MNA system
 * The width of each element is the length of the string representation of the element
 * without trailing zeros
 */
static std::vector<int> getMaxWidths(const Eigen::MatrixXd& A, const Eigen::VectorXd& b) {
	std::vector<int> maxWidths(A.cols(), 0);
	int maxWidthB = 0;
	int width;
	std::string str;
	for (int i = 0; i < A.rows(); ++i) {
		for (int j = 0; j < A.cols(); ++j) {
			str = std::to_string(A(i, j));
			str.erase(str.find_last_not_of('0') + 1, std::string::npos);

			width = str.length();
			if (width > maxWidths[j]) {
				maxWidths[j] = width;
			}
		}

		str = std::to_string(b(i));
		str.erase(str.find_last_not_of('0') + 1, std::string::npos);

		width = str.length();
		if (width > maxWidthB) {
			maxWidthB = width;
		}
	}
	maxWidths.push_back(maxWidthB); // Add the width of b to the end of the vector
	return maxWidths;
}

/* Prints the MNA system in a more readable format
 * The system is printed in the form:
 * [A(1,1) A(1,2) ... A(1,n)] [x(1)]   [b(1)]
 * [A(2,1) A(2,2) ... A(2,n)] [x(2)]   [b(2)]
 * ...
 * [A(n,1) A(n,2) ... A(n,n)] [x(n)] = [b(n)]
 * Where A is the static_matrix, x is the node_VC_vector and b is the source_vector
 */
static void printSystem(const Eigen::MatrixXd& A, const Eigen::VectorXd& b) {
	int rows = A.rows();
	int midRow = rows / 2; // Middle row index for alignment
	std::vector<int> widths = getMaxWidths(A, b); // Get the maximum widths of the elements in A and b

	for (int i = 0; i < rows; ++i) {
		// Print row of A
		std::cout << "[";
		for (int j = 0; j < A.cols(); ++j) {
			std::cout << std::setw(widths[j]) << A(i, j);
			if (j < A.cols() - 1) std::cout << " ";
		}
		std::cout << "] ";

		// Print corresponding x
		std::cout << "[x" << std::setw(std::to_string(b.size()).size()) << i + 1 << "] ";

		// Print '=' only on the middle row
		if (i == midRow) {
			std::cout << "= ";
		} else {
			std::cout << "  ";
		}

		// Print corresponding b
		std::cout << "[" << std::setw(widths.back()) << b(i) << "]" << std::endl;
	}
}

/* Support of << operator for printing a MNASystem object*/
std::ostream& operator<<(std::ostream &out, spic::System &system)
{
	printSystem(system.A, system.b);
	return out;
}
