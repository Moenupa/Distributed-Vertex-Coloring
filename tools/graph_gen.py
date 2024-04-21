import argparse
from random import randint, sample

def parse_args():
    parser = argparse.ArgumentParser(description='Generate a random graph')
    parser.add_argument('-v', '--n-vertex', type=int, default=10, help='No. of Vertices (default: 1000)')
    parser.add_argument('-e', '--n-edge', type=int, default=10, help='No. of Edges (default: 1000)')
    return parser.parse_args()


def generate(n_vertex: int, n_edge: int):
    graph = [[0 for _ in range(n_vertex)] for _ in range(n_vertex)]
        
    count = 0
    while count < n_edge:
        u, v = sample(range(n_vertex), 2)
        if graph[u][v] == 0:
            graph[u][v] = graph[v][u] = 1
            count +=1
            
    for i in range(n_vertex):
        graph[i][i] = -1

    # deg = max(sum(row) for row in graph)

    print(n_vertex, n_edge)

    for row in graph:
        print(' '.join(map(str, row)))
 

if __name__ == '__main__':
    args = parse_args()
    generate(args.n_vertex, args.n_edge)