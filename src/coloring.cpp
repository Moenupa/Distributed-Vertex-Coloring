#include "utils/graphio.h"
#include "utils/graph.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <unordered_set>
#include <omp.h>

/**
 * @brief Report wrapper for result and performance.
 *
 * @param t_exec Execution time
 * @param n_color Number of colors
 * @param n_conflict Number of conflicts
 */
typedef struct report
{
	double t_exec;
	int n_color;
	int n_conflict;
} report;

void print_header()
{
	printf(" %-10s | %-10s | %-15s | %-10s | %-14s | %-10s\n",
		   "Algorithm",
		   "# Threads",
		   "# Conf.Fixes",
		   "# Colors",
		   "T Exec.  (s)",
		   "# Conf.");
}

void print_report(int n_thread, report r, std::string note, int conflicts)
{
	printf(" %-10s | %-10d | %-15d | %-10d | %-14.10f | %d\n",
		   note.c_str(),
		   n_thread,
		   r.n_conflict,
		   r.n_color,
		   r.t_exec,
		   conflicts);
}

inline int max(vertex_t len, int colors[])
{
	int val = -1;
	for (int i = 0; i < len; i++)
		if (colors[i] > val)
			val = colors[i];
	return val + 1;
}

namespace D2Coloring
{
	/*
	Distance-2 Graph coloring
	Traverses entire graph to find conflicts (i.e. adjecent vertices with same color), returns the number of such
	conflicts as return value, indices of these vertices are written into out array.

	*row and *col: pointers define the starting point of the graph
	n_vertex:		   number of vertices in the graph
	colors:		   the array storing color values assigned to each vertex (-1 means unassigned)
	isDetected:    the array that will be used as temporal storage to find whether the vertex is already marked as
	conflict, initially all values assumed to be false (taken as parameter because of efficiency considerations)
	out:		   output array that contains all the vertices marked to be re-colored (size >= n_vertex/2)

	returns:       number of conflicts detected.
	*/
	int detect_conflicts(edge_t *row, vertex_t *col, vertex_t n_vertex, int colors[], bool isDetected[], int out[])
	{
		unsigned int index = 0;
		int c, colStart, colEnd, d2colStart, d2colEnd, conflictIndex, temp;
		int i, j, k;
#pragma omp parallel for private(j, k, c, colStart, colEnd, d2colStart, d2colEnd, conflictIndex, temp)
		for (i = 0; i < n_vertex; i++)
		{
			c = colors[i];
			colStart = row[i];
			colEnd = row[i + 1];
			for (j = colStart; j < colEnd; j++)
			{
				if (colors[col[j]] == c)
				{
					conflictIndex = i < col[j] ? i : col[j];
					if (!isDetected[conflictIndex])
					{
						isDetected[conflictIndex] = true;
#pragma omp atomic capture
						temp = index++;
						out[temp] = conflictIndex;
					}
				}

				d2colStart = row[col[j]];
				d2colEnd = row[col[j] + 1];
				for (k = d2colStart; k < d2colEnd; ++k)
				{
					if (colors[col[k]] == c && col[k] != i)
					{
						conflictIndex = i < col[k] ? i : col[k];
						if (!isDetected[conflictIndex])
						{
							isDetected[conflictIndex] = true;
#pragma omp atomic capture
							temp = index++;
							out[temp] = conflictIndex;
						}
					}
				}
			}
		}

		// reset isDetected array
#pragma omp parallel for
		for (edge_t e = 0; e < index; e++)
			isDetected[out[e]] = false;

		return index;
	}

	/**
	 * @brief Simple First Fit algorithm that always finds the smallest available color for the vertex
	 *
	 * @param vid: vertex id
	 * @param row: row pointer
	 * @param col: column pointer
	 * @param n_vertex: number of vertices
	 * @param colors: color array shaped (n_vertex, )
	 * @param color_used: array to track used colors
	 */
	int firstfit(int vid, edge_t *row, vertex_t *col, vertex_t n_vertex, int colors[], bool color_used[])
	{
		int row_l = row[vid];
		int row_r = row[vid + 1];

		// track whether a color is used it not
		for (int i = row_l; i < row_r; i++)
		{
			int c = colors[col[i]];
			if (c >= 0)
				color_used[c] = true;

			int d2colStart = row[col[i]], d2colEnd = row[col[i] + 1];
			for (int j = d2colStart; j < d2colEnd; j++)
			{
				c = colors[col[j]];
				if (c >= 0 && col[j] != vid)
					color_used[c] = true;
			}
		}

		// return the smallest unused color
		for (int c = 0; c < n_vertex + 1; c++)
		{
			if (color_used[c])
				continue;
			
			for (int i = row_l; i < row_r; i++)
			{
				int c = colors[col[i]];
				if (c >= 0)
					color_used[c] = false;

				for (edge_t j = row[col[i]]; j < row[col[i] + 1]; j++)
				{
					c = colors[col[j]];
					if (c >= 0 && col[j] != vid)
						color_used[c] = false;
				}
			}
			return c;
		}

		throw std::runtime_error("exhaust color limit |v|+1");
	}

	/**
	 * @brief Color the graph sequentially
	 * 
	 * @param row: row pointer
	 * @param col: column pointer
	 * @param n_vertex: number of vertices
	 * @param colors: color array shaped (n_vertex, )
	*/
	report color_graph_seq(edge_t *row, vertex_t *col, vertex_t n_vertex, int colors[])
	{
		report result;
		double t_start, t_end;
		int n_color = 0;
		bool *color_used = new bool[n_vertex + 1]();

		t_start = omp_get_wtime();
		for (int i = 0; i < n_vertex; i++)
		{
			int c = firstfit(i, row, col, n_vertex, colors, color_used);
			colors[i] = c;
			if (c > n_color)
				n_color = c;
		}
		t_end = omp_get_wtime();
		delete[] color_used;

		result.n_color = n_color + 1;
		result.t_exec = t_end - t_start;
		result.n_conflict = 0;

		return result;
	}

	report color_graph_par(edge_t *row, vertex_t *col, vertex_t n_vertex, int colors[])
	{
		report result;
		double t_start, t_end;
		int n_merge_conflict = -1;

		int confArrSize = n_vertex / 2 + 1;
		int *conflictedVertices = new int[confArrSize]();
		bool *isVertexDetected = new bool[n_vertex]();
		static bool *color_used;
#pragma omp threadprivate(color_used)

#pragma omp parallel
		{
			color_used = new bool[n_vertex + 1]();
		}

		// first stage coloring
		int i, c;
		t_start = omp_get_wtime();
#pragma omp parallel for private(c)
		for (i = 0; i < n_vertex; i++)
		{
			c = firstfit(i, row, col, n_vertex, colors, color_used);
			colors[i] = c;
		}

		int n_conflict = 0;
		do
		{
			// detect conflicted vertices and recolor
			n_conflict = detect_conflicts(row, col, n_vertex, colors, isVertexDetected, conflictedVertices);
#pragma omp for private(c)
			for (i = 0; i < n_conflict; i++)
			{
				c = firstfit(conflictedVertices[i], row, col, n_vertex, colors, color_used);
				colors[conflictedVertices[i]] = c;
			}
			++n_merge_conflict;
		} while (n_conflict > 0);
		t_end = omp_get_wtime();

		// clean up
		delete[] isVertexDetected;
		delete[] conflictedVertices;
#pragma omp parallel
		{
			delete[] color_used;
		}
		result.n_color = max(n_vertex, colors);
		result.t_exec = t_end - t_start;
		result.n_conflict = n_merge_conflict;
		return result;
	}
}

int main(int argc, char *argv[])
{
	using namespace std;

	// program called with ./coloring
	// should be ./coloring [FILE] [MAX_THREADS]
	if (argc < 2)
	{
		cout << "Usage: ./coloring [FILE] [THREADS]" << endl;
		exit(EXIT_FAILURE);
	}
	
	int max_threads = 16;
	if (argc == 3)
	{
		max_threads = min(stoi(argv[2]), omp_get_max_threads());
	}

	edge_t *row_ptr;
	vertex_t *col_ind;
	eweight_t *ewghts;
	vweight_t *vwghts;
	vertex_t n_vertex;

	if (read_graph(argv[1], &row_ptr, &col_ind, &ewghts, &vwghts, &n_vertex, 0) == -1)
	{
		cout << "error in graph read" << endl;
		exit(EXIT_FAILURE);
	}

	// Performance analysis
	report r;

	int *colors = new int[n_vertex];
	fill_n(colors, n_vertex, -1);

	print_header();

	// Sequential versions
	r = D2Coloring::color_graph_seq(row_ptr, col_ind, n_vertex, colors);

	int conflicts;

	// these two are used in the detect_conflicts, for correctness we only need to check conflict count.
	bool *isDetected = new bool[n_vertex]();
	int *out = new int[n_vertex]();

	omp_set_num_threads(1);
	conflicts = D2Coloring::detect_conflicts(row_ptr, col_ind, n_vertex, colors, isDetected, out);
	fill_n(isDetected, n_vertex, false);

	print_report(1, r, "Sequential", conflicts);

	// Parallel versions
	int threads = 1;
	while (threads <= max_threads)
	{
		fill_n(colors, n_vertex, -1); // reinitialize
		omp_set_num_threads(threads);

		r = D2Coloring::color_graph_par(row_ptr, col_ind, n_vertex, colors);

		omp_set_num_threads(1);
		conflicts = D2Coloring::detect_conflicts(row_ptr, col_ind, n_vertex, colors, isDetected, out);
		fill_n(isDetected, n_vertex, false);

		print_report(threads, r, "Parallel", conflicts);

		threads <<= 1;
	}

	return 0;
}