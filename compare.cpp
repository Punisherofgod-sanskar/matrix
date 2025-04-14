// File: compare.cpp
// Purpose: Compare two matrices A and B to check if they are approximately equal
// Notes: Used to verify results like A ≈ L*U in LU decomposition

#include <iostream>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <chrono>

using namespace std;
using namespace chrono;

// Helper function to compute 1D index from 2D coordinates
int idx(int i, int j, int n) {
    return i * n + j;
}

// Generates a random n x n integer matrix with values between 0 and 9
vector<int> generateMatrix(int n) {
    vector<int> mat(n * n);
    for (int i = 0; i < n * n; ++i)
        mat[i] = rand() % 10;
    return mat;
}

// Performs element-wise addition of matrices A and B
vector<int> add(const vector<int>& A, const vector<int>& B, int n) {
    vector<int> C(n * n);
    for (int i = 0; i < n * n; ++i)
        C[i] = A[i] + B[i];
    return C;
}

// Performs element-wise subtraction of matrices A and B
vector<int> subtract(const vector<int>& A, const vector<int>& B, int n) {
    vector<int> C(n * n);
    for (int i = 0; i < n * n; ++i)
        C[i] = A[i] - B[i];
    return C;
}

// Standard cubic-time matrix multiplication: C = A × B
vector<int> standardMultiply(const vector<int>& A, const vector<int>& B, int n) {
    vector<int> C(n * n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < n; ++k)
                C[idx(i, j, n)] += A[idx(i, k, n)] * B[idx(k, j, n)];
    return C;
}

// Forward declaration of Strassen’s algorithm
vector<int> strassen(const vector<int>& A, const vector<int>& B, int n);

// Implementation of Winograd’s variant of Strassen’s algorithm
vector<int> winogradStrassen(const vector<int>& A, const vector<int>& B, int n) {
    // Base case: 1x1 matrix multiplication
    if (n == 1)
        return { A[0] * B[0] };

    // Partition matrix size
    int newSize = n / 2;
    int size = newSize * newSize;

    // Submatrices
    vector<int> A11(size), A12(size), A21(size), A22(size);
    vector<int> B11(size), B12(size), B21(size), B22(size);

    // Fill submatrices
    for (int i = 0; i < newSize; ++i) {
        for (int j = 0; j < newSize; ++j) {
            int i1 = idx(i, j, newSize);
            A11[i1] = A[idx(i, j, n)];
            A12[i1] = A[idx(i, j + newSize, n)];
            A21[i1] = A[idx(i + newSize, j, n)];
            A22[i1] = A[idx(i + newSize, j + newSize, n)];

            B11[i1] = B[idx(i, j, n)];
            B12[i1] = B[idx(i, j + newSize, n)];
            B21[i1] = B[idx(i + newSize, j, n)];
            B22[i1] = B[idx(i + newSize, j + newSize, n)];
        }
    }

    // Strassen intermediate matrices S1–S10
    auto S1 = subtract(B12, B22, newSize);
    auto S2 = add(A11, A12, newSize);
    auto S3 = add(A21, A22, newSize);
    auto S4 = subtract(B21, B11, newSize);
    auto S5 = add(A11, A22, newSize);
    auto S6 = add(B11, B22, newSize);
    auto S7 = subtract(A12, A22, newSize);
    auto S8 = add(B21, B22, newSize);
    auto S9 = subtract(A11, A21, newSize);
    auto S10 = add(B11, B12, newSize);

    // Strassen recursive products P1–P7
    auto P1 = strassen(A11, S1, newSize);
    auto P2 = strassen(S2, B22, newSize);
    auto P3 = strassen(S3, B11, newSize);
    auto P4 = strassen(A22, S4, newSize);
    auto P5 = strassen(S5, S6, newSize);
    auto P6 = strassen(S7, S8, newSize);
    auto P7 = strassen(S9, S10, newSize);

    // Reconstruct result quadrants using Strassen recombination formulas
    auto C11 = add(subtract(add(P5, P4, newSize), P2, newSize), P6, newSize);
    auto C12 = add(P1, P2, newSize);
    auto C21 = add(P3, P4, newSize);
    auto C22 = subtract(subtract(add(P5, P1, newSize), P3, newSize), P7, newSize);

    // Merge submatrices into result matrix C
    vector<int> C(n * n);
    for (int i = 0; i < newSize; ++i) {
        for (int j = 0; j < newSize; ++j) {
            int i1 = idx(i, j, newSize);
            C[idx(i, j, n)] = C11[i1];
            C[idx(i, j + newSize, n)] = C12[i1];
            C[idx(i + newSize, j, n)] = C21[i1];
            C[idx(i + newSize, j + newSize, n)] = C22[i1];
        }
    }
    return C;
}

// Wrapper for Strassen's algorithm: switches to standard method for small sizes
vector<int> strassen(const vector<int>& A, const vector<int>& B, int n) {
    if (n <= 64) return standardMultiply(A, B, n);
    return winogradStrassen(A, B, n);
}

// Helper to convert raw timing to readable string with appropriate units
string formatTime(double seconds) {
    ostringstream out;
    out << fixed << setprecision(3);
    if (seconds < 1e-3)
        out << seconds * 1e6 << " µs";
    else if (seconds < 1.0)
        out << seconds * 1e3 << " ms";
    else
        out << seconds << " s";
    return out.str();
}

int main() {
    // Set response header for HTML output
    cout << "Content-type: text/html\n\n";

    // Use <pre> for preformatted colored output
    cout << "<pre style='color: #00ffe1; font-size: 1rem;'>";

    // Read input from form (e.g., size=8)
    string data;
    getline(cin, data);
    int n = 0;
    auto pos = data.find("=");
    if (pos != string::npos)
        n = stoi(data.substr(pos + 1));

    // Ensure input size is a power of 2
    if (n <= 0 || (n & (n - 1)) != 0) {
        cout << "Please enter a power-of-two matrix size (e.g., 2, 4, 8, 16...)</pre>";
        return 0;
    }

    // Generate random matrices A and B
    auto A = generateMatrix(n);
    auto B = generateMatrix(n);

    // Time standard multiplication
    auto t1 = high_resolution_clock::now();
    auto C1 = standardMultiply(A, B, n);
    auto t2 = high_resolution_clock::now();

    // Time Strassen’s multiplication
    auto t3 = high_resolution_clock::now();
    auto C2 = strassen(A, B, n);
    auto t4 = high_resolution_clock::now();

    // Time Winograd variant
    auto t5 = high_resolution_clock::now();
    auto C3 = winogradStrassen(A, B, n);
    auto t6 = high_resolution_clock::now();

    // Compute durations
    duration<double> time1 = t2 - t1;
    duration<double> time2 = t4 - t3;
    duration<double> time3 = t6 - t5;

    // Display timing results
    cout << "Matrix Size: " << n << " × " << n << "\n\n";
    cout << "Standard Algorithm Time: " << formatTime(time1.count()) << "\n";
    cout << "Strassen's Algorithm Time: " << formatTime(time2.count()) << "\n";
    cout << "Winograd's Variant Time: " << formatTime(time3.count()) << "\n";
    cout << "</pre>";

    // Hidden HTML div to pass raw data to frontend JavaScript (e.g., for graph)
    cout << "<div id='chart-data' style='display:none;' ";
    cout << "data-size='" << n << "' ";
    cout << "data-standard='" << time1.count() << "' ";
    cout << "data-strassen='" << time2.count() << "' ";
    cout << "data-winograd='" << time3.count() << "'></div>";

    return 0;
}
