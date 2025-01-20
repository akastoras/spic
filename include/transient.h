#pragma once

#include <vector>
#include <cmath>
#include <cassert>
#include <ostream>

namespace spic {
	/* Transiet Specifcation of Source elements */
	class TransientSpecs {
		public:
		enum TranType {EXP, SIN, PULSE, PWL} type;

		/* Method parameters */
		union {
			struct { // EXP
				double i1, i2, td1, tc1, td2, tc2;
				double idiff; // Internal constant variables
			} exp;

			struct { // SIN
				double i1, ia, fr, td, df, ph;
				double initial_phase, omega;
			} sin;

			struct { // PULSE
				double i1, i2, td, tr, tf, pw, per;
				double diff, peak, fall_start, fall_end;
			} pulse;

			struct { // PWL
				std::vector<std::pair<float, float>> *points;
				std::vector<double> slopes;
			} pwl;
		};

		// EXP & SIN constructor
		TransientSpecs(TranType type, float f1, float f2, float f3, float f4, float f5, float f6) : type(type)
		{
			if (type == EXP) {
				exp = {f1, f2, f3, f4, f5, f6};
				exp.idiff = exp.i2 - exp.i1;
			} else if (type == SIN) {
				sin = {f1, f2, f3, f4, f5, f6};
				sin.initial_phase = (2 * M_PI * sin.ph / 360.0);
				sin.omega = 2 * M_PI * sin.fr;
			} else {
				assert(0); // Throw error, should not reach here
			}
		}

		// PULSE constructor
		TransientSpecs(TranType type, float f1, float f2, float f3, float f4, float f5, float f6, float f7) : type(type)
		{
			assert(type == PULSE);
			pulse = {f1, f2, f3, f4, f5, f6, f7};
			pulse.diff = pulse.i2 - pulse.i1;
			pulse.peak = pulse.td + pulse.tr;
			pulse.fall_start = pulse.peak + pulse.pw;
			pulse.fall_end = pulse.fall_start + pulse.tf;
		}

		// PWL constructor
		TransientSpecs(TranType type, std::vector<std::pair<float, float>> *points) : type(type)
		{
			assert(type == PWL);
			pwl.points = points;

			for (int i = 0; i < pwl.points->size() - 1; i++) {
				double dt = (*pwl.points)[i + 1].first - (*pwl.points)[i].first;
				double dv = (*pwl.points)[i + 1].second - (*pwl.points)[i].second;
				pwl.slopes.push_back(dv / dt);
			}
		}

		~TransientSpecs()
		{
			if (type == PWL) {
				delete pwl.points;
			}
		}

		double eval(double t);
		private:
		double exp_eval(double t);
		double sin_eval(double t);
		double pulse_eval(double t);
		double pwl_eval(double t);
	};

	/* Transient Analysis Command Struct */
	class TransientAnalysis {
		public:
		double time_step;
		double fin_time;

		TransientAnalysis(double time_step, double fin_time) :
			time_step(time_step), fin_time(fin_time) {}
	};
}

std::ostream& operator<<(std::ostream &out, const spic::TransientSpecs &transient_specs);
std::ostream& operator<<(std::ostream &out, const spic::TransientAnalysis &transient_analysis);
std::ostream& operator<<(std::ostream &out, const std::vector<spic::TransientAnalysis> &transient_list);