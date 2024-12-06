import subprocess
import numpy as np
import os

def run_ngspice(cir_file):
	# Run ngspice with the specified .cir file
	out_file = 'ngspice_output.txt'
	result = subprocess.run(['ngspice', '-b', '-o', out_file, cir_file], capture_output=True, text=True)
	if result.returncode != 0:
		raise RuntimeError(f"ngspice failed: {result.stderr}")

	if not os.path.exists(cir_file):
		raise FileNotFoundError(f"File not found: {cir_file}")

	# Extract the solution from the ngspice output
	with open(out_file, 'r') as file:
		lines = file.readlines()

	# Extract the values from the ngspice output
	solution = []
	for line in lines:
		line = line.strip()
		if line.startswith('V('):
			value = float(line.split()[1])
			solution.append(value)

	return np.array(solution)

def run_cpp_program(executable):
	# Run the C++ program
	result = subprocess.run([executable], capture_output=True, text=True)
	if result.returncode != 0:
		raise RuntimeError(f"C++ program failed: {result.stderr}")

	# Extract the solution from the C++ program output
	# Assuming the output is in a specific format, extract the values
	# This part may need to be adjusted based on the actual output format
	solution = []
	for line in result.stdout.splitlines():
		value = float(line.strip())
		solution.append(value)

	return np.array(solution)

def calculate_errors(ngspice_solution, cpp_solution):
	# Calculate the relative errors
	relative_errors = np.abs((ngspice_solution - cpp_solution) / ngspice_solution)
	max_error = np.max(relative_errors)
	avg_error = np.mean(relative_errors)
	return max_error, avg_error

def main():
	cir_file = 'tests/part3_simple.cir'
	cpp_executable = 'path/to/your/cpp_program'

	if not os.path.exists(cir_file):
		raise FileNotFoundError(f"File not found: {cir_file}")

	ngspice_solution = run_ngspice(cir_file)
	# cpp_solution = run_cpp_program(cpp_executable)

	# max_error, avg_error = calculate_errors(ngspice_solution, cpp_solution)
	print(ngspice_solution)

	# print(f"Maximum Relative Error: {max_error}")
	# print(f"Average Relative Error: {avg_error}")

if __name__ == "__main__":
	main()