#include <iostream>
#include <vector>

using namespace std;

class Graph
{
    int N;
    vector<vector<int>> mat;

public:
    Graph(int N) : N(N), mat(N, vector<int>(N, false)) {}

    void parseStdin() {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                cin >> mat[i][j];
            }
        }
    }

    void print() {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                cout << mat[i][j] << " ";
            }
            cout << endl;
        }
    }

    void addEdge(int u, int v)
    {
        if (u == v)
        {
            return;
        }

        mat[u][v] = true;
        mat[v][u] = true;
    }

    void sequentialVertexColoring()
    {
        vector<int> color(N, -1);

        for (int u = 0; u < N; ++u)
        {
            for (int c = 0; c < N; ++c)
            {
                if (color[u] == -1 && isColorAvailable(u, c, color))
                {
                    color[u] = c;
                    break;
                }
            }
        }

        // Print the colors
        for (int u = 0; u < N; ++u)
        {
            cout << "Vertex " << u << " is colored with " << color[u] << endl;
        }
    }

private:
    bool isColorAvailable(int u, int c, const vector<int> &color)
    {
        for (int v = 0; v < N; ++v)
        {
            if (mat[u][v] && color[v] == c)
            {
                return false;
            }
        }
        return true;
    }
};