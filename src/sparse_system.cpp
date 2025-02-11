#include <iomanip>
#include <iostream>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Dense>

#include "netlist.h"

#include "sparse_system.h"

namespace spic {
	/* Implemantation of the Sparse MNA System */
	MNASparseSystem::MNASparseSystem(Netlist &netlist, int total_nodes)
		: MNASparseSystem(netlist, total_nodes, total_nodes - 1 + netlist.voltage_sources.size() + netlist.inductors.size()) {}

	MNASparseSystem::MNASparseSystem(Netlist &netlist, int total_nodes, int MNA_matrix_dim)
		: netlist(netlist), total_nodes(total_nodes), SparseSystem(MNA_matrix_dim)
		{
			create_dc_sparse_system();
		}

	void MNASparseSystem::create_dc_sparse_system()
	{
		int total_voltage_sources = netlist.voltage_sources.size();
		int total_inductors = netlist.inductors.size();
		int total_resistors = netlist.resistors.size();
		int node_pos, node_neg, matrix_inductor_idx, matrix_voltage_idx;
		std::vector<Eigen::Triplet<double>> triplets;
		triplets.reserve(4 * (total_resistors + total_voltage_sources + total_inductors));

		// Fill the matrix with the stamps of the resistors
		for (auto it = netlist.resistors.elements.begin(); it != netlist.resistors.elements.end(); ++it) {
			add_resistor_stamp(triplets, it->node_positive, it->node_negative, it->value);
		}

		// Fill the source vector with the stamps of the current sources
		for (auto it = netlist.current_sources.elements.begin(); it != netlist.current_sources.elements.end(); ++it) {
			add_current_source_stamp(it->node_positive, it->node_negative, it->value);
		}

		// Fill the matrix with the stamps of the voltage sources
		for (int i = 0; i < total_voltage_sources; i++) {
			node_pos = netlist.voltage_sources.elements[i].node_positive;
			node_neg = netlist.voltage_sources.elements[i].node_negative;
			add_voltage_source_stamp(triplets, node_pos, node_neg, i, netlist.voltage_sources.elements[i].value);
		}

		// Fill the matrix with the stamps of the inductors
		for (int i = 0; i < total_inductors; i++) {
			node_pos = netlist.inductors.elements[i].node_positive;
			node_neg = netlist.inductors.elements[i].node_negative;
			add_voltage_source_stamp(triplets, node_pos, node_neg, total_voltage_sources + i, 0);
		}

		A.setFromTriplets(triplets.begin(), triplets.end());
	}


	/* For a resistor, do the following adjustments to the static_matrix (A):
	 * A(<->,<+>) -= 1/resistance
	 * A(<+>,<->) -= 1/resistance
	 * A(<+>,<+>) += 1/resistance
	 * A(<->,<->) += 1/resistance
	 * Where <+>,<-> are the positive and negative nodes of the resistor
	 */
	void MNASparseSystem::add_resistor_stamp(std::vector<Eigen::Triplet<double>> &triplets,
												node_id_t node_positive, node_id_t node_negative,
												float value)
	{
		double conductance = 1.0 / value;
		if (node_positive > 0 && node_negative > 0) {
			triplets.push_back(Eigen::Triplet<double>(node_positive-1, node_negative-1, -conductance));
			triplets.push_back(Eigen::Triplet<double>(node_negative-1, node_positive-1, -conductance));
		}
		if (node_positive > 0) {
			triplets.push_back(Eigen::Triplet<double>(node_positive-1, node_positive-1, conductance));
		}
		if (node_negative > 0) {
			triplets.push_back(Eigen::Triplet<double>(node_negative-1, node_negative-1, conductance));
		}
	}

	/* For a current source, do the following adjustments to the source vector (b):
	 * b(<+>) -= value
	 * b(<->) += value
	 * Where <+>,<-> are the positive and negative nodes of the current source
	 */
	void MNASparseSystem::add_current_source_stamp(node_id_t node_positive, node_id_t node_negative, float value) {
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
	void MNASparseSystem::add_voltage_source_stamp(std::vector<Eigen::Triplet<double>> &triplets,
													node_id_t node_positive, node_id_t node_negative,
													int voltage_src_id, float value)
	{
		int matrix_voltage_idx = total_nodes - 1 + voltage_src_id;
		b(matrix_voltage_idx) = value;
		if (node_positive > 0) {
			triplets.push_back(Eigen::Triplet<double>(matrix_voltage_idx, node_positive - 1, 1));
			triplets.push_back(Eigen::Triplet<double>(node_positive - 1, matrix_voltage_idx, 1));
		}
		if (node_negative > 0) {
			triplets.push_back(Eigen::Triplet<double>(matrix_voltage_idx, node_negative - 1, -1));
			triplets.push_back(Eigen::Triplet<double>(node_negative - 1, matrix_voltage_idx, -1));
		}
	}

	void MNASparseSystemTransient::create_initial_tran_system()
	{
		int total_inductors = netlist.inductors.size();
		int total_capacitors = netlist.capacitors.size();
		int total_voltage_sources = netlist.voltage_sources.size();
		int node_pos, node_neg;
		std::vector<Eigen::Triplet<double>> triplets;
		triplets.reserve(4 * total_capacitors + total_inductors);

		// Fill the matrix with the stamps of the capacitors
		for (auto it = netlist.capacitors.elements.begin(); it != netlist.capacitors.elements.end(); ++it) {
			add_capacitor_stamp(triplets, it->node_positive, it->node_negative, it->value);
		}

		// Fill the matrix with the stamps of the inductors
		for (int i = 0; i < total_inductors; i++) {
			add_inductor_stamp(triplets, total_voltage_sources + i, netlist.inductors.elements[i].value);
		}

		C.setFromTriplets(triplets.begin(), triplets.end());
	}

	/*
	 * Calculates the A = G + C / time_step matrix for a Transient Analysis run
	 */
	void MNASparseSystemTransient::create_tran_system(double time_step)
	{
		if (transient_method == BE) {
			mna_sparse_system.A = G + C / time_step;
		} else if (transient_method == TR) {
			mna_sparse_system.A = G + C * (2 / time_step);
		}
	}

	/*
	 * Update the b part of the Transient Equation for the Backward Euler method
	 */
	void MNASparseSystemTransient::update_tran_system_be(Eigen::VectorXd &e, double time_step)
	{
		mna_sparse_system.b = e + (C * mna_sparse_system.x) / time_step;
	}

	/*
	 * Update the b part of the Transient Equation for the Trapezoidal method
	 */
	void MNASparseSystemTransient::update_tran_system_tr(Eigen::VectorXd &e_new, Eigen::VectorXd &e_old, double time_step)
	{
		mna_sparse_system.b = e_new + e_old - (G - C * (2.0 / time_step)) * mna_sparse_system.x;
	}

	/* Adds capacitor stamps for the transient part (C) of the MNA system */
	void MNASparseSystemTransient::add_capacitor_stamp(std::vector<Eigen::Triplet<double>> &triplets,
													node_id_t node_positive, node_id_t node_negative, float value)
	{
		if (node_positive > 0 && node_negative > 0) {
			triplets.push_back(Eigen::Triplet<double>(node_positive-1, node_negative-1, -value));
			triplets.push_back(Eigen::Triplet<double>(node_negative-1, node_positive-1, -value));
		}
		if (node_positive > 0) {
			triplets.push_back(Eigen::Triplet<double>(node_positive-1, node_positive-1, value));
		}
		if (node_negative > 0) {
			triplets.push_back(Eigen::Triplet<double>(node_negative-1, node_negative-1, value));
		}
	}

	/* Adds inductor stamps for the transient part (C) of the MNA system */
	void MNASparseSystemTransient::add_inductor_stamp(std::vector<Eigen::Triplet<double>> &triplets,
													int voltage_src_id, float value)
	{
		int matrix_voltage_idx = mna_sparse_system.total_nodes - 1 + voltage_src_id;
		triplets.push_back(Eigen::Triplet<double>(matrix_voltage_idx, matrix_voltage_idx, -value));
	}
}