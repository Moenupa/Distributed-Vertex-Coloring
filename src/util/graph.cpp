#include <iostream>
#include <vector>

using namespace std;

void parse_stdin(vector<vector<int>>& mat, int size) {
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            cin >> mat[i][j];
        }
    }
}

void print_matrix(vector<vector<int>>& mat, int size) {
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            cout << mat[i][j] << " ";
        }
        cout << endl;
    }
}

bool is_color_available(int u, int c, const vector<vector<int>> &mat, int s, int e)
{
    for (int v = s; v < e; ++v)
    {
        if (mat[u][v] && mat[v][v] == c)
        {
            return false;
        }
    }
    return true;
}

void firstfit(vector<vector<int>> &mat, int s, int e)
{
    int len = e-s;

    for (int u = s; u < e; ++u)
    {
        for (int c = 0; c < len; ++c)
        {
            if (mat[u][u] == -1 && is_color_available(u, c, mat, s, e))
            {
                mat[u][u] = c;
                break;
            }
        }
        cout << "Vertex " << u << " is colored with " << mat[u][u] << endl;
    }
}