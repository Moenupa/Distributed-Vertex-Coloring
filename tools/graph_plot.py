import numpy as np
import networkx as nx
import matplotlib.pyplot as plt


def read_matrix(path: str = 'example/1.txt') -> np.ndarray:
    mat = np.genfromtxt(path, delimiter=' ', skip_header=1)
    
    # if the graph is not colored
    if mat[0][0] == -1:
        for i in range(mat.shape[0]):
            mat[i][i] = i
            
    return mat


def visualize(mat: np.ndarray):
    n = mat.shape[0]
    
    edges = [(i, j) for i in range(n) for j in range(i) if mat[i][j] == 1]
    G = nx.Graph()
    G.add_nodes_from([i for i in range(n)])
    G.add_edges_from(edges)
    
    if n < 300:
        nx.draw(G, node_color=[mat[i][i] for i in range(n)], 
                with_labels=True, font_color='whitesmoke')
        plt.savefig('res/test.png')
        
        
if __name__ == '__main__':
    file = read_matrix()
    visualize(file)
    print(file)