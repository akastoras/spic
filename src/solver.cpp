#include <cmath>
#include <iostream>
#include <omp.h>

#include <Eigen/LU>
#include <Eigen/Cholesky>
#include <Eigen/IterativeLinearSolvers>

#include "system.h"
#include "commands.h"
#include "util.h"
#include "solver.h"

namespace spic {
	bool Solver::LU_integrated_decompose()
	{
		logger.log(INFO, "LU_integrated_decompose: called.");
		// TODO: How to check if LU was successfull
		// A matrix here is considered to be invertble and
		// and to check that Eigen allows to use .isInvertible() method
		// but only in FullPivLU. So we assume that the cirtuit matrices are invertible?
		lu = new Eigen::PartialPivLU<Eigen::Ref<Eigen::MatrixXd>>(system.A);
		return true;
	}

	void Solver::LU_integrated_solve(Eigen::VectorXd &b)
	{
		if (!successful_decomposition || !lu) {
			logger.log(ERROR, "LU_integrated_solve(): called without a decomposition.");
		}
		system.x = lu->solve(b);
	}

	bool Solver::cholesky_integrated_decompose()
	{
		logger.log(INFO, "cholesky_integrated_decompose called.");
		cholesky = new Eigen::LLT<Eigen::Ref<Eigen::MatrixXd>>(system.A);
		if (cholesky->info() != Eigen::Success) {
			logger.log(WARNING, "cholesky_integrated_decomposition(): failed, MNA System is not SPD.");
			return false;
		}
		return true;
	}

	void Solver::cholesky_integrated_solve(Eigen::VectorXd &b)
	{
		if (!successful_decomposition || cholesky->info() != Eigen::Success) {
			logger.log(ERROR, "cholesky_integrated_solve(): called without a successful decomposition.");
			return;
		}
		system.x = cholesky->solve(b);
	}


	bool Solver::LU_custom_decompose()
	{
		logger.log(INFO, "LU_custom_decompose(): called.");

		// Initialize permutation vector
		perm = new Eigen::VectorXi(system.n);
		Eigen::MatrixXd &A = system.A;

		// Initialize permutation vector
		for (int i = 0; i < system.n; i++) {
			(*perm)(i) = i;
		}

		for (int k = 0; k < system.n; k++) {
			int pivot = k;
			double max_val = std::abs(A(k,k));

			// Find pivot row
			for (int i = k + 1; i < system.n; i++) {
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
			for (int i = k + 1; i < system.n; i++) {
				A(i,k) /= A(k,k);
			}

			// Update the rest of the matrix
			for (int i = k + 1; i < system.n; i++) {
				for (int j = k + 1; j < system.n; j++) {
					A(i,j) -= A(i,k)*A(k,j);
				}
			}
		}
		return true;
	}

	void Solver::LU_custom_solve(Eigen::VectorXd &b)
	{
		if (!successful_decomposition || !perm) {
			logger.log(ERROR, "LU_custom_solve(): called without a successful decomposition.");
			return;
		}
		// Storing y directly on x, since x's initial value will be y
		Eigen::VectorXd &x = system.x;
		Eigen::MatrixXd &A = system.A;

		// Forward substitution
		for (int i = 0; i < system.n; i++) {
			x(i) = b((*perm)(i));
			for (int j = 0; j < i; j++) {
				x(i) -= A(i,j)*x(j);
			}
			// x(i) = x(i) / L(i,i) == x(i) / 1 == x(i)
		}

		// Backward substitution
		for (int i = system.n - 1; i >= 0; i--) {
			for (int j = i + 1; j < system.n; j++) {
				x(i) -= A(i,j)*x(j);
			}
			x(i) /= A(i,i);
		}
	}

	bool Solver::cholesky_custom_decompose()
	{
		logger.log(INFO, "cholesky_custom_decompose(): called.");
		Eigen::MatrixXd &A = system.A;
		double squaredElement;

		for (int k = 0; k < system.n; k++) {
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

			for (int i = k + 1; i < system.n; i++) {
				// Write only L matrix
				A(i,k) = (A(i,k) - A.row(i).head(k).dot(A.row(k).head(k))) / A(k,k);
			}
		}

		return true;
	}

	void Solver::cholesky_custom_solve(Eigen::VectorXd &b)
	{
		if (!successful_decomposition) {
			logger.log(ERROR, "cholesky_custom_solve(): called without a successful decomposition.");
			return;
		}
		// Storing y directly on x, since x's initial value will be y
		Eigen::VectorXd &x = system.x;
		Eigen::MatrixXd &A = system.A;

		// Forward substitution
		for (int i = 0; i < system.n; i++) {
			x(i) = b(i);
			for (int j = 0; j < i; j++) {
				x(i) -= A(i,j)*x(j);
			}
			x(i) /= A(i,i);
		}

		// Backward substitution
		for (int i = system.n - 1; i >= 0; i--) {
			for (int j = i + 1; j < system.n; j++) {
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
		logger.log(INFO, "CG_integrated_compute(): called.");
		cg = new Eigen::ConjugateGradient<Eigen::MatrixXd, Eigen::Lower|Eigen::Upper>(system.A);
		cg->setTolerance(options.itol);
	}

	void Solver::CG_integrated_solve(Eigen::VectorXd &b)
	{
		system.x = cg->solveWithGuess(b, system.x);
		// system.x = cg->solve(b);
		iterations = cg->iterations();
		error = cg->error();
	}


	/* CG Method custom compute*/
	void Solver::CG_custom_compute()
	{
		logger.log(INFO, "CG_custom_compute(): called.");

		// Calculate the diagonal matrix of preconditioner
		inv_precond = new Eigen::VectorXd(system.n);
		(*inv_precond) = system.A.diagonal().array().inverse();
	}

	void Solver::CG_custom_solve(Eigen::VectorXd &b)
	{
		if (!inv_precond) {
			logger.log(ERROR, "CG_custom_solve(): called without a preconditioner.");
			return;
		}

		int cg_iter = 0;
		double alpha, beta, rho, rho1, cg_error = options.itol + 1;

		Eigen::VectorXd r = b - system.A*system.x;
		Eigen::VectorXd z = r.cwiseProduct(*inv_precond);
		Eigen::VectorXd p(system.n);
		Eigen::VectorXd q(system.n);

		double bnorm = b.norm();
		if (bnorm < EPS) {
			system.x.setZero();
			return;
		}

		while (cg_error > options.itol && cg_iter < system.n) {
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
			q = system.A*p; // subroutine
			alpha = rho / p.dot(q);
			system.x += alpha*p;
			r -= alpha*q;

			// Check for convergence
			cg_error = r.norm() / bnorm;
		}

		iterations = cg_iter;
		error = cg_error;
	}


	/* BiCG Method integrated implementation */
	void Solver::BiCG_integrated_compute()
	{
		logger.log(INFO, "BiCG_integrated_compute(): called.");
		bicg = new Eigen::BiCGSTAB<Eigen::MatrixXd>(system.A);
		bicg->setTolerance(options.itol);
	}

	void Solver::BiCG_integrated_solve(Eigen::VectorXd &b)
	{
		system.x = bicg->solveWithGuess(b, system.x);
		iterations = bicg->iterations();
		error = bicg->error();
	}


	/* BiCG Method custom implementation */
	void Solver::BiCG_custom_compute()
	{
		logger.log(INFO, "BiCG_custom_compute(): called.");

		// Calculate the diagonal matrix of preconditioner
		inv_precond = new Eigen::VectorXd(system.n);
		(*inv_precond) = system.A.diagonal().array().inverse();
		for (int i = 0; i < system.n; i++) {
			if (system.A(i,i) < EPS) {
				(*inv_precond)(i) = 1;
			}
		}
	}

	bool Solver::BiCG_custom_solve(Eigen::VectorXd &b)
	{
		if (!inv_precond) {
			logger.log(ERROR, "BiCG_custom_solve(): called without a preconditioner.");
			return false;
		}

		int bicg_iter = 0;
		double alpha, beta, omega, rho, rho1, bicg_error = options.itol + 1;
		Eigen::VectorXd r = b - system.A*system.x;

		Eigen::VectorXd r_tilda = r;
		Eigen::VectorXd z(system.n), z_tilda(system.n);
		Eigen::VectorXd p(system.n), p_tilda(system.n);
		Eigen::VectorXd q(system.n), q_tilda(system.n);

		double bnorm = b.norm();
		if (bnorm < EPS) {
			system.x.setZero();
			return true;
		}

		while (bicg_error > options.itol && bicg_iter < system.n) {
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

			q = system.A*p; // subroutine
			q_tilda = system.A.transpose()*p_tilda; // subroutine

			omega = p_tilda.dot(q);
			if (abs(omega) < EPS) {
				return false;
			}

			alpha = rho / omega;
			system.x += alpha*p;
			r -= alpha*q;
			r_tilda -= alpha*q_tilda;

			bicg_error = r.norm() / bnorm;
		}

		iterations = bicg_iter;
		error = bicg_error;
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
	}

	/* solve() is called with a b to solve the system
	 *  - For iterative methods solve() must be called after compute()
	 *  - For direct methods solve() must be called after decompose()
	 */
	void Solver::solve(Eigen::VectorXd &b)
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

	/* Dump performance counters to a file */
	void Solver::dump_perf_counters(std::filesystem::path &filename)
	{
		std::ofstream file(filename.string(), std::ofstream::out);
		file << "secs_in_decompose: " << perf_counter.secs_in_decompose_calls << std::endl;
		file << "secs_in_compute: " << perf_counter.secs_in_compute_calls << std::endl;
		file << "secs_in_solve: " << perf_counter.secs_in_solve_calls << std::endl;
		file << "decompose_calls: " << perf_counter.decompose_calls << std::endl;
		file << "compute_calls: " << perf_counter.compute_calls << std::endl;
		file << "solve_calls: " << perf_counter.solve_calls << std::endl;
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