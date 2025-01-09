#include <cmath>
#include <iostream>
#include <omp.h>

#include <Eigen/LU>
#include <Eigen/Cholesky>
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/SparseLU>
#include <Eigen/SparseCholesky>

#include "system.h"
#include "commands.h"
#include "util.h"
#include "solver.h"

namespace spic {
	bool Solver::LU_integrated_decompose()
	{
		if (options.sparse) {
			logger.log(INFO, "LU_integrated_decompose: called with a sparse system.");
			sparse_lu = new Eigen::SparseLU<Eigen::SparseMatrix<double>>(sparse_system->A);
		} else {
			logger.log(INFO, "LU_integrated_decompose: called with a dense system.");
			lu = new Eigen::PartialPivLU<Eigen::Ref<Eigen::MatrixXd>>(system->A);
		}
		return true;
	}

	void Solver::LU_integrated_solve(const Eigen::VectorXd &b)
	{
		if (options.sparse) {
			if (!successful_decomposition || !sparse_lu) {
				logger.log(ERROR, "LU_integrated_solve(): called without a decomposition.");
			}
			sparse_system->x = sparse_lu->solve(b);
		} else {
			if (!successful_decomposition || !lu) {
				logger.log(ERROR, "LU_integrated_solve(): called without a decomposition.");
			}
			system->x = lu->solve(b);
		}
	}

	bool Solver::cholesky_integrated_decompose()
	{
		if (options.sparse) {
			logger.log(INFO, "cholesky_integrated_decompose: called with a sparse system.");
			sparse_cholesky = new Eigen::SimplicialLLT<Eigen::SparseMatrix<double>, Eigen::Lower, Eigen::COLAMDOrdering<int>>(sparse_system->A);
			if (sparse_cholesky->info() != Eigen::Success) {
				logger.log(WARNING, "cholesky_integrated_decomposition(): failed, MNA System is not SPD.");
				return false;
			}
		} else {
			logger.log(INFO, "cholesky_integrated_decompose called with a dense system.");
			cholesky = new Eigen::LLT<Eigen::Ref<Eigen::MatrixXd>>(system->A);
			if (cholesky->info() != Eigen::Success) {
				logger.log(WARNING, "cholesky_integrated_decomposition(): failed, MNA System is not SPD.");
				return false;
			}
		}
		return true;
	}

	void Solver::cholesky_integrated_solve(const Eigen::VectorXd &b)
	{

		if (options.sparse) {
			if (!successful_decomposition || sparse_cholesky->info() != Eigen::Success) {
				logger.log(ERROR, "cholesky_integrated_solve(): called without a successful decomposition.");
			}
			sparse_system->x = sparse_cholesky->solve(b);
		} else {
			if (!successful_decomposition || cholesky->info() != Eigen::Success) {
				logger.log(ERROR, "cholesky_integrated_solve(): called without a successful decomposition.");
				return;
			}
			system->x = cholesky->solve(b);
		}
	}

	bool Solver::LU_custom_decompose()
	{
		logger.log(INFO, "LU_custom_decompose(): called.");

		Eigen::MatrixXd &A = system->A;
		int n = system->n;

		// Initialize permutation vector
		for (int i = 0; i < n; i++) {
			(*perm)(i) = i;
		}

		for (int k = 0; k < n; k++) {
			int pivot = k;
			double max_val = std::abs(A(k,k));

			// Find pivot row
			for (int i = k + 1; i < n; i++) {
				if (std::abs(A(i,k)) > max_val) {
					max_val = A(i,k);
					pivot = i;
				}
			}

			// Swap rows in A and permutation vector
			if (pivot != k) {
				A.row(k).swap(A.row(pivot));
				std::swap((*perm)[k], (*perm)[pivot]);
			}

			// Check for singular matrix
			if (A(k, k) == 0) {
				logger.log(ERROR, "LU_custom_decompose(): Singular matrix in LU, cannot proceed.");
				return false;
			}

			// Compute kth column of L matrix
			for (int i = k + 1; i < n; i++) {
				A(i,k) /= A(k,k);
			}

			// Update the rest of the matrix
			for (int i = k + 1; i < n; i++) {
				for (int j = k + 1; j < n; j++) {
					A(i,j) -= A(i,k)*A(k,j);
				}
			}
		}
		return true;
	}

	void Solver::LU_custom_solve(const Eigen::VectorXd &b)
	{
		if (!successful_decomposition || !perm) {
			logger.log(ERROR, "LU_custom_solve(): called without a successful decomposition.");
			return;
		}
		// Storing y directly on x, since x's initial value will be y
		Eigen::VectorXd &x = system->x;
		Eigen::MatrixXd &A = system->A;

		// Forward substitution
		for (int i = 0; i < system->n; i++) {
			x(i) = b((*perm)(i));
			for (int j = 0; j < i; j++) {
				x(i) -= A(i,j)*x(j);
			}
			// x(i) = x(i) / L(i,i) == x(i) / 1 == x(i)
		}

		// Backward substitution
		for (int i = system->n - 1; i >= 0; i--) {
			for (int j = i + 1; j < system->n; j++) {
				x(i) -= A(i,j)*x(j);
			}
			x(i) /= A(i,i);
		}
	}

	bool Solver::cholesky_custom_decompose()
	{
		logger.log(INFO, "cholesky_custom_decompose(): called.");
		Eigen::MatrixXd &A = system->A;
		double squaredElement;

		for (int k = 0; k < system->n; k++) {
			squaredElement = A(k,k) - A.row(k).head(k).squaredNorm();
			if (squaredElement < 0) {
				logger.log(ERROR, "cholesky_custom_decompose(): failed, MNA System is not SPD.");
				return false;
			}

			A(k,k) = std::sqrt(squaredElement);

			// Check for singular matrix
			if (A(k, k) == 0) {
				logger.log(ERROR, "cholesky_custom_decompose(): Singular matrix in cholesky, cannot proceed.");
				return false;
			}

			for (int i = k + 1; i < system->n; i++) {
				// Write only L matrix
				A(i,k) = (A(i,k) - A.row(i).head(k).dot(A.row(k).head(k))) / A(k,k);
			}
		}

		return true;
	}

	void Solver::cholesky_custom_solve(const Eigen::VectorXd &b)
	{
		if (!successful_decomposition) {
			logger.log(ERROR, "cholesky_custom_solve(): called without a successful decomposition.");
			return;
		}
		// Storing y directly on x, since x's initial value will be y
		Eigen::VectorXd &x = system->x;
		Eigen::MatrixXd &A = system->A;

		// Forward substitution
		for (int i = 0; i < system->n; i++) {
			x(i) = b(i);
			for (int j = 0; j < i; j++) {
				x(i) -= A(i,j)*x(j);
			}
			x(i) /= A(i,i);
		}

		// Backward substitution
		for (int i = system->n - 1; i >= 0; i--) {
			for (int j = i + 1; j < system->n; j++) {
				// Algorithmically, accessing A(i,j) is equivalent to A(j,i)
				// since A is symmetric, and since we don't store L^T
				// we access A(j,i) instead of A(i,j)
				x(i) -= A(j,i)*x(j);
			}
			x(i) /= A(i,i);
		}
	}


	/* CG Method integrated compute */
	void Solver::CG_integrated_compute()
	{
		if (options.sparse) {
			logger.log(INFO, "CG_integrated_compute(): called with a sparse system.");
			sparse_cg = new Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Lower|Eigen::Upper>(sparse_system->A);
			sparse_cg->setTolerance(options.itol);
		} else {
			logger.log(INFO, "CG_integrated_compute(): called with a dense system.");
			cg = new Eigen::ConjugateGradient<Eigen::MatrixXd, Eigen::Lower|Eigen::Upper>(system->A);
			cg->setTolerance(options.itol);
		}
	}

	void Solver::CG_integrated_solve(const Eigen::VectorXd &b)
	{
		if (options.sparse) {
			sparse_system->x = sparse_cg->solveWithGuess(b, sparse_system->x);
			iterations = sparse_cg->iterations();
			error = sparse_cg->error();
		} else {
			system->x = cg->solveWithGuess(b, system->x);
			iterations = cg->iterations();
			error = cg->error();
		}
	}

	/* CG Method custom compute*/
	void Solver::CG_custom_compute()
	{
		logger.log(INFO, "CG_custom_compute(): called.");

		if (options.sparse) {
			// Calculate the diagonal matrix of preconditioner
			inv_precond = new Eigen::VectorXd(sparse_system->n);
			inv_precond->setOnes();
			for (int k = 0; k < sparse_system->A.outerSize(); ++k) {
				for (Eigen::SparseMatrix<double>::InnerIterator it(sparse_system->A, k); it; ++it) {
					if (it.row() == it.col()) {
						if (it.value() >= EPS) {
							(*inv_precond)[it.row()] = 1.0 / it.value();
						}
					}
				}
			}
		} else {
			// Calculate the diagonal matrix of preconditioner
			inv_precond = new Eigen::VectorXd(system->n);
			(*inv_precond) = system->A.diagonal().array().inverse();
			for (int i = 0; i < system->n; i++) {
				if (system->A(i,i) < EPS) {
					(*inv_precond)(i) = 1;
				}
			}
		}
	}

	void Solver::CG_custom_solve(const Eigen::VectorXd &b)
	{
		if (!inv_precond) {
			logger.log(ERROR, "CG_custom_solve(): called without a preconditioner.");
			return;
		}

		int cg_iter = 0;
		double alpha, beta, rho, rho1, cg_error = options.itol + 1;

		int n = (options.sparse) ? sparse_system->n : system->n;
		Eigen::VectorXd &x = (options.sparse) ? sparse_system->x : system->x;
		Eigen::VectorXd r;
		if (options.sparse) {
			r = b - sparse_system->A * x;
		} else {
			r = b - system->A * x;
		}
		Eigen::VectorXd z(n);
		Eigen::VectorXd p(n);
		Eigen::VectorXd q(n);

		double bnorm = b.norm();
		if (bnorm < EPS) {
			x.setZero();
			return;
		}

		while (cg_error > options.itol && cg_iter < n) {
			cg_iter++;
			z = r.cwiseProduct(*inv_precond); // subroutine
			rho = r.dot(z);

			if (cg_iter == 1) {
				p = z;
			} else {
				beta = rho / rho1;
				p = z + beta*p;
			}
			rho1 = rho;

			if (options.sparse) { // subroutine
				q = sparse_system->A*p;
			} else {
				q = system->A*p;
			}
			
			alpha = rho / p.dot(q);
			x += alpha*p;
			r -= alpha*q;

			// Check for convergence
			cg_error = r.norm() / bnorm;
		}

		iterations = cg_iter;
		error = cg_error;

		// Values lower than itol should be considered as 0
		prune_output_vector();
	}


	/* BiCG Method integrated implementation */
	void Solver::BiCG_integrated_compute()
	{
		if (options.sparse) {
			logger.log(INFO, "BiCG_integrated_compute(): called with a sparse system.");
			sparse_bicg = new Eigen::BiCGSTAB<Eigen::SparseMatrix<double>>(sparse_system->A);
			sparse_bicg->setTolerance(options.itol);
		} else {
			logger.log(INFO, "BiCG_integrated_compute(): called with a dense system.");
			bicg = new Eigen::BiCGSTAB<Eigen::MatrixXd>(system->A);
			bicg->setTolerance(options.itol);
		}
	}

	void Solver::BiCG_integrated_solve(const Eigen::VectorXd &b)
	{
		if (options.sparse) {
			sparse_system->x = sparse_bicg->solveWithGuess(b, sparse_system->x);
			iterations = sparse_bicg->iterations();
			error = sparse_bicg->error();
		} else {
			system->x = bicg->solveWithGuess(b, system->x);
			iterations = bicg->iterations();
			error = bicg->error();
		}
	}

	/* BiCG Method custom implementation */
	void Solver::BiCG_custom_compute()
	{
		logger.log(INFO, "BiCG_custom_compute(): called.");

		if (options.sparse) {
			// Calculate the diagonal matrix of preconditioner
			inv_precond = new Eigen::VectorXd(sparse_system->n);
			inv_precond->setOnes();
			for (int k = 0; k < sparse_system->A.outerSize(); ++k) {
				for (Eigen::SparseMatrix<double>::InnerIterator it(sparse_system->A, k); it; ++it) {
					if (it.row() == it.col()) {
						if (it.value() >= EPS) {
							(*inv_precond)[it.row()] = 1.0 / it.value();
						}
					}
				}
			}
		} else {
			// Calculate the diagonal matrix of preconditioner
			inv_precond = new Eigen::VectorXd(system->n);
			(*inv_precond) = system->A.diagonal().array().inverse();
			for (int i = 0; i < system->n; i++) {
				if (system->A(i,i) < EPS) {
					(*inv_precond)(i) = 1;
				}
			}
		}
	}

	bool Solver::BiCG_custom_solve(const Eigen::VectorXd &b)
	{
		if (!inv_precond) {
			logger.log(ERROR, "BiCG_custom_solve(): called without a preconditioner.");
			return false;
		}

		int bicg_iter = 0;
		double alpha, beta, omega, rho, rho1, bicg_error = options.itol + 1;

		int n = (options.sparse) ? sparse_system->n : system->n;
		Eigen::VectorXd &x = (options.sparse) ? sparse_system->x : system->x;
		Eigen::VectorXd r;

		if (options.sparse) {
			r = b - sparse_system->A * x;
		} else {
			r = b - system->A * x;
		}

		Eigen::VectorXd r_tilda = r;
		Eigen::VectorXd z(n), z_tilda(n);
		Eigen::VectorXd p(n), p_tilda(n);
		Eigen::VectorXd q(n), q_tilda(n);

		double bnorm = b.norm();
		if (bnorm < EPS) {
			x.setZero();
			return true;
		}

		while (bicg_error > options.itol && bicg_iter < n) {

			bicg_iter++;
			z = r.cwiseProduct(*inv_precond); // subroutine
			z_tilda = r_tilda.cwiseProduct(*inv_precond); // subroutine
			rho = r_tilda.dot(z);

			if (abs(rho) < EPS) {
				return false;
			}

			if (bicg_iter == 1) {
				p = z;
				p_tilda = z_tilda;
			} else {
				beta = rho / rho1;
				p = z + beta*p;
				p_tilda = z_tilda + beta*p_tilda;
			}
			rho1 = rho;

			if (options.sparse) {
				q = sparse_system->A * p; // subroutine
				q_tilda = sparse_system->A.transpose() * p_tilda; // subroutine
			} else {
				q = system->A * p;
				q_tilda = system->A.transpose() * p_tilda;
			}

			omega = p_tilda.dot(q);
			if (abs(omega) < EPS) {
				return false;
			}

			alpha = rho / omega;
			x += alpha*p;
			r -= alpha*q;
			r_tilda -= alpha*q_tilda;

			bicg_error = r.norm() / bnorm;
		}

		iterations = bicg_iter;
		error = bicg_error;

		// Values lower than itol should be considered as 0
		prune_output_vector();
		
		return true;
	}

	/* Wrapper functions */

	/* Decompose is called before solve for direct methods */
	bool Solver::decompose()
	{
		double start = omp_get_wtime();
		bool res;

		switch (method)
		{
		case CHOLESKY:
			if (options.custom) {
				res = cholesky_custom_decompose();
			} else {
				res = cholesky_integrated_decompose();
			}
			if (!res) {
				return false;
			} else {
				break;
			}
		case LU:
			if (options.custom) {
				res = LU_custom_decompose();
			} else {
				res = LU_integrated_decompose();
			}
			break;
		default:
			logger.log(ERROR, "decompose(): Invalid method.");
			exit(1);
		}

		successful_decomposition = res;
		perf_counter.secs_in_decompose_calls += omp_get_wtime() - start;
		perf_counter.decompose_calls++;
		return res;
	}

	/* Compute is called before solve for Iterative methods */
	void Solver::compute()
	{
		double start = omp_get_wtime();

		switch (method)
		{
		case BiCG:
			if (options.custom) {
				BiCG_custom_compute();
			} else {
				BiCG_integrated_compute();
			}
			break;
		case CG:
			if (options.custom) {
				CG_custom_compute();
			} else {
				CG_integrated_compute();
			}
			break;
		default:
			logger.log(ERROR, "compute(): Invalid method.");
			exit(1);
		}

		perf_counter.secs_in_compute_calls += omp_get_wtime() - start;
		perf_counter.compute_calls++;
	}

	/* solve() is called with a b to solve the system
	 *  - For iterative methods solve() must be called after compute()
	 *  - For direct methods solve() must be called after decompose()
	 */
	void Solver::solve(const Eigen::VectorXd &b)
	{
		double start = omp_get_wtime();
		bool res;

		switch (method)
		{
		case CHOLESKY:
			if (options.custom) {
				cholesky_custom_solve(b);
			} else {
				cholesky_integrated_solve(b);
			}
			break;
		case LU:
			if (options.custom) {
				LU_custom_solve(b);
			} else {
				LU_integrated_solve(b);
			}
			break;
		case CG:
			if (options.custom) {
				CG_custom_solve(b);
			} else {
				CG_integrated_solve(b);
			}
			logger.log(INFO, "CG: Error was " + std::to_string(error) + " in "
									+ std::to_string(iterations) + " Iterations: ");
			break;
		case BiCG:
			if (options.custom) {
				res = BiCG_custom_solve(b);
				if (!res) {
					logger.log(ERROR, "BiCG_custom_solve(): failed.");
				}
			} else {
				BiCG_integrated_solve(b);
			}

			logger.log(INFO, "BiCG: Error was " + std::to_string(error) + " in "
									+ std::to_string(iterations) + " Iterations: ");
			break;
		default:
			logger.log(ERROR, "solve(): Invalid method.");
			exit(1);
		}

		perf_counter.secs_in_solve_calls += omp_get_wtime() - start;
		perf_counter.solve_calls++;
	}
	
	/* Make values less than the specified tolerance equal to zero */ 
	void Solver::prune_output_vector()
	{
		Eigen::VectorXd &x = (options.sparse) ? sparse_system->x : system->x;

		assert(options.iter == true);

		for (int i = 0; i < system->n; i++) {
			if (std::abs(x(i)) < options.itol) {
				x(i) = 0;
			}
		}
	}

	/* Dump performance counters to a file */
	void Solver::dump_perf_counters(std::filesystem::path &filename, double g_time)
	{
		std::ofstream file(filename.string(), std::ofstream::out);
		file << "secs_in_decompose:\t" << perf_counter.secs_in_decompose_calls << std::endl;
		file << "secs_in_compute:\t" << perf_counter.secs_in_compute_calls << std::endl;
		file << "secs_in_solve:\t" << perf_counter.secs_in_solve_calls << std::endl;
		file << "decompose_calls:\t" << perf_counter.decompose_calls << std::endl;
		file << "compute_calls:\t" << perf_counter.compute_calls << std::endl;
		file << "solve_calls:\t" << perf_counter.solve_calls << std::endl;
		file << "total_secs:\t" << g_time << std::endl;
		file.close();
	}
}

std::ostream& operator<<(std::ostream &out, const spic::options_t &options) {
	out << "\tCustom: " << (options.custom ? "Enabled" : "Disabled") << std::endl;
	out << "\tSPD: "  << (options.spd ? "Enabled" : "Disabled") << std::endl;
	out << "\tIter: " << (options.iter ? "Enabled" : "Disabled") << std::endl;
	out << "\tItol: " << options.itol << std::endl;
	return out;
}