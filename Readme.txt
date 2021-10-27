# Artifact for Language Equation Solving via Boolean Automata Manipulation

## Introduction

This is a C++ implemantation of the algorithms proposed in Solving Language Equation Solving via Boolean Automata Manipulation

## Installation

Type `make` to complie and the executable is `abc`

```
make
```

It has been tested successfully under Ubuntu 20.04.1 with GCC_VERSION=9.3.0

## Execution

Run `./abc` from your console and execute command `solve` to solve the language equantion. `solve -h` shows detailed arguments of the command.
You can also run the `ssat` command from the console, as the following examples show.

## Benchmarks

All benchmarks are in directory `benchmark/` and `casestudy/`.  Directory `benchmark` contains ISCAS (directory `iccas/`) and ITC (directory`itc99-poli/i99t/`) benchmarks.

For ISCAS benchmark, each directory in `iccas/` consists of the original circuit S `s*.bench`, the fixed component `s*fle.mv`, a particular solution to the unknown part X `s*X.mv`, and two scripts.
The script `s*abc.script` is used to solve the language equation problem and the script `s*abcv.script` will solve the language equation problem and verify the solution.

For ITC benchmark, each directory in `itc99-poli/i99t/` consists of the original circuit S `b*.blif`, the fixed component `s*fle.blif`, a particular solution to the unknown part X `s*X.blif`, and two scripts.
The script `b*abc.script` is used to solve the language equation problem and the script `b*abcv.script` will solve the language equation problem and verify the solution.

## Reproduce the experimental results
There are two scripts.
To run ISCAS benchmarks, type `./runabcIccas.sh <num of circuit>`. For example, type `./runabcIccas.sh 208` to solve language equation with circuit s208.
To run ISCAS benchmarks and verify the solution, type `./runabcIccas.sh <num of circuit> v`.  For example, type `./runabcIccas.sh 208 v` to solve language equation and verify the solution with circuit s208.

To run ITC benchmarks, type `./runabcItc.sh <num of circuit>`. For example, type `./runabcItc.sh 01` to solve language equation with circuit b01.
To run ITC benchmarks and verify the solution, type `./runabcItc.sh <num of circuit> v`.  For example, type `./runabcItc.sh 01 v` to solve language equation and verify the solution with circuit b01.