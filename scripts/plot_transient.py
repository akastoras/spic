import os
import re
import argparse

def parse_file(file_path):
	with open(file_path, 'r') as file:
		data = file.read()

	node_pattern = re.compile(r'Node: (\S+)\n(.*?)\nEND: \1', re.DOTALL)
	nodes = node_pattern.findall(data)

	node_data = {}
	for node, values in nodes:
		pairs = [tuple(map(float, line.split())) for line in values.strip().split('\n')]
		node_data[node] = pairs

	return node_data

def generate_gnuplot_commands(node_data, input_dir, base_name):
	output_dir = os.path.join(input_dir, "golden", base_name, "transient")
	os.makedirs(output_dir, exist_ok=True)

	commands = []
	for node, values in node_data.items():
		plot_file = os.path.join(output_dir, f"tran_{node}.dat")
		image_file = os.path.join(output_dir, f"tran_{node}.png")

		with open(plot_file, 'w') as f:
			for time, value in values:
				f.write(f"{time} {value}\n")

		plot_command = (
			f"gnuplot -e \"set terminal png ;"
			f"set output '{image_file}' ;"
			f"set object 1 rectangle from screen 0,0 to screen 1,1 fillcolor rgb 'black' behind ;"
			f"set border lc rgb 'white' ;"
			f"set tics textcolor rgb 'white' ;"
			f"set key textcolor rgb 'white' ;"
			f"set title textcolor rgb 'white' ;"
			f"set xlabel textcolor rgb 'white' ;"
			f"set ylabel textcolor rgb 'white' ;"
			f"set grid ;"
			f"plot '{plot_file}' with lines title 'V({node}) vs time'\""
		)
		commands.append(plot_command)

	return commands

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='Process .output files and generate plots.')
	parser.add_argument('directory', type=str, help='Path to the directory containing .output files')
	args = parser.parse_args()

	input_dir = args.directory

	for file_name in os.listdir(input_dir):
		if file_name.endswith('.output'):
			file_path = os.path.join(input_dir, file_name)
			base_name = file_name[:-7]  # Remove the '.output' extension

			node_data = parse_file(file_path)
			commands = generate_gnuplot_commands(node_data, input_dir, base_name)

			for command in commands:
				os.system(command)