import argparse
import os
import numpy as np
import networkx as nx
import matplotlib.pyplot as plt
import seaborn as sns


RES_PATH = 'res'


def firstfit(mat: np.ndarray) -> np.ndarray:
    n = mat.shape[0]

    colormap = np.zeros(n, dtype=int)
    color_used = [False] * (n+1)

    for i in range(n):
        for j in range(n):
            if mat[i][j] == 1 and colormap[j] != 0:
                color_used[colormap[j]] = True

        for j in range(1, n+1):
            if not color_used[j]:
                colormap[i] = j
                break

        for j in range(n):
            if mat[i][j] == 1 and colormap[j] != 0:
                color_used[colormap[j]] = False

    return colormap


def read_matrix(path: str) -> np.ndarray:
    mat = np.genfromtxt(path, delimiter=' ', skip_header=1)
    return mat


def visualize(mat: np.ndarray, do_coloring: bool = False) -> None:
    os.makedirs(RES_PATH, exist_ok=True)
    n = mat.shape[0]

    edges = [(i, j) for i in range(n) for j in range(i) if mat[i][j] == 1]
    G = nx.Graph()
    G.add_nodes_from([i for i in range(n)])
    G.add_edges_from(edges)

    if do_coloring:
        colormap = firstfit(mat)
    else:
        colormap = np.arange(1, n+1)
    colormark = 'c' if do_coloring else 'x'

    if n <= 300:
        plt.figure(figsize=(8, 6))
        plt.title(f'|V|={n} |C|={colormap.max()} Graph')
        nx.draw(G, node_color=colormap.tolist(),
                with_labels=(n < 50), font_color='whitesmoke',
                node_size=(300 if n < 50 else 100),
                edge_color='#999')
        plt.savefig(
            f'{RES_PATH}/{n}_{colormark}_graph.png',
            dpi=300
        )
        plt.clf()

    if n <= 1000:
        plt.figure(figsize=(8, 6))
        plt.title(f'|V|={n} |C|={colormap.max()} Heatmap')
        sns.heatmap(mat, xticklabels=False, yticklabels=False,
                    cmap=sns.light_palette("seagreen", as_cmap=True))
        plt.savefig(
            f'{RES_PATH}/{n}_{colormark}_heatmap.png',
            dpi=300
        )
        plt.clf()


def parse_args():
    parser = argparse.ArgumentParser(description='Visualize a graph')
    parser.add_argument('-f', '--file', type=str, metavar='PATH', required=True,
                        help='Paht of matrix file')
    parser.add_argument('-c', '--color', dest='coloring', action='store_true',
                        help='Generate coloring')
    return parser.parse_args()


if __name__ == '__main__':
    args = parse_args()
    mat = read_matrix(args.file)
    visualize(mat, args.coloring)
