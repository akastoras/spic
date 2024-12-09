# Script that given two directories with the same structure, compares the output of the files in the directories.
# The directories include one file named "dc_op.dat" that has one line for each node in the form <node_name> <voltage>
# and one subdirectory called dc_sweeps that contains one file for each sweep in the form <sweep_name>.dat
# where <sweep_name> is V<source_name>_<start_value>_<end_value>_<step>_V(<node_name>).dat
# The files contain lines in the form <source_voltage> <node_voltage> where <source_voltage> is the voltage of the source
# that is being incremented in the sweep and <node_voltage> is the voltage of the node that we check
# The script reads the output of the files in the directories and creates numpy arrays with them, then calculates
# the relative error between the two arrays and prints the maximum and average error found. It also calculates the relative error
# between the dc_op.dat files and prints the maximum and average error found.

# The first file is considered the golden and the second file is the output that is compared to the golden.

import os
import sys
import numpy as np
import re
import argparse
import csv

def read_dc_op_file(file_path):
	with open(file_path, 'r') as file:
		lines = file.readlines()
		dc_op = {}
		for line in lines:
			node_name, voltage = line.upper().split()
			dc_op[node_name] = float(voltage)
		return dc_op

def read_dc_sweep_file(file_path):
	with open(file_path, 'r') as file:
		lines = file.readlines()
		sweep = []
		for line in lines:
			source_voltage, node_voltage = line.upper().split()
			sweep.append((float(source_voltage), float(node_voltage)))
		return sweep

def compare_dc_op_files(file1, file2):
	dc_op1 = read_dc_op_file(file1)
	dc_op2 = read_dc_op_file(file2)
	max_error = 0
	average_error = 0

	if len(dc_op1) == 0:
		return 0, 0

	for node_name in dc_op1:
		voltage1 = dc_op1[node_name]
		voltage2 = dc_op2[node_name]
		if voltage1 == 0:
			print("Node {} has voltage 0 in golden file".format(node_name))
			continue # TODO: Do something smarter here
		error = abs(voltage1 - voltage2) / abs(voltage1)
		max_error = max(max_error, error)
		average_error += error
	average_error /= len(dc_op1)
	return max_error, average_error

def compare_dc_sweep_files(file1, file2):
	sweep1 = read_dc_sweep_file(file1)
	sweep2 = read_dc_sweep_file(file2)
	max_error = 0
	average_error = 0

	if (len(sweep1) != len(sweep2)):
		print(f"Files {file1} and {file2} have different number of lines")
		sys.exit(1)
	
	for i in range(len(sweep1)):
		source_voltage1, node_voltage1 = sweep1[i]
		source_voltage2, node_voltage2 = sweep2[i]
		error = abs(node_voltage1 - node_voltage2) / abs(node_voltage1)
		max_error = max(max_error, error)
		average_error += error
	average_error /= len(sweep1)
	return max_error, average_error

def compare_directories(dir1, dir2):
	dc_op_file1 = os.path.join(dir1, "dc_op.dat")
	dc_op_file2 = os.path.join(dir2, "dc_op.dat")
	max_error_dc_op, average_error_dc_op = compare_dc_op_files(dc_op_file1, dc_op_file2)

	dc_sweeps_dir1 = os.path.join(dir1, "dc_sweeps")
	dc_sweeps_dir2 = os.path.join(dir2, "dc_sweeps")

	results = []

	# DC_OP results
	results.append(["DC_OP", max_error_dc_op, average_error_dc_op])

	# DC_Sweeps results
	max_error_dc_sweep = 0
	average_error_dc_sweep = 0
	sweep_count = 0

	for file in os.listdir(dc_sweeps_dir1):
		if file.endswith(".dat"):
			file1 = os.path.join(dc_sweeps_dir1, file)
			file2 = os.path.join(dc_sweeps_dir2, file)
			max_error, average_error = compare_dc_sweep_files(file1, file2)
			results.append([file, max_error, average_error])
			max_error_dc_sweep = max(max_error_dc_sweep, max_error)
			average_error_dc_sweep += average_error
			sweep_count += 1

	if sweep_count > 0:
		average_error_dc_sweep /= sweep_count

	results.append(["DC_Sweeps total", max_error_dc_sweep, average_error_dc_sweep])

	return results

def main():
	parser = argparse.ArgumentParser(description='Compare output of two directories')
	parser.add_argument('dir1', type=str, help='First directory')
	parser.add_argument('dir2', type=str, help='Second directory')
	parser.add_argument('--output_csv', type=str, default='comparison_results.csv', help='Output CSV file (default: comparison_results.csv)')
	args = parser.parse_args()

	results = compare_directories(args.dir1, args.dir2)

	# Write results to CSV
	with open(args.output_csv, 'w', newline='') as csvfile:
		csvwriter = csv.writer(csvfile)
		csvwriter.writerow(["File", "Max error", "Average error"])
		for row in results:
			csvwriter.writerow(row)

if __name__ == "__main__":
	main()