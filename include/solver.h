#pragma once

#include <iostream>

#include "system.h"
#include "util.h"

namespace spic {
	class Solver {
		public:
		// Variables states klp
		typedef enum {LU, CHOLESKY} method_t;
		method_t method;
		bool custom;
		System &system;
		Logger &logger;
		union {
			Eigen::PartialPivLU<Eigen::Ref<Eigen::MatrixXd>> *lu;
			Eigen::LLT<Eigen::Ref<Eigen::MatrixXd>> *cholesky;
		};

		Solver(System &system, Logger &logger)
			: Solver(system, LU, logger) {}
		Solver(System &system, method_t method, Logger &logger)
			: Solver(system, method, false, logger) {}
		Solver(System &system, bool spd, bool custom, Logger &logger)
			: system(system), method(spd ? CHOLESKY : LU), custom(custom), logger(logger) {}
		~Solver() {}

		/* Wrappers for decompose and solve */
		bool decompose();
		void solve(Eigen::VectorXd &b);
		
		private:
		/* LU custom and integrated decompose and solve functions*/
		void LU_custom_decompose();
		void LU_custom_solve();
		void LU_integrated_decompose();
		void LU_integrated_solve(Eigen::VectorXd &b);

		/* Cholesky custom and integrated decompose and solve functions*/
		bool cholesky_integrated_decompose();
		void cholesky_integrated_solve(Eigen::VectorXd &b);
		bool cholesky_custom_decompose();
		void cholesky_custom_solve(Eigen::VectorXd &b);
	};
}
