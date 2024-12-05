#pragma once

#include <cmath>
#include <Eigen/LU>
#include <Eigen/Cholesky>
#include <iostream>
#include "system.h"
#include "commands.h"
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
		Solver(System &system, options_t options, Logger &logger)
			: Solver(system, options.spd ? CHOLESKY : LU, options.custom, logger) {}
		Solver(System &system, method_t method, bool custom, Logger &logger)
			: system(system), method(method), custom(custom), logger(logger) {}
		~Solver() {}

		bool LU_integrated_decompose()
		{
			logger.log(INFO, "LU_integrated_decompose called.");
			// TODO: How to check if LU was successfull
			lu = new Eigen::PartialPivLU<Eigen::Ref<Eigen::MatrixXd>>(system.A);
			return true;
		}

		bool LU_integrated_solve(Eigen::VectorXd &b)
		{
			logger.log(INFO, "LU_integrated_solve called.");
			system.x = lu->solve(b);
			return true;
		}

		bool cholesky_integrated_decompose()
		{
			logger.log(INFO, "cholesky_integrated_decompose called.");
			cholesky = new Eigen::LLT<Eigen::Ref<Eigen::MatrixXd>>(system.A);
			return cholesky->info() == Eigen::Success;
		}

		bool cholesky_integrated_solve(Eigen::VectorXd &b)
		{
			logger.log(INFO, "cholesky_integrated_solve called.");
			system.x = cholesky->solve(b);
			return cholesky->info() == Eigen::Success;
		}

		// Functions
		bool decompose()
		{
			switch (method)
			{
			case LU:
				if (custom) {
					break;
				} else {
					LU_integrated_decompose();
					break;
				}

			case CHOLESKY:
				if (custom) {
					break;
				} else {
					cholesky_integrated_decompose();
					break;
				}

			default:
				break;
			}
			return false;
		}

		bool solve(Eigen::VectorXd &b)
		{
			switch (method)
			{
			case LU:
				if (custom) {
					break;
				} else {
					LU_integrated_solve(b);
					break;
				}
				break;

			case CHOLESKY:
				if (custom) {
					break;
				} else {
					cholesky_integrated_solve(b);
					break;
				}
				break;

			default:
				break;
			}

			return true;
		}

		private:
		// bool LU_custom_decompose() {
		// 	int n = system.A.rows();
		// 	for (int k = 0; k < n; k++) {
		// 		x = fabs(A(k,k));
		// 	}
		// }
	};
}
