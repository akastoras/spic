# spic: A subset of SPICE

The following text documents the functionality and user interface of `spic`. This project was developed for the [ECE513 - Circuit Simulation Algorithms](https://www.e-ce.uth.gr/studies/undergraduate/courses/ece513/?lang=en) course. The goal is to build a simulator for SPICE netlists in C++ with the [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page) library.

## Clone Instructions

1. Clone the repository
```shell
git clone https://github.com/akastoras/spic
cd spic
```

2. Download essential submodules
```shell
git submodule update --init --recursive
```

## Build Instructions

To build spic run the following instructions:

1. Create a build subdirectory:
```shell
mkdir build
cd build
```

2. Configure the build:
```shell
cmake -DVERBOSIRY=<VERBOSITY_LEVEL> ..
```
The verbosity levels are the following:
* 0 - No extra printing (only the basic final output)
* 1 - Print all the netlist nodes
* 2 - Print all the netlist and the node table's entries
* 3 - Print all the netlist, the node table's entries and analytical lexer output

3. Execute build:
```shell
make
```

## Running spic
```shell
./spic [OPTIONS...]
```

For the OPTIONS let's take a look at the output of `./spic --help`:
```
Allowed options:
  --help                       produce help message
  --cir_file arg               Path to the circuit file
  --output_dir arg             Output directory
  --bypass_options             Bypass .cir file options
  --disable_dc_sweeps          Disable DC Sweeps
  --spd                        Enable SPD option
  --custom                     Enable custom solver option
  --sparse                     Enable sparse solver option
  --iter                       Enable iterative solver option
  --itol arg (=0.001)          Set iteration tolerance
  --transient_method arg (=TR) Set derivative calculation method
```

For example, if we have a test.cir file that contains all the options we need, we will use it with:
```shell
./spic --cir_file test.cir --output_dir test_output 
```

The `test_output` subdirectory will contain all the output of the run. An example

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

## Functionality

The tool is structured in the following way:

### Parsing .cir files
A subset of the SPICE systax is supported for the input files. The parser is built using Flex and Bison tools.

### DC Analysis
After parsing the script we build the system using the MNA method. We solve the systems and print the results in the `dc_op.dat` file.

We also support dc sweeps for voltage and current sources. A DC sweep is specified with the following line in the options part of a .cir file:

```
.DC <source_name> <start_value> <end_value> <step>
```

And the output of the sweep of a node is printed to a file if a print or plot command are used like the following:

```
.PRINT V(<node_name>) ...
```

A `.PLOT` command works like a `.PRINT` one but also produces a graph with `gnuplot`.

### Dense and Sparse Matrices
By default, `spic` stores all matrices in dense format. If the `.OPTIONS SPARSE` option is used, `spic` uses sparse systems supported by Eigen.

### Solvers
For solving the MNA system, we support 8 solvers:
* **Integrated LU**: Eigen's built'in implementation of LU decomposition
* **Custom LU**: Our unoptimized implementation of LU decomposition (only for dense systems)
* **Integrated Cholesky**: Eigen's built'in implementation of Cholesky decomposition
* **Custom Cholesky**: Our unoptimized implementation of Cholesky decomposition (only for dense systems)
* **Integrated CG**: Eigen's built-in version
* **Custom CG**: Our implementation of CG iterative solver using Eigen's optimized operators
* **Integrated Bi-CG**: Eigen's built in Bi-CGSTAB version
* **Custom Bi-CG**: Our implementation of BiCG iterative solver using Eigen's optimized operators

Other than the custom versions of the direct solvers which are anotated to be supported only for dense systems, all other 

### Transient Analysis

We also support two types of transient analyses and four different transient specification functions for voltage and current sources. A transient analysis is defined as:

```
.TRAN <time_step> <fin_time>
```

and the type of analysis to use is given with:

```
.OPTIONS METHOD=<TR|BE>
```

where TR is for using the Trapezoidal and BE is for using the Backward-Euler
transent analysis method.

The transient specification functions we support are the following:
- `EXP`
- `SIN`
- `PULSE`
- `PWL`

An example of their (optional) usage on a volatge source definition could be the following:

```
V<src_name> <node_pos> <node_neg> <value> [transient_spec]
```

## Verification Scripts
We have implemented a number of Verification related scripts that automate the process
of executing a simulation and comparing it with the results of ngspice. See more [here](scripts/README.md).

## Performance Evaluation

In [Time_Evaluation.xlsx](Time_Evaluation.xlsx) we have some performance metrics frome runs in several large circuits for both sparse and dense methods.