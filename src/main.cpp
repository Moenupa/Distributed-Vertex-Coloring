#include <iostream>
#include "util/graph.cpp"

using namespace std;

int main() {
    int size;
    cin >> size;

    Graph g = Graph(size);
    g.parseStdin();
    g.print();

    g.sequentialVertexColoring();
    return 0;
}
