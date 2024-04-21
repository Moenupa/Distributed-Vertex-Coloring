#include <string.h>
#include <stdlib.h>
#include "graph.h"

#ifndef GRAPHIO_H
#define GRAPHIO_H

int read_graph(char * gpath, edge_t ** xadj, vertex_t ** adj,
	eweight_t ** ew, vweight_t ** vw, vertex_t * n_vertex, int loop);

#endif
