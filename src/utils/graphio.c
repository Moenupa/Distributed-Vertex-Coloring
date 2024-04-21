#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mmio.h"
#include "graphio.h"


typedef struct {
	// src
	vertex_t u;
	// dest
	vertex_t v;
	// weight
	eweight_t w;
}
Edge;

int cmp(const void * a, const void * b) {
	const vertex_t * ia = (const vertex_t * ) a;
	const vertex_t * ib = (const vertex_t * ) b;
	return * ia - * ib;
}

int tricmp(const void * t1, const void * t2) {
	Edge * tr1 = (Edge * ) t1;
	Edge * tr2 = (Edge * ) t2;
	if (tr1 -> u == tr2 -> u) {
		return (int)(tr1 -> v - tr2 -> v);
	}
	return (int)(tr1 -> u - tr2 -> u);
}

int ends_with(const char * str, const char * suffix) {
	if (!str || !suffix) return 0;
	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	if (lensuffix > lenstr) return 0;
	return (strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0);
}

int read_chaco(FILE * fp, edge_t ** xadj, vertex_t ** adj,
	eweight_t ** ew, vweight_t ** vw,
	vertex_t * n_vertex, int loop) {

	int state = 0, fmt = 0, ncon = 1, i;
	vertex_t num_vertex = -1, vcount = 0, jv;
	edge_t num_edge = -1, ecount = 0;
	char * temp, * graphLine = (char * ) malloc(sizeof(char) * 10000000 + 1);

	while (fgets(graphLine, 10000000, fp) != NULL) {
		for (i = 0; i < (int) strlen(graphLine); i++) {
			char c = graphLine[i];
			if (c != ' ' && c != '\t' && c != '\n') {
				break;
			}
		}

		/* read the line wrt fmt and mw */
		if (graphLine[0] == '%') {
			continue;
		} else if (state == 0) {
			temp = strtok(graphLine, " \t\n");
			num_vertex = atoi(temp);

			temp = strtok(NULL, " \t\n");
			num_edge = atoi(temp);

			temp = strtok(NULL, " \t\n");
			if (temp != NULL) {
				fmt = atoi(temp);
				temp = strtok(NULL, " \t\n");
				if (temp != NULL) {
					ncon = atoi(temp);
				}
			}

			* n_vertex = num_vertex;
			( * xadj) = (edge_t * ) malloc(sizeof(edge_t) * (num_vertex + 1));
			( * xadj)[0] = 0;

			( * vw) = (vweight_t * ) malloc(sizeof(vweight_t) * num_vertex);
			( * adj) = (vertex_t * ) malloc(sizeof(vertex_t) * 2 * num_edge);
			( * ew) = (eweight_t * ) malloc(sizeof(eweight_t) * 2 * num_edge);

			state = 1;
		} else {
			if (vcount == num_vertex) {
				fprintf(stderr, "contains more than %ld lines\n", (long) num_vertex);
				return -1;
			}

			temp = strtok(graphLine, " \t\n");

			if (fmt >= 100) {
				temp = strtok(NULL, " \t\n");
			}

			if (fmt % 100 >= 10) {
				( * vw)[vcount] = atoi(temp);
				for (i = 1; i < ncon; i++) {
					temp = strtok(NULL, " \t\n");
				}
			} else {
				( * vw)[vcount] = 1;
			}

			while (temp != NULL) {
				if (ecount == 2 * num_edge) {
					fprintf(stderr, "file contains more than %ld edges.\n", (long) num_edge);
					return -1;
				}

				( * adj)[ecount] = atoi(temp) - 1;
				if (( * adj)[ecount] == vcount && !loop) {
					continue;
				}

				temp = strtok(NULL, " \t\n");
				if (fmt % 10 == 1) {
					( * ew)[ecount] = atoi(temp);
					temp = strtok(NULL, " \t\n");
				} else {
					( * ew)[ecount] = 1;
				}

				if (( * ew)[ecount] < 0) {
					fprintf(stderr, "negative edge weight %lf at (%ld,%ld).\n", ( * ew)[ecount], (long) vcount, (long)(( * adj)[ecount]));
					return -1;
				}
				ecount++;
			}

			vcount++;
			( * xadj)[vcount] = ecount;
		}
	}

	if (vcount != num_vertex) {
		fprintf(stderr, "num vertex %ld != %ld.\n", (long) num_vertex, (long) vcount);
		return -1;
	}

	if (ecount != 2 * num_edge) {
		fprintf(stderr, "num edge %ld != %ld.\n", (long) ecount, (long)(2 * num_edge));
		( * adj) = (vertex_t * ) realloc(( * adj), sizeof(vertex_t) * ecount);
		( * ew) = (eweight_t * ) realloc(( * ew), sizeof(eweight_t) * ecount);
	}

	for (jv = 0; jv < vcount; jv++) {
		qsort(( * adj) + ( * xadj)[jv], ( * xadj)[jv + 1] - ( * xadj)[jv], sizeof(vertex_t), cmp);
	}

	return 1;
}

int read_mtx(FILE * fp, edge_t ** xadj, vertex_t ** adj,
	eweight_t ** ew, vweight_t ** vw,
	vertex_t * n_vertex, int loop, int offset) {

	Edge * edge;
	// lower bound and upper bound
	vertex_t l, r;
	vertex_t u, v;
	vertex_t M, N, wi;
	eweight_t w;
	int n_edge, curr_edge;
	edge_t k;

	MM_typecode matcode;

	if (mm_read_banner(fp, & matcode) != 0) 
	{
		fprintf(stderr, "fail at mm_read_banner.\n");
		return -1;
	}
	if (mm_read_mtx_crd_size(fp, & M, & N, & n_edge) != 0) 
	{
		fprintf(stderr, "fail at mm_read_mtx_crd_size.\n");
		return -1;
	}
	if (M != N) 
	{
		fprintf(stderr, "fail to assert M == N.\n");
		return -1;
	}

	* n_vertex = N;

	l = 1 - offset;
	r = N - offset;

	edge = (Edge * ) malloc(2 * n_edge * sizeof(Edge));
	curr_edge = 0;
	if (mm_is_pattern(matcode)) 
	{
		for (edge_t i = 0; i < n_edge; i++) {
			fscanf(fp, "%d %d\n", & u, & v);

			if (u < l || v < l || u > r || v > r) {
				fprintf(stderr, 
					"coord (%ld,%ld) not in range [%ld,%ld].\n", 
					(long) u, (long) v, (long) l, (long) r);
				return -1;
			}

			if (loop || u != v) {
				edge[curr_edge].u = u;
				edge[curr_edge].v = v;
				edge[curr_edge].w = 1;
				curr_edge++;
				if (mm_is_symmetric(matcode) && u != v) {
					edge[curr_edge].u = edge[curr_edge - 1].v;
					edge[curr_edge].v = edge[curr_edge - 1].u;
					edge[curr_edge].w = edge[curr_edge - 1].w;
					curr_edge++;
				}
			}
		}
	} else {
		for (edge_t i = 0; i < n_edge; i++) {
			if (mm_is_real(matcode))
				fscanf(fp, "%d %d %lf\n", & u, & v, & w);
			else if (mm_is_integer(matcode))
				fscanf(fp, "%d %d %d\n", & u, & v, & wi);

			w = (double) wi;

			if (u < l || v < l || u > r || v > r) {
				fprintf(stderr, 
					"coord (%ld,%ld) not in range [%ld,%ld].\n", 
					(long) u, (long) v, (long) l, (long) r);
				return -1;
			}

			if (w != 0 && (loop || u != v)) {
				edge[curr_edge].u = u;
				edge[curr_edge].v = v;
				edge[curr_edge].w = fabs(w);
				curr_edge++;
				if (mm_is_symmetric(matcode) && u != v) {
					edge[curr_edge].u = edge[curr_edge - 1].v;
					edge[curr_edge].v = edge[curr_edge - 1].u;
					edge[curr_edge].w = edge[curr_edge - 1].w;
					curr_edge++;
				}
			}
		}
	}

	qsort(edge, curr_edge, sizeof(Edge), tricmp);

	( * xadj) = (edge_t * ) malloc(sizeof(edge_t) * (N + 1));
	memset(( * xadj), 0, sizeof(edge_t) * (N + 1));

	k = 0;
	( * xadj)[edge[0].u + offset]++;
	for (edge_t ei = 1; ei < curr_edge; ei++) {
		u = edge[ei].u;
		if (u != edge[ei - 1].u || edge[ei].v != edge[ei - 1].v) {
			// not same as previous
			( * xadj)[u + offset]++;
			k = u;
		} else {
			edge[k].w += edge[u].w;
		}
	}
	for (u = 2; u <= N; u++)
		( * xadj)[u] += ( * xadj)[u - 1];

	( * adj) = (vertex_t * ) malloc(sizeof(vertex_t) * ( * xadj)[N]);
	( * ew) = (eweight_t * ) malloc(sizeof(eweight_t) * ( * xadj)[N]);
	( * adj)[0] = edge[0].v - 1 + offset;
	( * ew)[0] = edge[0].w;
	k = 1;

	for (edge_t ei = 1; ei < curr_edge; ei++) {
		u = edge[ei].u;
		if (u != edge[ei - 1].u || edge[ei].v != edge[ei - 1].v) {
			// not same as previous
			( * adj)[k] = edge[ei].v - 1 + offset;
			( * ew)[k++] = edge[ei].w;
		}
	}

	( * vw) = (vweight_t * ) malloc(sizeof(vweight_t) * N);
	for (u = 0; u < N; u++)
		( * vw)[u] = 1;

	free(edge);
	return 0;
}

int read_cache(FILE * bp, edge_t ** xadj, vertex_t ** adj,
	eweight_t ** ew, vweight_t ** vw, vertex_t * n_vertex) {

	fread(n_vertex, sizeof(vertex_t), 1, bp);

	( * xadj) = (edge_t * ) malloc(sizeof(edge_t) * ( * n_vertex + 1));
	fread( * xadj, sizeof(edge_t), (size_t)( * n_vertex + 1), bp);

	( * adj) = (vertex_t * ) malloc(sizeof(vertex_t) * ( * xadj)[ * n_vertex]);
	fread( * adj, sizeof(vertex_t), (size_t)( * xadj)[ * n_vertex], bp);

	( * ew) = (eweight_t * ) malloc(sizeof(eweight_t) * ( * xadj)[ * n_vertex]);
	fread( * ew, sizeof(eweight_t), (size_t)( * xadj)[ * n_vertex], bp);

	( * vw) = (vweight_t * ) malloc(sizeof(vweight_t) * ( * n_vertex));
	fread( * vw, sizeof(vweight_t), * n_vertex, bp);

	return 0;
}

int save_cache(FILE * bp, edge_t * xadj, vertex_t * adj,
	eweight_t * ew, vweight_t * vw, vertex_t n_vertex) {

	fwrite( & n_vertex, sizeof(vertex_t), (size_t) 1, bp);
	fwrite(xadj, sizeof(edge_t), (size_t)(n_vertex + 1), bp);
	fwrite(adj, sizeof(vertex_t), (size_t)(xadj[n_vertex]), bp);
	fwrite(ew, sizeof(eweight_t), (size_t)(xadj[n_vertex]), bp);
	fwrite(vw, sizeof(vweight_t), (size_t)(n_vertex), bp);

	return 0;
}

int read_graph(char * gpath, edge_t ** xadj, vertex_t ** adj,
	eweight_t ** ew, vweight_t ** vw, vertex_t * n_vertex, int loop) {

	char bpath[1024];
	FILE * bp, * fp;
	// read status
	int status = 0;

	// read from binary cache, if possible
	sprintf(bpath, "%s.bin", gpath);
	bp = fopen(bpath, "rb");
	if (bp != NULL) 
	{
		status = read_cache(bp, xadj, adj, ew, vw, n_vertex);
		fclose(bp);
		if (status == -1)
		{
			fprintf(stderr, "fail to read cache.\n");
			return -1;
		}
		return 0;
	}

	// no binary, read from raw and save a cache
	fp = fopen(gpath, "r");
	if (fp == NULL) 
	{
		fprintf(stderr, "fail to find file.\n");
		return -1;
	}

	if (ends_with(gpath, ".mtx"))
		status = read_mtx(fp, xadj, adj, ew, vw, n_vertex, loop, 0);
	else if (ends_with(gpath, ".txt"))
		status = read_mtx(fp, xadj, adj, ew, vw, n_vertex, loop, 1);
	else if (ends_with(gpath, ".graph"))
		status = read_chaco(fp, xadj, adj, ew, vw, n_vertex, loop);
	else
		status = -1;

	fclose(fp);
	if (status == -1)
	{
		fprintf(stderr, "fail to read.\n");
		return -1;
	}

	// write to binary cache, so next time its faster
	bp = fopen(bpath, "wb");
	if (bp != NULL) 
	{
		status = save_cache(bp, *xadj, *adj, *ew, *vw, *n_vertex);
		fclose(bp);
	}
	if (bp == NULL || status == -1)
	{
		fprintf(stderr, "fail to save cache.\n");
		return -1;
	}

	return 0;
}