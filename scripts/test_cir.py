import subprocess
import os
import argparse

def get_version_name(custom, sparse, iter_solver, version_num):
	version = []
	if custom:
		version.append("CUSTOM")
	else:
		version.append("INTEGRATED")

	if sparse:
		version.append("SPARSE")
	if iter_solver:
		version.append("ITER")
	version.append(version_num)
	return "_".join(version)

def main():
	parser = argparse.ArgumentParser(description="Run spic with circuit file and process output to make csv with errors.")
	parser.add_argument("--cir_file", help="Path to the circuit file", default=None)
	parser.add_argument("--tests_dir", help="Path to the tests_dir", default=None)
	parser.add_argument("--spd", 	action='store_true', help="Enable SPD matrix option")
	parser.add_argument("--custom",	action='store_true', help="Enable custom solver option")
	parser.add_argument("--sparse", action='store_true', help='Enable sparse matrix option')
	parser.add_argument("--iter",	action='store_true', help="Enable iterative solver option")
	parser.add_argument("--itol", help="Set iteration tolernace", default="1e-3")
	parser.add_argument("--disable_dc_sweeps",	action='store_true', help="Disable DC Sweeps")
	parser.add_argument("--version", help="Version of evaluation", default="0")
	
	args = parser.parse_args()
	
	tests_dir = args.tests_dir
	cir_file = args.cir_file
	spd = args.spd
	custom = args.custom
	sparse = args.sparse
	iter_solver = args.iter
	itol = args.itol
	disable_dc_sweeps = args.disable_dc_sweeps
	cmp_dirs_args = []
	spic_option_args = ["--bypass_options"]
	
	if spd:
		spic_option_args.append("--spd")
	if custom:
		spic_option_args.append("--custom")
	if sparse:
		spic_option_args.append("--sparse")
	if iter_solver:
		spic_option_args.append("--iter")
		spic_option_args.append(f"--itol={args.itol}")
	if disable_dc_sweeps:
		spic_option_args.append("--disable_dc_sweeps")
		cmp_dirs_args.append("--disable_dc_sweeps")

	version = get_version_name(custom, sparse, iter_solver, args.version)

	test_name = cir_file.removesuffix(".cir").split("/")[-1]
	spic_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
	golden_dir = os.path.join(spic_dir, tests_dir, "golden", test_name + "_golden")
	output_dir = os.path.join(spic_dir, tests_dir, "output", version, test_name + "_output")
	csv_file = os.path.join(spic_dir, tests_dir, "eval", version, test_name + ".csv")
	spic_bin = os.path.join(spic_dir, "build/spic")

	os.makedirs(os.path.join(spic_dir, tests_dir, "eval", version), exist_ok=True)

	if not os.path.isfile(cir_file):
		raise Exception(f"Not a file: {cir_file}")

	# 1. Check if golden exists
	result = subprocess.run(["python3", os.path.join(spic_dir,"scripts/make_golden.py"),
					"--cir_file", cir_file,
					"--output_dir", golden_dir])
	if result.returncode != 0:
		raise RuntimeError(f"make_golden.py failed for cir_file {cir_file}")

	# 2. Run spic
	result = subprocess.run([spic_bin, "--cir_file", cir_file, "--output_dir", output_dir] + spic_option_args)
	if result.returncode != 0:
		raise RuntimeError(f"spic execution failed for cir_file {cir_file}")

	# 3. Run compare_dirs_csv.py
	result = subprocess.run(["python3", os.path.join(spic_dir,"scripts/compare_dirs_csv.py"),
				golden_dir, output_dir, "--output_csv", csv_file] + cmp_dirs_args)
	if result.returncode != 0:
		raise RuntimeError(f"compare_dirs_csv.py failed for cir_file {cir_file}")

if __name__ == "__main__":
	main()