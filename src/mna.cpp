#include <iomanip>
#include <iostream>
#include <Eigen/Core>
#include <Eigen/Dense>

#include "netlist.h"

#include "mna.h"

namespace spic {
	MNASystemDC::MNASystemDC(Netlist &netlist, int total_nodes) 
	: MNASystemDC(netlist, total_nodes, total_nodes - 1 + netlist.voltage_sources.size() + netlist.inductors.size()) {}

	MNASystemDC::MNASystemDC(Netlist &netlist, int total_nodes, int MNA_matrix_dim)
		: netlist(netlist), total_nodes(total_nodes),
		static_matrix(MNA_matrix_dim, MNA_matrix_dim),
		source_vector(MNA_matrix_dim)
	{
		int total_voltage_sources = netlist.voltage_sources.size();
		int total_inductors = netlist.inductors.size();
		int m2 = total_voltage_sources + total_inductors;
		int node_pos, node_neg, matrix_inductor_idx, matrix_voltage_idx;

		// Initialize matrices
		static_matrix.setZero();
		source_vector.setZero();

		// Fill the matrix with the stamps of the resistors
		for (auto it = netlist.resistors.elements.begin(); it != netlist.resistors.elements.end(); ++it) {
			if (it->node_positive > 0 && it->node_negative > 0) {
				static_matrix(it->node_positive-1, it->node_negative-1) -= 1 / it->value;
				static_matrix(it->node_negative-1, it->node_positive-1) -= 1 / it->value;
			}
			if (it->node_positive > 0) {
				static_matrix(it->node_positive-1, it->node_positive-1) += 1 / it->value;
			}
			if (it->node_negative > 0) {
				static_matrix(it->node_negative-1, it->node_negative-1) += 1 / it->value;
			}
		}

		// Fill the source vector with the stamps of the current sources
		for (auto it = netlist.current_sources.elements.begin(); it != netlist.current_sources.elements.end(); ++it) {
			if (it->node_positive > 0) {
				source_vector(it->node_positive - 1) -= it->value;
			}
			if (it->node_negative > 0) {
				source_vector(it->node_negative - 1) += it->value;
			}
		}
		
		// Fill the matrix with the stamps of the voltage sources
		for (int i = 0; i < total_voltage_sources; i++) {
			node_pos = netlist.voltage_sources.elements[i].node_positive;
			node_neg = netlist.voltage_sources.elements[i].node_negative;
			
			matrix_voltage_idx = total_nodes - 1 + i;
			
			source_vector(matrix_voltage_idx) = netlist.voltage_sources.elements[i].value;
			if (node_pos > 0) {
				static_matrix(matrix_voltage_idx, node_pos - 1) += 1;
				static_matrix(node_pos - 1, matrix_voltage_idx) += 1;
			}
			if (node_neg > 0) {
				static_matrix(matrix_voltage_idx, node_neg - 1) -= 1;
				static_matrix(node_neg - 1, matrix_voltage_idx) -= 1;
			}
		}

		// Fill the matrix with the stamps of the inductors
		for (int i = 0; i < total_inductors; i++) {
			// In DC  analysis consider inductors as Voltage Sources with 0 value
			node_pos = netlist.inductors.elements[i].node_positive;
			node_neg = netlist.inductors.elements[i].node_negative;
			
			matrix_inductor_idx = MNA_matrix_dim - total_inductors + i;
			
			source_vector(matrix_inductor_idx) = 0;
			if (node_pos > 0) {
				static_matrix(matrix_inductor_idx, node_pos - 1) += 1;
				static_matrix(node_pos - 1, matrix_inductor_idx) += 1;
			}
			if (node_neg > 0) {
				static_matrix(matrix_inductor_idx, node_neg - 1) -= 1;
				static_matrix(node_neg - 1, matrix_inductor_idx) -= 1;
			}
		}
	}
}

static void printSystem(const Eigen::MatrixXf& A, const Eigen::VectorXf& b) {
    int rows = A.rows();
    int midRow = rows / 2; // Middle row index for alignment
    int width = 5; // Fixed width for each element

    for (int i = 0; i < rows; ++i) {
        // Print row of A
        std::cout << "[";
        for (int j = 0; j < A.cols(); ++j) {
            std::cout << std::setw(width) << A(i, j);
            if (j < A.cols() - 1) std::cout << " ";
        }
        std::cout << "] ";

        // Print corresponding x
        std::cout << "[x" << i + 1 << "] ";

        // Print '=' only on the middle row
        if (i == midRow) {
            std::cout << "= ";
        } else {
            std::cout << "  ";
        }

        // Print corresponding b
        std::cout << "[" << std::setw(width) << b(i) << "]";

        std::cout << std::endl;
    }
}


std::ostream& operator<<(std::ostream &out, spic::MNASystemDC &system)
{
	printSystem(system.static_matrix, system.source_vector);
	// out << "Static Matrix:\n" << system.static_matrix << std::endl;
	// out << "Source Vector:\n" << system.source_vector << std::endl;
	return out;
}

