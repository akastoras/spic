#include <vector>
#include <cmath>
#include <cassert>
#include <ostream>

#include "transient.h"

namespace spic {
	double TransientSpecs::eval(double t) {
		switch (type)
		{
		case EXP:
			return exp_eval(t);
		case SIN:
			return sin_eval(t);
		case PULSE:
			return pulse_eval(t);
		case PWL:
			return pwl_eval(t);
		}
		assert(0);
	}

	double TransientSpecs::exp_eval(double t)
	{
		if (t <= exp.td1){
			return exp.i1;
		} else if (t <= exp.td2) {
			return exp.i1 + exp.idiff * (1.0 - std::exp(-(t - exp.td1) / exp.tc1));
		} else {
			return exp.i1 + exp.idiff * (std::exp(-(t - exp.td2) / exp.tc2) - std::exp(-(t - exp.td1) / exp.tc1));
		}
	}

	double TransientSpecs::sin_eval(double t)
	{
		if (t <= sin.td) {
			return sin.i1 + sin.ia * std::sin(sin.initial_phase);
		} else {
			return sin.i1 + sin.ia * std::sin(sin.omega * (t - sin.td) + sin.initial_phase) * std::exp(-(t - sin.td) * sin.df);
		}
	}

	double TransientSpecs::pulse_eval(double t)
	{
		// Get modulo to think only in terms of a single period
		double t_rem = std::fmod(t, pulse.per);

		if (t_rem <= pulse.td) {
			return pulse.i1;
		} else if (t_rem <= pulse.peak) {
			return pulse.i1 + (pulse.diff * (t_rem - pulse.td)) / pulse.tr;
		} else if (t_rem <= pulse.fall_start) {
			return pulse.i2;
		} else if (t_rem <= pulse.fall_end) {
			return pulse.i2 - (pulse.diff * (t_rem - pulse.fall_start)) / pulse.tf;
		} else {
			return pulse.i1;
		}
	}

	double TransientSpecs::pwl_eval(double t) {
		// Case of t before first point
		if (t < (*pwl.points)[0].first) {
			return (*pwl.points)[0].second;
		}

		// Case of t between points
		for (int i = 0; i < pwl.points->size() - 1; i++) {
			if (t >= (*pwl.points)[i].first && t < (*pwl.points)[i + 1].first) {
				return (*pwl.points)[i].second + pwl.slopes[i] * (t - (*pwl.points)[i].first);
			}
		}

		// Case of t after the last point (no need to check)
		return (*pwl.points)[pwl.points->size() - 1].second;
	}
}

std::ostream& operator<<(std::ostream &out, const spic::TransientSpecs &transient_specs)
{
	switch (transient_specs.type) {
		case spic::TransientSpecs::EXP:
			out << "EXP: i1=" << transient_specs.exp.i1 << ", i2=" << transient_specs.exp.i2
				<< ", td1=" << transient_specs.exp.td1 << ", tc1=" << transient_specs.exp.tc1
				<< ", td2=" << transient_specs.exp.td1 << ", tc2=" << transient_specs.exp.tc2;
			break;
		case spic::TransientSpecs::SIN:
			out << "SIN: i1=" << transient_specs.sin.i1 << ", ia=" << transient_specs.sin.ia
				<< ", fr=" << transient_specs.sin.fr << ", td=" << transient_specs.sin.td
				<< ", df=" << transient_specs.sin.df << ", ph=" << transient_specs.sin.ph;
			break;
		case spic::TransientSpecs::PULSE:
			out << "PULSE: i1=" << transient_specs.pulse.i1 << ", i2=" << transient_specs.pulse.i2
				<< ", td=" << transient_specs.pulse.td << ", tr=" << transient_specs.pulse.tr
				<< ", tf=" << transient_specs.pulse.tf << ", pw=" << transient_specs.pulse.pw
				<< ", per=" << transient_specs.pulse.per;
			break;
		case spic::TransientSpecs::PWL:
			out << "PWL: points=";
			for (const auto &point : *transient_specs.pwl.points) {
				out << "(" << point.first << ", " << point.second << ") ";
			}
			break;
		default:
			out << "Unknown transient type";
			break;
	}
	return out;
}

std::ostream& operator<<(std::ostream &out, const spic::TransientAnalysis &transient_analysis)
{
	out << transient_analysis.time_step << " " << transient_analysis.fin_time << std::endl;
	return out;
}

std::ostream& operator<<(std::ostream &out, const std::vector<spic::TransientAnalysis> &transient_list)
{
	out << "\t" << "Transient Analysis List (step, finish):" << std::endl;
	for (const auto &ta : transient_list) {
		out << "\t\t * " << ta << std::endl;
	}
	return out;
}