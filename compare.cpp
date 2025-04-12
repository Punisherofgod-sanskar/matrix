#include <iostream>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <chrono>

using namespace std;
using namespace chrono;

int idx(int i, int j, int n) {
    return i * n + j;
}

vector<int> generateMatrix(int n) {
    vector<int> mat(n * n);
    for (int i = 0; i < n * n; ++i)
        mat[i] = rand() % 10;
    return mat;
}

vector<int> add(const vector<int>& A, const vector<int>& B, int n) {
    vector<int> C(n * n);
    for (int i = 0; i < n * n; ++i)
        C[i] = A[i] + B[i];
    return C;
}

vector<int> subtract(const vector<int>& A, const vector<int>& B, int n) {
    vector<int> C(n * n);
    for (int i = 0; i < n * n; ++i)
        C[i] = A[i] - B[i];
    return C;
}

vector<int> standardMultiply(const vector<int>& A, const vector<int>& B, int n) {
    vector<int> C(n * n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < n; ++k)
                C[idx(i, j, n)] += A[idx(i, k, n)] * B[idx(k, j, n)];
    return C;
}

vector<int> strassen(const vector<int>& A, const vector<int>& B, int n);

vector<int> winogradStrassen(const vector<int>& A, const vector<int>& B, int n) {
    if (n == 1)
        return { A[0] * B[0] };

    int newSize = n / 2;
    int size = newSize * newSize;

    vector<int> A11(size), A12(size), A21(size), A22(size);
    vector<int> B11(size), B12(size), B21(size), B22(size);

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

    auto P1 = strassen(A11, S1, newSize);
    auto P2 = strassen(S2, B22, newSize);
    auto P3 = strassen(S3, B11, newSize);
    auto P4 = strassen(A22, S4, newSize);
    auto P5 = strassen(S5, S6, newSize);
    auto P6 = strassen(S7, S8, newSize);
    auto P7 = strassen(S9, S10, newSize);

    auto C11 = add(subtract(add(P5, P4, newSize), P2, newSize), P6, newSize);
    auto C12 = add(P1, P2, newSize);
    auto C21 = add(P3, P4, newSize);
    auto C22 = subtract(subtract(add(P5, P1, newSize), P3, newSize), P7, newSize);

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

vector<int> strassen(const vector<int>& A, const vector<int>& B, int n) {
    if (n <= 64) return standardMultiply(A, B, n);
    return winogradStrassen(A, B, n);
}

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
    cout << "Content-type: text/html\n\n";
    cout << "<pre style='color: #00ffe1; font-size: 1rem;'>";

    string data;
    getline(cin, data);
    int n = 0;
    auto pos = data.find("=");
    if (pos != string::npos)
        n = stoi(data.substr(pos + 1));

    if (n <= 0 || (n & (n - 1)) != 0) {
        cout << "Please enter a power-of-two matrix size (e.g., 2, 4, 8, 16...)</pre>";
        return 0;
    }

    auto A = generateMatrix(n);
    auto B = generateMatrix(n);

    auto t1 = high_resolution_clock::now();
    auto C1 = standardMultiply(A, B, n);
    auto t2 = high_resolution_clock::now();

    auto t3 = high_resolution_clock::now();
    auto C2 = strassen(A, B, n);
    auto t4 = high_resolution_clock::now();

    auto t5 = high_resolution_clock::now();
    auto C3 = winogradStrassen(A, B, n);
    auto t6 = high_resolution_clock::now();

    duration<double> time1 = t2 - t1;
    duration<double> time2 = t4 - t3;
    duration<double> time3 = t6 - t5;

    cout << "Matrix Size: " << n << " × " << n << "\n\n";
    cout << "Standard Algorithm Time: " << formatTime(time1.count()) << "\n";
    cout << "Strassen's Algorithm Time: " << formatTime(time2.count()) << "\n";
    cout << "Winograd's Variant Time: " << formatTime(time3.count()) << "\n";
    cout << "</pre>";

    // Hidden JSON block
    cout << "<div id='chart-data' style='display:none;' ";
    cout << "data-size='" << n << "' ";
    cout << "data-standard='" << time1.count() << "' ";
    cout << "data-strassen='" << time2.count() << "' ";
    cout << "data-winograd='" << time3.count() << "'></div>";

    return 0;
}
