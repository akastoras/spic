#pragma once

#include <iostream>
#include <filesystem>

#include <Eigen/LU>
#include <Eigen/Cholesky>
#include <Eigen/IterativeLinearSolvers>

#include "system.h"
#include "util.h"

#define EPS 1e-10

namespace spic {
	typedef struct options {
		bool custom; // Enable usage of custom implementations
		bool spd; // If iter=false: Enable cholesky decomp, else: Enable conjugate gradient
		bool iter; // Enables iterative methods (conjugate gradient & biconjugate gradient)
		double itol; // The convergence threshold for iterative methods
	} options_t;

	class Solver {
		public:
		typedef enum {LU, CHOLESKY, CG, BiCG} method_t;

		/* General variables */
		method_t method;

		/* References to global spic stuff */
		options_t &options;
		System &system;
		Logger &logger;

		// Algorithm specific variables
		union {
			Eigen::VectorXi *perm;
			Eigen::VectorXd *inv_precond;
		};

		union {
			// Direct
			bool successful_decomposition;

			// Iterative
			struct {
				int iterations;
				double error;
			};
		};

		union {
			Eigen::PartialPivLU<Eigen::Ref<Eigen::MatrixXd>> *lu;
			Eigen::LLT<Eigen::Ref<Eigen::MatrixXd>> *cholesky;
			Eigen::ConjugateGradient<Eigen::MatrixXd, Eigen::Lower|Eigen::Upper> *cg;
			Eigen::BiCGSTAB<Eigen::MatrixXd> *bicg;
		};

		struct {
			double secs_in_solve_calls;
			double secs_in_decompose_calls;
			double secs_in_compute_calls;
			int solve_calls;
			int decompose_calls;
			int compute_calls;
		} perf_counter;

		Solver(System &system, options_t &options, Logger &logger)
			: system(system), options(options), logger(logger)
		{
			perf_counter.secs_in_decompose_calls = 0;
			perf_counter.secs_in_compute_calls = 0;
			perf_counter.secs_in_solve_calls = 0;
			perf_counter.decompose_calls = 0;
			perf_counter.compute_calls = 0;
			perf_counter.solve_calls = 0;

			if (options.iter) {
				if (options.spd) {
					method = CG;
				} else {
					method = BiCG;
				}

				compute();
			} else {
				if (options.spd) {
					method = CHOLESKY;
				} else {
					method = LU;
				}

				if (!decompose()) {
					logger.log(ERROR, "Exiting due to non-SPD MNA system");
					exit(EXIT_FAILURE);
				}
			}
		}
		~Solver() {}

		/* Wrappers for decompose and solve */
		void solve(Eigen::VectorXd &b);
		void dump_perf_counters(std::filesystem::path &filename);

		private:
		bool decompose();
		void compute();

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

		/* Conjugate gradient custom and integrated solve functions */
		void CG_integrated_compute();
		void CG_integrated_solve(Eigen::VectorXd &b);
		void CG_custom_compute();
		void CG_custom_solve(Eigen::VectorXd &b);

		/* BiConjugate gradient custom and integrated solve functions */
		void BiCG_integrated_compute();
		void BiCG_integrated_solve(Eigen::VectorXd &b);
		void BiCG_custom_compute();
		bool BiCG_custom_solve(Eigen::VectorXd &b);
	};
}

std::ostream& operator<<(std::ostream &out, const spic::options_t &options);