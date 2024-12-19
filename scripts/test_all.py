# Scripts that takesas argument the tests directory and runs all the tests in that directory
# Usage: python test_all.py tests/
# Also reads the csv files with name tests_dir/eval/test_name.csv
# Where the lines are of the form
# File,Max error,Average error
# and the first line is the header
#
# And computes the average error for each test
# and creates a new csv file with the results for all the tests
# in the tests_dir/eval directory
# The file is named tests_dir/eval/<version>.csv
# The first line is the header
#  Where <version> is an argument
# The lines are of the form
# Test,Max error,Average error
# and the first line is the header

import sys
import os
import subprocess
import argparse
import csv
from prettytable import PrettyTable

from test_cir import get_version_name


def run_tests(tests_dir, custom, iter_methods, itol, version_num):
	# Compute average error for each test
	tests = os.listdir(tests_dir)
	for test in tests:
		if test.endswith('.cir'):
			params = []
			if "30K" in test or "20K" in test or "10K" in test or "5K" in test:
				continue

			if "SPD" in test:
				params.append("--spd")
			if custom:
				params.append("--custom")
			if iter_methods:
				params.append("--iter")
				params.append(f"--itol={itol}")

			test_path = os.path.join(tests_dir, test)
			print('Running test', test_path)
			script_dir = os.path.dirname(os.path.realpath(__file__))
			test_cir = os.path.join(script_dir, "test_cir.py")
			result = subprocess.run(['python3', test_cir, "--cir_file", test_path, "--version", version_num] + params)
			if result.returncode != 0:
				raise RuntimeError(f"test_cir.py failed for cir_file {test_path}")

def read_csv_files(tests_dir, version):
	# Read the csv files with the results of the tests
	csv_dir = os.path.join(tests_dir, "eval", version)
	results = []

	for csv_file in os.listdir(csv_dir):
		if csv_file.endswith('.csv'):
			csv_file_path = os.path.join(csv_dir, csv_file)
			with open(csv_file_path, 'r') as f:
				lines = f.readlines()
				_, dc_op_max_error, dc_op_avg_error = lines[1].strip().split(',')
				_, dc_sweeps_max_error, dc_sweeps_avg_error = lines[-1].strip().split(',')
				results.append({
					'test': csv_file,
					'dc_op_max_error': dc_op_max_error,
					'dc_op_avg_error': dc_op_avg_error,
					'dc_sweeps_max_error': dc_sweeps_max_error,
					'dc_sweeps_avg_error': dc_sweeps_avg_error
				})
	return results

def main():
	parser = argparse.ArgumentParser(description='Run all tests in a directory')
	parser.add_argument('tests_dir', help='Directory containing tests')
	parser.add_argument('--custom', action='store_true', help='Directory containing tests')
	parser.add_argument('--iter', action='store_true', help='Directory containing tests')
	parser.add_argument('--itol', help='Tolerance for iterative solver', default='1e-3')
	parser.add_argument("--version", help="Version of evaluation", default="0")

	args = parser.parse_args()
	tests_dir = args.tests_dir
	version_str = get_version_name(args.custom, args.iter, args.itol, args.version)

	run_tests(tests_dir, args.custom, args.iter, args.itol, args.version)
	results = read_csv_files(tests_dir, version_str)

	# Write results to CSV
	pt = PrettyTable()
	with open(os.path.join(tests_dir, "eval", version_str + ".csv"), 'w', newline='') as csvfile:
		csvwriter = csv.writer(csvfile)
		csvwriter.writerow(["Test", "DC OP Max error", "DC OP Average error", "DC Sweeps Max error","DC Sweeps Average error"])
		pt.field_names = ["Test", "DC OP Max error", "DC OP Average error", "DC Sweeps Max error","DC Sweeps Average error"]
		for result in results:
			csvwriter.writerow(result.values())
			pt.add_row(result.values())

	with open(os.path.join(tests_dir, "eval", version_str + ".rpt"), 'w') as f:
		f.write(str(pt))

if __name__ == '__main__':
	main()