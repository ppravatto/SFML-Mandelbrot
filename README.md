# SFML-Mandelbrot
A simple C++ code to visualize the Mandelbrot set using SFML as the graphical engine.

## Requirements
- [SFML](https://www.sfml-dev.org/index.php)
- [OpenMP](https://www.openmp.org/) (only for the parallel version)

## Compiling and running
The script can be compiled using the provided `Makefile`. In order to compile the serial code use the command:
```
make
```
The parallel version of the program can be compiled using the command:
```
make parallel
```
The number of parallel threads involved in the computation can be set using the environment variable `$OMP_NUM_THREADS`.

Once compiled the program can be executed and should automatically open an interactive window. You can click and drag a zoom box to expand some regions of the plot.
