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

def run_tests(tests_dir, version):
	# Compute average error for each test
	tests = os.listdir(tests_dir)
	for test in tests:
		if test.endswith('.cir'):
			test_path = os.path.join(tests_dir, test)
			print('Running test', test_path)
			script_dir = os.path.dirname(os.path.realpath(__file__))
			test_cir = os.path.join(script_dir, "test_cir.py")
			subprocess.run(['python3', test_cir, "--cir_file", test_path, "--version", version])

def read_csv_files(tests_dir):
	# Read the csv files with the results of the tests
	csv_files = os.listdir(os.path.join(tests_dir, "eval"))
	results = {}
	for csv_file in csv_files:

		if csv_file.endswith('.csv'):
			csv_file_path = os.path.join(tests_dir, "eval", csv_file)
			with open(csv_file_path, 'r') as f:
				lines = f.readlines()
				_, dc_op_max_error, dc_op_avg_error = line[1].strip().split(',')
				_, dc_sweeps_max_error, dc_sweeps_avg_error = line[-1].strip().split(',')
				results[csv_file] = {
					'dc_op_max_error': dc_op_max_error,
					'dc_op_avg_error': dc_op_avg_error,
					'dc_sweeps_max_error': dc_sweeps_max_error,
					'dc_sweeps_avg_error': dc_sweeps_avg_error
				}
	return results

def main():
	parser = argparse.ArgumentParser(description='Run all tests in a directory')
	parser.add_argument('tests_dir', help='Directory containing tests')
	parser.add_argument("--version", help="Version of evaluation", default="spic")

	args = parser.parse_args()
	tests_dir = args.tests_dir
	version = args.version

	run_tests(tests_dir, version)
	results = read_csv_files(tests_dir)

	# Write results to CSV
	with open(s.path.join(tests_dir, "eval", version + ".csv"), 'w', newline='') as csvfile:
		csvwriter = csv.writer(csvfile)
		csvwriter.writerow(["Test", "DC OP Max error", "DC OP Average error", "DC Sweeps Max error","DC Sweeps Average error"])
		for row in results:
			csvwriter.writerow(row)

if __name__ == '__main__':
	main()