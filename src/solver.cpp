#include <cmath>
#include <Eigen/LU>
#include <Eigen/Cholesky>
#include <iostream>

#include "system.h"
#include "commands.h"
#include "util.h"
#include "solver.h"

namespace spic {
	void Solver::LU_custom_decompose() { }
	void Solver::LU_custom_solve() { }

	void Solver::LU_integrated_decompose()
	{
		logger.log(INFO, "LU_integrated_decompose called.");
		// TODO: How to check if LU was successfull
		lu = new Eigen::PartialPivLU<Eigen::Ref<Eigen::MatrixXd>>(system.A);
		// lu = new Eigen::FullPivLU<Eigen::MatrixXd>(system.A);
	}

	void Solver::LU_integrated_solve(Eigen::VectorXd &b)
	{
		logger.log(INFO, "LU_integrated_solve called.");
		// int rank = lu->rank();
		// if (rank != system.n) {
		// 	logger.log(ERROR, "Rank of MNA is " + std::to_string(rank) + " which is not equal to dimension " + std::to_string(system.n));
		// 	logger.log(ERROR, "The system does not have a unique solution");
		// }
		system.x = lu->solve(b);
	}

	bool Solver::cholesky_integrated_decompose()
	{
		logger.log(INFO, "cholesky_integrated_decompose called.");
		cholesky = new Eigen::LLT<Eigen::Ref<Eigen::MatrixXd>>(system.A);
		if (cholesky->info() != Eigen::Success) {
			logger.log(WARNING, "Cholesky decomposition failed, MNA System is not SPD.");
			return false;
		}
		return true;
	}

	void Solver::cholesky_integrated_solve(Eigen::VectorXd &b)
	{
		logger.log(INFO, "cholesky_integrated_solve called.");
		system.x = cholesky->solve(b);
	}
	
			
	bool Solver::cholesky_custom_decompose() { return true;}
	void Solver::cholesky_custom_solve(Eigen::VectorXd &b) {}

	// Functions
	bool Solver::decompose()
	{
		bool res;

		switch (method)
		{
		case CHOLESKY:
			if (custom) {
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
			if (custom) {
				LU_custom_decompose();
			} else {
				LU_integrated_decompose();
			}
			break;
		default:
			break;
		}

		return true;
	}

	void Solver::solve(Eigen::VectorXd &b)
	{
		switch (method)
		{
		case CHOLESKY:
			if (custom) {
				cholesky_custom_solve(b);
			} else {
				cholesky_integrated_solve(b);
			}
			break;
		case LU:
			if (custom) {
				LU_custom_solve();
			} else {
				LU_integrated_solve(b);
			}
			break;

		default:
			break;
		}
	}
}