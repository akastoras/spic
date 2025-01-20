#pragma once

#include <iostream>
#include <filesystem>

#include <Eigen/LU>
#include <Eigen/Cholesky>
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/SparseLU>
#include <Eigen/SparseCholesky>

#include "system.h"
#include "util.h"

#define EPS 1e-23

namespace spic {
	typedef enum { BE, TR } transient_method_t;

	typedef struct options {
		bool custom; // Enable usage of custom implementations
		bool spd; // If iter=false: Enable cholesky decomp, else: Enable conjugate gradient
		bool iter; // Enables iterative methods (conjugate gradient & biconjugate gradient)
		bool sparse; // Enables the usage of sparse matrices
		double itol; // The convergence threshold for iterative methods
		transient_method_t transient_method; // Method for calculatg derivative in Transient Analysis
	} options_t;

	class Solver {
		public:
		typedef enum {LU, CHOLESKY, CG, BiCG} method_t;

		/* General variables */
		method_t method;

		/* References to global spic stuff */
		options_t &options;
		
		/* Pointer to dense/sparse system */
		union {
			System *system;
			SparseSystem *sparse_system;
		};
		
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
			// LU
			Eigen::PartialPivLU<Eigen::Ref<Eigen::MatrixXd>> *lu;
			Eigen::SparseLU<Eigen::SparseMatrix<double>> *sparse_lu;
			// Cholesky
			Eigen::LLT<Eigen::Ref<Eigen::MatrixXd>> *cholesky;
			Eigen::SimplicialLLT<Eigen::SparseMatrix<double>, Eigen::Lower, Eigen::COLAMDOrdering<int>> *sparse_cholesky;
			// CG
			Eigen::ConjugateGradient<Eigen::MatrixXd, Eigen::Lower|Eigen::Upper> *cg;
			Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Lower|Eigen::Upper> *sparse_cg;
			// BiCG
			Eigen::BiCGSTAB<Eigen::MatrixXd> *bicg;
			Eigen::BiCGSTAB<Eigen::SparseMatrix<double>> *sparse_bicg;
		};

		struct {
			double secs_in_solve_calls;
			double secs_in_decompose_calls;
			double secs_in_compute_calls;
			int solve_calls;
			int decompose_calls;
			int compute_calls;
		} perf_counter;

		Solver(SparseSystem &sparse_system, options_t &options, Logger &logger)
			: Solver(NULL, &sparse_system, options, logger) {}

		Solver(System &system, options_t &options, Logger &logger)
			: Solver(&system, NULL, options, logger) {}

		Solver(System *arg_system, SparseSystem *arg_sparse_system, options_t &options, Logger &logger)
			: options(options), logger(logger)
		{
			if (options.sparse) {
				sparse_system = arg_sparse_system;
			} else {
				system = arg_system;
			}

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
		void solve(const Eigen::VectorXd &b);
		void dump_perf_counters(std::filesystem::path &filename, double g_time);


		/* All functions except the constructor, solve and dump_performance_counters are private */
		private:
		bool decompose();
		void compute();

		/* LU custom and integrated decompose and solve functions*/
		bool LU_custom_decompose();
		void LU_custom_solve(const Eigen::VectorXd &b);
		bool LU_integrated_decompose();
		void LU_integrated_solve(const Eigen::VectorXd &b);

		/* Cholesky custom and integrated decompose and solve functions*/
		bool cholesky_integrated_decompose();
		void cholesky_integrated_solve(const Eigen::VectorXd &b);
		bool cholesky_custom_decompose();
		void cholesky_custom_solve(const Eigen::VectorXd &b);

		/* Conjugate gradient custom and integrated solve functions */
		void CG_integrated_compute();
		void CG_integrated_solve(const Eigen::VectorXd &b);
		void CG_custom_compute();
		void CG_custom_solve(const Eigen::VectorXd &b);

		/* BiConjugate gradient custom and integrated solve functions */
		void BiCG_integrated_compute();
		void BiCG_integrated_solve(const Eigen::VectorXd &b);
		void BiCG_custom_compute();
		bool BiCG_custom_solve(const Eigen::VectorXd &b);

		/* Helper functions */
		void prune_output_vector();

	};
}

std::ostream& operator<<(std::ostream &out, const spic::options_t &options);