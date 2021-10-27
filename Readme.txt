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
The script `s*abc.script` is used to solve the language equation problem. It specifies the input files F S and the sets of input symbols U V, 
The script `s*abcv.script` solves the language equation problem and verifies the solution. It specifies the input files F S, the file R for verification, and the sets of input symbols U V.

For ITC benchmark, each directory in `itc99-poli/i99t/` consists of the original circuit S `b*.blif`, the fixed component F `b*fle.blif`, a particular solution R to the unknown part X `b*X.blif`, and two scripts.
The script `b*abc.script` is used to solve the language equation problem. It specifies the input files F S and the sets of input symbols U V, 
The script `b*abcv.script` will solve the language equation problem and verify the solution. It specifies the input files F S, the file R for verification, and the sets of input symbols U V.

Directory `casestudy/` consists of six directory b0410 to b0460 derived from itc99-poli/i99t/b04 with different latch number in X. For example, directory b0410 indicates that the latch number in b04X.blif is 10.
Each directory in `casestudy/` consists of the original circuit S `b04.blif`, the fixed component F `b04fle.blif`, a particular solution R to the unknown part X `b04X.blif`, and two scripts.
The script `b04abc.script` is used to solve the language equation problem. It specifies the input files F S and the sets of input symbols U V, 
The script `b04abcv.script` will solve the language equation problem and verify the solution. It specifies the input files F S, the file R for verification, and the sets of input symbols U V.

Note that all the scripts solve the language equantion without optimization for F and S. To perform optimization, please add `-w` to the corresponding script.
For example, to perform optimization for `benchmark/iccas/s208/s208abc.script`, we can modify the command in script to 
`solve -w X,Clear,C_8,C_7,C_6,C_5,C_4,C_3,C_2,C_1,C_0,Y_8,Y_7,Y_6,Y_5 Y_4,Y_3,Y_2,Y_1 benchmark/iccas/s208/s208.bench benchmark/iccas/s208/s208fle.mv`
by adding `-w`.

## Reproduce the experimental results
There are two scripts.
To run ISCAS benchmarks, type `./runabcIccas.sh <num of circuit>`. For example, type `./runabcIccas.sh 208` to solve language equation with circuit s208.
To run ISCAS benchmarks and verify the solution, type `./runabcIccas.sh <num of circuit> v`.  For example, type `./runabcIccas.sh 208 v` to solve language equation and verify the solution with circuit s208.

To run ITC benchmarks, type `./runabcItc.sh <num of circuit>`. For example, type `./runabcItc.sh 01` to solve language equation with circuit b01.
To run ITC benchmarks and verify the solution, type `./runabcItc.sh <num of circuit> v`.  For example, type `./runabcItc.sh 01 v` to solve language equation and verify the solution with circuit b01.

## Reproduce the experimental results for case study
Type `./runabcCaseAnalysis.sh <num of latch in b04X.blif>`. For example, type `./runabcCaseAnalysis.sh 10` to solve language equation for circuit b04 and b04X.blif with 10 latches.
To verify the solution, type `./runabcCaseAnalysis.sh <num of latch in b04X.blif> v`.  For example, type `./runabcIccas.sh 10 v` to solve language equation and verify the solution for circuit b04 and b04X.blif with 10 latches..
