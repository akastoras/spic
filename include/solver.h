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
		Eigen::VectorXi *perm;
		bool successful_decomposition;

		union {
			Eigen::PartialPivLU<Eigen::Ref<Eigen::MatrixXd>> *lu;
			Eigen::LLT<Eigen::Ref<Eigen::MatrixXd>> *cholesky;
		};

		struct {
			double secs_in_solve_calls;
			double secs_in_decompose_calls;
			int solve_calls;
			int decompose_calls;
		} perf_counter;

		Solver(System &system, Logger &logger)
			: Solver(system, LU, logger) {}
		Solver(System &system, method_t method, Logger &logger)
			: Solver(system, method, false, logger) {}
		Solver(System &system, bool spd, bool custom, Logger &logger)
			: system(system), method(spd ? CHOLESKY : LU), custom(custom), logger(logger) {
				perf_counter.secs_in_decompose_calls = 0;
				perf_counter.secs_in_solve_calls = 0;
				perf_counter.solve_calls = 0;
				perf_counter.decompose_calls = 0;
			}
		~Solver() {}

		/* Wrappers for decompose and solve */
		bool decompose();
		void solve(Eigen::VectorXd &b);
		void dump_perf_counters(std::filesystem::path &filename);

		private:
		/* LU custom and integrated decompose and solve functions*/
		bool LU_custom_decompose();
		void LU_custom_solve(Eigen::VectorXd &b);
		bool LU_integrated_decompose();
		void LU_integrated_solve(Eigen::VectorXd &b);

		/* Cholesky custom and integrated decompose and solve functions*/
		bool cholesky_integrated_decompose();
		void cholesky_integrated_solve(Eigen::VectorXd &b);
		bool cholesky_custom_decompose();
		void cholesky_custom_solve(Eigen::VectorXd &b);
	};
}
