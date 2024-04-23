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

inline int max(vertex_t len, int colormap[])
{
	int val = -1;
	for (int i = 0; i < len; i++)
		if (colormap[i] > val)
			val = colormap[i];
	return val + 1;
}

namespace D2Coloring
{
	/**
	 * @brief Find number of conflicts in the graph
	 * 
	 * @param row: row pointer
	 * @param col: column pointer
	 * @param n_vertex: number of vertices
	 * @param colormap: color array shaped (n_vertex, )
	 * @param heatmap: map to track detected conflicts
	 * @param conflict_vid: output array to store conflicted vertices
	*/
	int detect_conflicts(edge_t *row, vertex_t *col, vertex_t n_vertex, int colormap[], bool heatmap[], int conflict_vid[])
	{
		unsigned int count = 0;
		#pragma omp parallel for
		for (int i = 0; i < n_vertex; i++)
		{
			int c = colormap[i];
			int vid, temp;
			for (int j = row[i]; j < row[i + 1]; j++)
			{
				if (colormap[col[j]] == c)
				{
					vid = i < col[j] ? i : col[j];
					if (!heatmap[vid])
					{
						heatmap[vid] = true;
						#pragma omp atomic capture
						temp = count++;
						conflict_vid[temp] = vid;
					}
				}

				for (int k = row[col[j]]; k < row[col[j] + 1]; k++)
				{
					if (colormap[col[k]] == c && col[k] != i)
					{
						vid = i < col[k] ? i : col[k];
						if (!heatmap[vid])
						{
							heatmap[vid] = true;
							#pragma omp atomic capture
							temp = count++;
							conflict_vid[temp] = vid;
						}
					}
				}
			}
		}

		#pragma omp parallel for
		for (edge_t e = 0; e < count; e++)
			heatmap[conflict_vid[e]] = false;

		return count;
	}

	/**
	 * @brief Simple First Fit algorithm that always finds the smallest available color for the vertex
	 *
	 * @param vid: vertex id
	 * @param row: row pointer
	 * @param col: column pointer
	 * @param n_vertex: number of vertices
	 * @param colormap: color array shaped (n_vertex, )
	 * @param color_used: array to track used colors
	 */
	int firstfit(int vid, edge_t *row, vertex_t *col, vertex_t n_vertex, int colormap[], bool color_used[])
	{
		int row_l = row[vid];
		int row_r = row[vid + 1];

		// track whether a color is used it not
		for (int i = row_l; i < row_r; i++)
		{
			int c = colormap[col[i]];
			if (c >= 0)
				color_used[c] = true;

			for (int j = row[col[i]]; j < row[col[i] + 1]; j++)
			{
				c = colormap[col[j]];
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
				int c = colormap[col[i]];
				if (c >= 0)
					color_used[c] = false;

				for (edge_t j = row[col[i]]; j < row[col[i] + 1]; j++)
				{
					c = colormap[col[j]];
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
	 * @param colormap: color array shaped (n_vertex, )
	 */
	report color_graph_seq(edge_t *row, vertex_t *col, vertex_t n_vertex, int colormap[])
	{
		report result;
		double t_start, t_end;
		int n_color = 0;
		bool *color_used = new bool[n_vertex + 1]();

		t_start = omp_get_wtime();
		for (int i = 0; i < n_vertex; i++)
		{
			int c = firstfit(i, row, col, n_vertex, colormap, color_used);
			colormap[i] = c;
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

	report color_graph_par(edge_t *row, vertex_t *col, vertex_t n_vertex, int colormap[])
	{
		report result;
		double t_start, t_end;
		int n_merge_conflict = -1;

		int *conflicts = new int[n_vertex / 2 + 1]();
		bool *heatmap = new bool[n_vertex]();
		static bool *color_used;
		#pragma omp threadprivate(color_used)

		#pragma omp parallel
		{
			color_used = new bool[n_vertex + 1]();
		}

		t_start = omp_get_wtime();
		#pragma omp parallel for
		for (int i = 0; i < n_vertex; i++)
		{
			int c = firstfit(i, row, col, n_vertex, colormap, color_used);
			colormap[i] = c;
		}

		int n_conflict = 0;
		do
		{
			// detect conflicted vertices and recolor
			n_conflict = detect_conflicts(row, col, n_vertex, colormap, heatmap, conflicts);
			#pragma omp for
			for (int i = 0; i < n_conflict; i++)
			{
				int c = firstfit(conflicts[i], row, col, n_vertex, colormap, color_used);
				colormap[conflicts[i]] = c;
			}
			++n_merge_conflict;
		} while (n_conflict > 0);
		t_end = omp_get_wtime();

		// clean up
		delete[] heatmap;
		delete[] conflicts;
		#pragma omp parallel
		{
			delete[] color_used;
		}
		result.n_color = max(n_vertex, colormap);
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

	int *colormap = new int[n_vertex];
	fill_n(colormap, n_vertex, -1);

	print_header();

	// Sequential versions
	r = D2Coloring::color_graph_seq(row_ptr, col_ind, n_vertex, colormap);

	int conflicts;

	// these two are used in the detect_conflicts, for correctness we only need to check conflict count.
	bool *heatmap = new bool[n_vertex]();
	int *conflict_vid = new int[n_vertex]();

	omp_set_num_threads(1);
	conflicts = D2Coloring::detect_conflicts(row_ptr, col_ind, n_vertex, colormap, heatmap, conflict_vid);
	fill_n(heatmap, n_vertex, false);

	print_report(1, r, "Sequential", conflicts);

	// Parallel versions
	int threads = 1;
	while (threads <= max_threads)
	{
		fill_n(colormap, n_vertex, -1); // reinitialize
		omp_set_num_threads(threads);

		r = D2Coloring::color_graph_par(row_ptr, col_ind, n_vertex, colormap);

		omp_set_num_threads(1);
		conflicts = D2Coloring::detect_conflicts(row_ptr, col_ind, n_vertex, colormap, heatmap, conflict_vid);
		fill_n(heatmap, n_vertex, false);

		print_report(threads, r, "Parallel", conflicts);

		threads <<= 1;
	}

	return 0;
}