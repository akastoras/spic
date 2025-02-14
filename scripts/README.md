# Verification Scripts

The `scripts` directory contains multiple scripts for running and evaluating SPICE simulations, including:
- `test_all.py`
- `test_cir.py`
- `make_golden.py`
- `compare_dirs_csv.py`
- `plot_transient.py`

## Prerequisites
- [Ngspice](https://ngspice.sourceforge.io/download.html) (used by `make_golden.py` for verification)

<!-- test_all -->
## `test_all.py`

This is the highest level script that a user can run for evaluating the performance of our tool against the ngspice tool.
It receives a directory containing all the test circuit files you want to run along with some running options to be passed
down to other scripts that actually run them. Each of the tests and corresponding options are passed to the `test_cir.py` script.

### Usage
```bash
python3 test_all.py tests/ [OPTIONS]
```
The OPTIONS can be:
- `tests_dir`: Directory containing test circuit files.
- `--custom`: Enable custom solver option.
- `--sparse`: Enable sparse matrix option.
- `--iter`: Enable iterative solver option.
- `--itol <value>`: Set tolerance for iterative solver (default: `1e-3`).
- `--version <value>`: Set evaluation version.

<!-- test_cir -->
## `test_cir.py`

This script's puprose is to run `spic` and evaluate its results based on the `ngspice` tool's behavior.
It takes a certain test circuit file and some running options as arguments, which it uses to perform the following steps:
1. It uses `make_golden.py` to create the golden output for that circuit file (using `ngspice`)
2. It runs `spic` to create our output for the same file
3. It uses `compare_dirs` to compare the results of `spic` against the golden results of `ngspice`

### Usage
```bash
python3 test_cir.py --cir_file <path_to_circuit_file> --test_dir <path_to_tests_dir> [OPTIONS]
```
The OPTIONS can be:
- `--cir_file <path>`: Path to the circuit file (.cir) to be processed.
- `--tests_dir <path>`: Path to the test directory where results and golden references are stored.
- `--spd`: Enable SPD matrix option.
- `--custom`: Enable custom solver option.
- `--sparse`: Enable sparse matri x option.
- `--iter`: Enable iterative solver option.
- `--itol <value>`: Set iteration tolerance (default: `1e-3`).
- `--disable_dc_sweeps`: Disable DC sweeps.
- `--version <value>`: Set the evaluation version number (default: `0`).

<!-- make_golden -->
## `make_golden.py`

This script takes a certain test circuit file along with an output directory name and uses `ngspice`
to get its results for that circuit file. Then, it parses them and creates a golden output directory
with the same file structure as that produced from `spic`

### Usage
```bash
python3 make_golden.py --cir_file <path_to_circuit_file> --output_dir <output_directory>
```

<!-- compare_dirs.py -->
## `compare_dirs_ccsv.py`

This script takes two output directories as arguments and, assuming they have the form of the result
directories produced by `spic`, it compares the outputs of the various analyses and dumps the results
in performace report csv files. An example of the file structure of such a directory is the following:

```shell
$ tree test_output
├── dc_op.dat # DC Analysis Output
├── test.cir # Copy of the .cir file
├── spic_performance.rpt # Performance counters of the tool
├── dc_sweeps # Subdirectory with output of DC Sweeps (total files: # prints x # sweeps)
│   ├── V2_1_2_0.1_V(3).dat # Output voltage in node 3 for DC Sweeps
│   │                       # of Voltage Source 2 from 1 Volt to 2 Volts with step 0.1
│   ├── V2_1_2_0.1_V(3).png # Graph using gnuplot
│   ├── V2_1_2_0.1_V(4).dat
|   └── V2_1_2_0.1_V(4).png
└── transient # Subdirectory with output of Transient Analyses (total files: # prints x # analyses)
    ├── tran_0.01_3_V(1).dat # Same as for DC Sweeps but for s transient analysis
    ├── tran_0.01_3_V(1).png
    ├── tran_0.01_3_V(5).dat
    └── tran_0.01_3_V(5).png
```

### Usage
```bash
python3 compare_dirs_csv.py <directory_1> <directory_2> [OPTIONS]
```

The OPTIONS can be:
- `--output_csv` OUTPUT_CSV Output CSV file (default: comparison_results.csv)
- `--disable_dc_sweeps`: Disable DC sweeps.


<!-- plot_transient.py -->
## `plot_transient.py`

This script is used for making part of the golden output only for the [IBM Transient tests](https://web.ece.ucsb.edu/~lip/PGBenchmarks/ibmpgbench.html). We do this since the solutions are already given, thus there is not need to waste time running those large benchmarks on `ngspice`. It takes the directory containing those solution files, create any plots requested by the test files and places them into the corresponding golden directories of those test files.

### Usage
```bash
python3 plot_transient.py <directory>
```