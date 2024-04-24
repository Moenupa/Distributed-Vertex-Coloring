# Parallel Vertex Coloring

 > @author Meng Wang  
 > @email mwang106@ur.rochester.edu  
 > @create date 2024-04-17 13:31  
 > @modify date 2024-04-24 19:38  
 > @desc Final Project: Parallel and Distributed Systems CSC458 SPRING 2024

A parallel implementation of the vertex coloring algorithm by **graph partitioning**, implemented in C++ using the `OpenMP` library.


## Structure

```
.
|-- src/            # dir for source code
|   |-- utils       # c code for graph io
|   |               #   and matrix market format io
|   `-- coloring.cpp
|-- tools/          # python utilities for visualization
`-- makefile        # to compile code or download data
```


## Compilation

Use the following command to compile this project

```bash
make 
```

This will result in `coloring` executable in the root directory.

## Data Preparation

Running any of the following commands will download the corresponding archive to path `./data/xxx.tar.gz` from the [SuiteSparse Matrix Collection](https://sparse.tamu.edu/). `nlpkkt240` may be large, be warned of disk space.

```bash
make nlpkkt80       # download schenk/nlpkkt80, ~500MB
make nlpkkt120      # download schenk/nlpkkt120, ~2GB
make nlpkkt240      # download schenk/nlpkkt240
```

To extract its contents run: (this will extract the contents to `./data/xxx/xxx.mtx`)

```
make extract
```

## Execution

To run the program, use the following command:

```bash
# ./coloring [matrix_path] [max_num_threads]
./coloring data/nlpkkt120/nlpkkt120.mtx 64
```

And it will print the following results in command line.

```
 Algorithm  | # Threads | # Conf.Fixes | # Colors | T Exec.  (s) | # Conf.   
 Sequential | 1         | 0            | 59       | 5.9887391880 | 0
 Parallel   | 1         | 0            | 59       | 8.9528536670 | 0
 Parallel   | 2         | 1            | 62       | 6.1929058890 | 0
 Parallel   | 4         | 1            | 62       | 3.1944426690 | 0
 Parallel   | 8         | 1            | 66       | 1.5765233710 | 0
 Parallel   | 16        | 1            | 72       | 0.8925240210 | 0
 Parallel   | 32        | 1            | 70       | 0.6883127570 | 0
 Parallel   | 64        | 1            | 71       | 0.3527218440 | 0
```
