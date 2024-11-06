# spic: A subset of SPICE

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
./spic <testfile>.cir
```