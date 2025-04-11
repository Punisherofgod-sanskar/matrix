#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <iomanip>

using namespace std;
using namespace chrono;

// Generate a square matrix filled with random integers
vector<vector<int>> generateMatrix(int n) {
    vector<vector<int>> mat(n, vector<int>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            mat[i][j] = rand() % 10;
    return mat;
}

// Standard O(n^3) multiplication
vector<vector<int>> standardMultiply(const vector<vector<int>>& A, const vector<vector<int>>& B) {
    int n = A.size();
    vector<vector<int>> C(n, vector<int>(n, 0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < n; ++k)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

// Helper for Strassen: add and subtract
vector<vector<int>> add(const vector<vector<int>>& A, const vector<vector<int>>& B) {
    int n = A.size();
    vector<vector<int>> result(n, vector<int>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            result[i][j] = A[i][j] + B[i][j];
    return result;
}

vector<vector<int>> subtract(const vector<vector<int>>& A, const vector<vector<int>>& B) {
    int n = A.size();
    vector<vector<int>> result(n, vector<int>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            result[i][j] = A[i][j] - B[i][j];
    return result;
}

// Strassen's recursive algorithm
vector<vector<int>> strassenMultiply(const vector<vector<int>>& A, const vector<vector<int>>& B) {
    int n = A.size();
    if (n == 1) {
        return { {A[0][0] * B[0][0]} };
    }

    int newSize = n / 2;
    vector<vector<int>> A11(newSize, vector<int>(newSize)),
        A12(newSize, vector<int>(newSize)),
        A21(newSize, vector<int>(newSize)),
        A22(newSize, vector<int>(newSize)),
        B11(newSize, vector<int>(newSize)),
        B12(newSize, vector<int>(newSize)),
        B21(newSize, vector<int>(newSize)),
        B22(newSize, vector<int>(newSize));

    for (int i = 0; i < newSize; i++) {
        for (int j = 0; j < newSize; j++) {
            A11[i][j] = A[i][j];
            A12[i][j] = A[i][j + newSize];
            A21[i][j] = A[i + newSize][j];
            A22[i][j] = A[i + newSize][j + newSize];

            B11[i][j] = B[i][j];
            B12[i][j] = B[i][j + newSize];
            B21[i][j] = B[i + newSize][j];
            B22[i][j] = B[i + newSize][j + newSize];
        }
    }

    auto M1 = strassenMultiply(add(A11, A22), add(B11, B22));
    auto M2 = strassenMultiply(add(A21, A22), B11);
    auto M3 = strassenMultiply(A11, subtract(B12, B22));
    auto M4 = strassenMultiply(A22, subtract(B21, B11));
    auto M5 = strassenMultiply(add(A11, A12), B22);
    auto M6 = strassenMultiply(subtract(A21, A11), add(B11, B12));
    auto M7 = strassenMultiply(subtract(A12, A22), add(B21, B22));

    auto C11 = add(subtract(add(M1, M4), M5), M7);
    auto C12 = add(M3, M5);
    auto C21 = add(M2, M4);
    auto C22 = add(subtract(add(M1, M3), M2), M6);

    vector<vector<int>> C(n, vector<int>(n));
    for (int i = 0; i < newSize; i++) {
        for (int j = 0; j < newSize; j++) {
            C[i][j] = C11[i][j];
            C[i][j + newSize] = C12[i][j];
            C[i + newSize][j] = C21[i][j];
            C[i + newSize][j + newSize] = C22[i][j];
        }
    }
    return C;
}

// Time formatting function
string formatTime(double seconds) {
    ostringstream out;
    out << fixed << setprecision(3);
    if (seconds < 1e-3) {
        out << seconds * 1e6 << " µs";
    }
    else if (seconds < 1.0) {
        out << seconds * 1e3 << " ms";
    }
    else {
        out << seconds << " s";
    }
    return out.str();
}

int main() {
    cout << "Content-type: text/html\n\n";
    cout << "<pre style='color: #00ffe1; font-size: 1rem;'>";

    string data;
    getline(cin, data);

    int n = 0;
    auto pos = data.find("=");
    if (pos != string::npos) {
        n = stoi(data.substr(pos + 1));
    }

    if (n <= 0 || (n & (n - 1)) != 0) {
        cout << "Please provide a valid power-of-two matrix size (e.g., 2, 4, 8, ...)";
        return 0;
    }

    auto A = generateMatrix(n);
    auto B = generateMatrix(n);

    auto start1 = high_resolution_clock::now();
    auto C1 = standardMultiply(A, B);
    auto end1 = high_resolution_clock::now();

    auto start2 = high_resolution_clock::now();
    auto C2 = strassenMultiply(A, B);
    auto end2 = high_resolution_clock::now();

    duration<double> time1 = end1 - start1;
    duration<double> time2 = end2 - start2;

    cout << "Matrix Size: " << n << " x " << n << "\n\n";
    cout << "Standard Algorithm Time: " << formatTime(time1.count()) << "\n";
    cout << "Strassen's Algorithm Time: " << formatTime(time2.count()) << "\n";

    cout << "</pre>";
    return 0;
}
