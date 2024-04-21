#include <iostream>
#include <chrono>
#include "util/graph.cpp"

using namespace std;

vector<vector<int>> mat;

int main() {

    using chrono::duration;
    using chrono::high_resolution_clock;
    auto start = high_resolution_clock::now();

    int size;
    cin >> size;
    mat = vector<vector<int>>(size, vector<int>(size, 0));

    parse_stdin(mat, size);
    print_matrix(mat, size);

    firstfit(mat, 0, size);

    auto end = high_resolution_clock::now();
    duration<double, milli> time = end - start;

    cout << "Duration: " << time.count() << " miliseconds." << endl;
    return 0;
}
