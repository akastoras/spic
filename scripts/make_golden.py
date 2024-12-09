import subprocess
import numpy as np
import argparse
import os
import re
import shutil

def run_ngspice(cir_file, out_file):
	# Run ngspice with the specified .cir file
	result = subprocess.run(['ngspice', '-b', '-o', out_file, cir_file], capture_output=True, text=True)
	if result.returncode != 0:
		raise RuntimeError(f"ngspice failed: {result.stderr}")

	if not os.path.exists(cir_file):
		raise FileNotFoundError(f"File not found: {cir_file}")

	# Extract the solution from the ngspice output
	with open(out_file, 'r') as file:
		lines = file.readlines()

	# Extract the values from the ngspice output
	# The worst code you will ever see. Here it comes:
	solution = []
	first_line = 0
	end_line = 0
	for i, line in enumerate(lines):
		line = line.strip()
		line = line.split()
		if len(line) == 2:
			if line[0] == "Node" and line[1] == "Voltage":
				first_line = i + 3
			if line[0] == "Source" and line[1] == "Current":
				end_line = i - 1
				
	for line in lines[first_line:end_line]:
		line = line.upper().strip()
		parts = line.split()
		node_name = parts[0]
		value = parts[1]
		if node_name.startswith('V('):
			node_name = node_name[2:-1]
		solution.append((node_name, value))

	return solution

def parse_dc_sweeps(file_path):
	dc_sweeps = []
	dc_sweep_pattern = re.compile(r'\.DC\s+([VI])(\S+)\s+(\S+)\s+(\S+)\s+(\S+)')

	with open(file_path, 'r') as file:
		for line in file:
			match = dc_sweep_pattern.match(line.upper().strip())
			if match:
				dc_sweeps.append({
					'type': match.group(1),
					'source_name': match.group(2),
					'start': match.group(3),
					'stop': match.group(4),
					'step': match.group(5)
				})

	return dc_sweeps

def parse_spice_output(file_path):
	sweeps = []
	current_sweep = None
	dc_sweep_pattern = re.compile(r'INDEX\s+([VI])-SWEEP\s+V\((\S+)\)')
	data_pattern = re.compile(r'(\d+)\s+([+-]?[\d\.\+eE\-]+)\s+([+-]?[\d\.\+eE\-]+)')

	with open(file_path, 'r') as file:
		lines = file.readlines()
		i = 0
		while i < len(lines):
			line = lines[i].upper().strip()

			match = dc_sweep_pattern.match(line)
			if match:
				current_sweep = {
					'sweep_type': match.group(1),
					'node': match.group(2),
					'data': []
				}
				i += 1
				while i < len(lines):
					line = lines[i].upper().strip()
					match = data_pattern.match(line)
					if match:
						index = int(match.group(1))
						sweep_value = match.group(2)
						node_value = match.group(3)
						current_sweep['data'].append((index, sweep_value, node_value))
					else:
						match = dc_sweep_pattern.match(line)
						if match and lines[i+2].split()[0] == "0":
							sweeps.append(current_sweep)
							i -= 1
							break
					i += 1
			i += 1
	if current_sweep:
		sweeps.append(current_sweep)
	return sweeps

def main():
	parser = argparse.ArgumentParser(description="Run ngspice with circuit file and process output.")
	parser.add_argument("--cir_file", help="Path to the circuit file")
	parser.add_argument("--output_dir", help="Output directory (default: <filename>_golden)", default=None)
	args = parser.parse_args()

	cir_file = os.path.abspath(args.cir_file)
	test_dir = os.path.dirname(cir_file)
	filename = os.path.splitext(os.path.basename(cir_file))[0]
	output_dir = args.output_dir if args.output_dir else f"{test_dir}/golden/{filename}_golden"
	ngspice_out_file = f"{os.path.join(output_dir, filename)}.ngspice.out"

	# If the output directory already exists, delete it
	if os.path.exists(output_dir):
		shutil.rmtree(output_dir)

	# Create output directory if it doesn't exist
	os.makedirs(output_dir, exist_ok=False)

	# Run ngspice and parse the output
	solution = run_ngspice(cir_file, ngspice_out_file)

	# Create dc_op.txt file
	dc_op_file = os.path.join(output_dir, "dc_op.dat")
	with open(dc_op_file, 'w') as f:
		for node_name, value in solution:
			f.write(f"{node_name} {value}\n")

	# Copy the .cir file to the output directory
	shutil.copy(cir_file, os.path.join(output_dir, os.path.basename(cir_file)))

	# Parse the .cir file for DC sweeps
	dc_sweeps = parse_dc_sweeps(cir_file)

	# Parse the ngspice output file for sweep data
	sweeps = parse_spice_output(ngspice_out_file)

	# Create dc_sweeps directory
	dc_sweeps_dir = os.path.join(output_dir, "dc_sweeps")
	os.makedirs(dc_sweeps_dir, exist_ok=True)

	# Write the sweep data to files
	for i, sweep in enumerate(sweeps):
		source_type = dc_sweeps[i%len(dc_sweeps)]['type']
		source_name = dc_sweeps[i%len(dc_sweeps)]['source_name']
		start = dc_sweeps[i%len(dc_sweeps)]['start']
		stop = dc_sweeps[i%len(dc_sweeps)]['stop']
		step = dc_sweeps[i%len(dc_sweeps)]['step']
		node = sweep['node']
		file_name = f"{source_type}{source_name}_{(start)}_{stop}_{step}_V({node}).dat"
		file_path = os.path.join(dc_sweeps_dir, file_name)
		with open(file_path, 'w') as f:
			for data in sweep['data']:
				f.write(f"{data[1]} {data[2]}\n")

# Example usage
if __name__ == "__main__":
	main()