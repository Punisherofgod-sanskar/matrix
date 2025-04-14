// File: eigenvalue_decomposition.cpp
// Purpose: Perform power iteration to approximate the dominant eigenvalue and eigenvector
// Note: Simple power method, assumes the matrix has a unique dominant eigenvalue

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <algorithm>

using namespace std;

const int MAX_ITER = 1000;     // Maximum number of iterations for power method
const double EPSILON = 1e-9;   // Tolerance for convergence

using Matrix = vector<vector<double>>;  // Matrix type alias
using Vector = vector<double>;          // Vector type alias

// Parses a string input into a matrix format, rows separated by ';' and values by ','
Matrix parseMatrix(const string& input) {
    Matrix mat;
    stringstream ss(input);
    string row;
    while (getline(ss, row, ';')) {
        stringstream rs(row);
        string val;
        Vector r;
        while (getline(rs, val, ',')) {
            if (!val.empty()) r.push_back(stod(val));  // Convert to double
        }
        if (!r.empty()) mat.push_back(r);
    }
    return mat;
}

// Returns the size of a square matrix (number of rows)
int size(const Matrix& A) {
    return A.size();
}

// Multiplies matrix A by vector x and returns resulting vector
Vector matVecMul(const Matrix& A, const Vector& x) {
    int n = size(A);
    Vector result(n, 0);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            result[i] += A[i][j] * x[j];
    return result;
}

// Computes dot product of two vectors
double dot(const Vector& a, const Vector& b) {
    double sum = 0;
    for (size_t i = 0; i < a.size(); ++i) sum += a[i] * b[i];
    return sum;
}

// Returns the Euclidean norm (length) of a vector
double norm(const Vector& v) {
    return sqrt(dot(v, v));
}

// Normalizes a vector to have unit length
Vector normalize(const Vector& v) {
    double n = norm(v);
    if (n < EPSILON) return v;  // Avoid division by near-zero
    Vector result = v;
    for (double& x : result) x /= n;
    return result;
}

// Orthogonalizes vector v against previously found eigenvectors (Gram-Schmidt)
Vector orthogonalize(const Vector& v, const vector<Vector>& prev) {
    Vector result = v;
    for (const Vector& u : prev) {
        double proj = dot(result, u);  // Project result onto u
        for (size_t i = 0; i < result.size(); ++i)
            result[i] -= proj * u[i];  // Subtract projection
    }
    return result;
}

// Computes outer product of two vectors (v1 * v2^T)
Matrix outer(const Vector& v1, const Vector& v2) {
    int n = v1.size();
    Matrix result(n, Vector(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            result[i][j] = v1[i] * v2[j];
    return result;
}

// Subtracts matrix B from matrix A (A - B)
Matrix subtract(const Matrix& A, const Matrix& B) {
    int n = size(A);
    Matrix result(n, Vector(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            result[i][j] = A[i][j] - B[i][j];
    return result;
}

// Generates an identity matrix of size n
Matrix identity(int n) {
    Matrix I(n, Vector(n, 0));
    for (int i = 0; i < n; ++i) I[i][i] = 1;
    return I;
}

// Computes inverse of matrix A using Gaussian elimination
Matrix inverse(const Matrix& A) {
    int n = size(A);
    Matrix aug = A;           // Copy of A
    Matrix I = identity(n);   // Identity matrix

    for (int i = 0; i < n; ++i) {
        double pivot = aug[i][i];
        if (abs(pivot) < EPSILON) continue;  // Skip near-zero pivot
        for (int j = 0; j < n; ++j) {
            aug[i][j] /= pivot;
            I[i][j] /= pivot;
        }
        for (int k = 0; k < n; ++k) {
            if (k == i) continue;
            double factor = aug[k][i];
            for (int j = 0; j < n; ++j) {
                aug[k][j] -= factor * aug[i][j];
                I[k][j] -= factor * I[i][j];
            }
        }
    }
    return I;
}

// Multiplies two matrices A and B (A * B)
Matrix multiply(const Matrix& A, const Matrix& B) {
    int n = size(A);
    Matrix result(n, Vector(n, 0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < n; ++k)
                result[i][j] += A[i][k] * B[k][j];
    return result;
}

// Converts a matrix to LaTeX format for display
string toLatex(const Matrix& M) {
    stringstream ss;
    ss << "\\begin{bmatrix}";
    for (int i = 0; i < size(M); i++) {
        for (int j = 0; j < size(M); j++) {
            ss << fixed << setprecision(3) << M[i][j];
            if (j < size(M) - 1) ss << " & ";
        }
        if (i < size(M) - 1) ss << " \\\\ ";
    }
    ss << "\\end{bmatrix}";
    return ss.str();
}

// Power iteration method to approximate dominant eigenvalue and eigenvector
pair<double, Vector> powerIteration(const Matrix& A) {
    int n = size(A);
    Vector b(n, 1);           // Start with vector of 1s
    b = normalize(b);
    double lambda_old = 0;

    for (int iter = 0; iter < MAX_ITER; ++iter) {
        Vector Ab = matVecMul(A, b);
        double lambda = dot(b, Ab);  // Rayleigh quotient approximation
        b = normalize(Ab);
        if (abs(lambda - lambda_old) < EPSILON) break;
        lambda_old = lambda;
    }
    return { lambda_old, b };
}

int main() {
    cout << "Content-type: text/html\n\n";

    // Read content length from environment variable
    char* clen = getenv("CONTENT_LENGTH");
    int len = clen ? atoi(clen) : 0;
    string data(len, '\0');
    cin.read(&data[0], len);

    // Parse matrix input from form data
    size_t pos = data.find("matrix=");
    if (pos == string::npos) {
        cout << "<p>Error: matrix not provided.</p>";
        return 1;
    }

    string encoded = data.substr(pos + 7);
    replace(encoded.begin(), encoded.end(), '+', ' ');
    string matrixStr;
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            int val = stoi(encoded.substr(i + 1, 2), nullptr, 16);
            matrixStr += static_cast<char>(val);
            i += 2;
        }
        else {
            matrixStr += encoded[i];
        }
    }

    Matrix A = parseMatrix(matrixStr);  // Convert to matrix
    int n = size(A);
    Matrix D(n, Vector(n, 0));          // Diagonal eigenvalue matrix
    Matrix P(n, Vector(n, 0));          // Eigenvector matrix
    Matrix original = A;

    vector<Vector> eigenvectors;

    // Perform deflation to find all eigenvalues and eigenvectors
    for (int i = 0; i < n; ++i) {
        auto [lambda, v] = powerIteration(A);
        Vector orth = orthogonalize(v, eigenvectors);
        Vector v_norm = normalize(orth);
        eigenvectors.push_back(v_norm);

        for (int j = 0; j < n; ++j)
            P[j][i] = v_norm[j];  // Fill eigenvector matrix

        D[i][i] = lambda;         // Fill diagonal matrix with eigenvalue

        // Deflate: A = A - λvv^T
        Matrix outerProd = outer(v_norm, v_norm);
        for (auto& row : outerProd)
            for (auto& x : row)
                x *= lambda;

        A = subtract(A, outerProd);
    }

    Matrix P_inv = inverse(P);                         // Inverse of eigenvector matrix
    Matrix A_reconstructed = multiply(multiply(P, D), P_inv);  // Reconstruct A as P D P^-1

    // Output results using MathJax for LaTeX rendering
    cout << R"(
    <html><head>
    <script src="https://polyfill.io/v3/polyfill.min.js?features=es6"></script>
    <script id="MathJax-script" async src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js"></script>
    </head><body>
    <div style='font-family: Inter, sans-serif; padding: 1rem; color: #000; background-color: #fff;'>
    <h2>Eigenvalue Decomposition</h2>
    <p>Original Matrix \( A = )" << toLatex(original) << R"( \)</p>
    <p>Eigenvector Matrix \( P = )" << toLatex(P) << R"( \)</p>
    <p>Diagonal Matrix \( D = )" << toLatex(D) << R"( \)</p>
    <p>Inverse Matrix \( P^{-1} = )" << toLatex(P_inv) << R"( \)</p>
    <p>Reconstructed Matrix \( A' = PDP^{-1} = )" << toLatex(A_reconstructed) << R"( \)</p>
    </div>
    <script>MathJax.typeset();</script>
    </body></html>
    )";

    return 0;
}
