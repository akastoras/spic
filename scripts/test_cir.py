import subprocess
import os
import argparse

def main():
	parser = argparse.ArgumentParser(description="Run spic with circuit file and process output to make csv with errors.")
	parser.add_argument("--cir_file", help="Path to the circuit file", default=None)
	parser.add_argument("--version", help="Version of evaluation", default="spic")
	args = parser.parse_args()
	cir_file = args.cir_file
	version = args.version
	
	test_name = cir_file.removesuffix(".cir").split("/")[-1]
	spic_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
	golden_dir = os.path.join(spic_dir, "tests/golden", test_name + "_golden")
	output_dir = os.path.join(spic_dir, "tests/output", version, test_name + "_output")
	csv_file = os.path.join(spic_dir, "tests/eval", version, test_name + ".csv")
	spic_bin = os.path.join(spic_dir, "build/spic")

	os.makedirs(os.path.join(spic_dir, "tests/eval", version), exist_ok=True)

	if not os.path.isfile(cir_file):
		raise Exception(f"Not a file: {cir_file}")
	
	# 1. Check if golden exists
	# if not os.path.isfile(golden_dir + "dc_op.dat"):
	subprocess.run(["python3", os.path.join(spic_dir,"scripts/make_golden.py"), 
					"--cir_file", cir_file, 
					"--output_dir", golden_dir])
	
	# 2. Run spic
	subprocess.run([spic_bin, "--cir_file", cir_file, "--output_dir", output_dir])

	# 3. Run compare_dirs_csv.py
	subprocess.run(["python3", os.path.join(spic_dir,"scripts/compare_dirs_csv.py"),
				golden_dir, output_dir, "--output_csv", csv_file])

if __name__ == "__main__":
	main()