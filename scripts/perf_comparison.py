# Get as argument a directory with the tests
# Get as argument a version number
# Read all directories in the tests directory

import sys
import os
import argparse
import csv
from prettytable import PrettyTable
import glob

def get_directories(tests_dir):
	# Get all directories in the tests directory that end in the version number
	directories = os.listdir(tests_dir)
	return_dirs = []
	for directory in directories:
		if os.path.isdir(os.path.join(tests_dir, directory)):
			return_dirs.append(directory)
	
	return return_dirs

def perf_comparison(tests_dir):
	# For each version_directory get a list of the test_directories in it
	# and create a dictionary with the name of the test_output_dir as the key
	# Then read the performance.rpt files in the test_directories and get the total_secs value for the key
	# Each version_directory will be a column in the output csv file, while each of the test_directories will be a row
	test_files = [os.path.basename(f) for f in glob.glob(f'{tests_dir}/*.cir')]
	test_files.sort()
	test_output_dirs = [test_file.replace('.cir', '_output') for test_file in test_files]

	output_dir = os.path.join(tests_dir, 'output')
	version_directories = get_directories(output_dir)
	perf_dict = {}
	pt = PrettyTable()

	for version_directory in version_directories:
		for test_file, test_output_dir in zip(test_files, test_output_dirs):
			perf_file = os.path.join(output_dir, version_directory, test_output_dir, 'spic_performance.rpt')
			
			if os.path.exists(perf_file):
				with open(perf_file, 'r') as f:
					lines = f.readlines()
					for line in lines:
						if 'total_secs:' in line:
							if test_output_dir not in perf_dict.keys():
								perf_dict[test_output_dir] = []
							perf_dict[test_output_dir].append(line.split()[1])
							break
			else:
				if test_output_dir not in perf_dict.keys():
					perf_dict[test_output_dir] = []
				perf_dict[test_output_dir].append('N/A')

	pt.field_names = ["Tests"] + version_directories
	for test in perf_dict.keys():
		pt.add_row([test] + perf_dict[test])
	print("Time in seconds:")
	print(str(pt))

def main():
	parser = argparse.ArgumentParser(description='Get all directories in the tests directory')
	parser.add_argument('tests_dir', type=str, help='Directory with the tests')
	args = parser.parse_args()
	perf_comparison(args.tests_dir)

if __name__ == '__main__':
	main()