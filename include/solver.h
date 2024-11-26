#pragma once

#include <cmath>
#include "system.h"
#include <Eigen/LU>
#include <Eigen/Cholesky>
#include <iostream>

namespace spic {
	class Solver {
		public:
		// Variables states klp
		enum {LU, CHOLESKY} method;
		bool custom;
		System &system;

		// Functions
		bool decompose() {
			switch (method)
			{
			case LU:
				LU_integrated_decompose();
				break;

			case CHOLESKY:
				break;

			default:
				break;
			}
			return false;
		}
		// bool solve(array y) {}

		private:
		// bool LU_custom_decompose() {
		// 	int n = system.A.rows();
		// 	for (int k = 0; k < n; k++) {
		// 		x = fabs(A(k,k));
		// 	}
		// }

		bool LU_integrated_decompose()
		{
			std::cout << "Here is the input matrix A before decomposition:\n" << system.A << "\n";
			Eigen::PartialPivLU<Eigen::Ref<Eigen::MatrixXd>> lu(system.A);
			std::cout << "Here is the input matrix A after decomposition:\n" << lu.matrixLU() << "\n";
			Eigen::PermutationMatrix<Eigen::Dynamic, Eigen::Dynamic, int> permutation = lu.permutationP();
			std::cout << "Here is the permutation matrix:\n" << permutation.indices() << "\n";
			// A = PLU
			return false;
		}
	};
}
