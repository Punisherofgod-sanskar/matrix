// File: lu_decomposition.cpp
// Purpose: Perform LU decomposition with basic input parsing and HTML output
// Notes: This version uses natural student-style comments and structure for clarity and readability

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>

using namespace std;

// Parses input like "1,2,3;4,5,6" into a 2D matrix (vector of vectors)
vector<vector<double>> parseMatrix(const string& input) {
    vector<vector<double>> matrix;
    stringstream ss(input);
    string row;

    // Split by ';' for rows
    while (getline(ss, row, ';')) {
        vector<double> r;
        stringstream rowStream(row);
        string num;

        // Split each row by ','
        while (getline(rowStream, num, ',')) {
            if (!num.empty()) r.push_back(stod(num)); // Convert string to double
        }

        if (!r.empty()) matrix.push_back(r); // Only add non-empty rows
    }
    return matrix;
}

// Basic LU decomposition without pivoting (not recommended for singular matrices)
void luDecompose(const vector<vector<double>>& A, vector<vector<double>>& L, vector<vector<double>>& U) {
    int n = A.size(); // Size of the matrix
    L.assign(n, vector<double>(n, 0)); // Initialize L with 0s
    U.assign(n, vector<double>(n, 0)); // Initialize U with 0s

    for (int i = 0; i < n; ++i) {
        // Build the upper triangular matrix U
        for (int k = i; k < n; ++k) {
            double sum = 0;
            for (int j = 0; j < i; ++j)
                sum += L[i][j] * U[j][k]; // Sum of L row * U column up to i
            U[i][k] = A[i][k] - sum; // Subtract to get upper element
        }

        // Build the lower triangular matrix L
        for (int k = i; k < n; ++k) {
            if (i == k) L[i][i] = 1; // Diagonal of L is always 1
            else {
                double sum = 0;
                for (int j = 0; j < i; ++j)
                    sum += L[k][j] * U[j][i]; // Sum of L row * U column up to i
                L[k][i] = (A[k][i] - sum) / U[i][i]; // Compute lower element
            }
        }
    }
}

// Converts matrix to LaTeX format for displaying on webpage using MathJax
string latex(const vector<vector<double>>& M) {
    stringstream ss;
    ss << "\\begin{bmatrix}";
    for (size_t i = 0; i < M.size(); ++i) {
        for (size_t j = 0; j < M[i].size(); ++j) {
            ss << fixed << setprecision(2) << M[i][j]; // Format each number to 2 decimal places
            if (j < M[i].size() - 1) ss << " & "; // Add '&' between columns
        }
        ss << " \\\\"; // New line between matrix rows (fixes visual formatting)
    }
    ss << "\\end{bmatrix}";
    return ss.str(); // Return the final LaTeX string
}

// Extracts a specific field from URL-encoded form data (e.g., matrix=...)
string getField(const string& data, const string& field) {
    size_t pos = data.find(field + "=");
    if (pos == string::npos) return "";
    size_t end = data.find("&", pos);
    string encoded = data.substr(pos + field.size() + 1,
        (end == string::npos ? data.length() : end) - pos - field.size() - 1);

    // Decode URL encoding (e.g., + -> space, %20 -> space)
    string result;
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '+') result += ' '; // Convert '+' to space
        else if (encoded[i] == '%' && i + 2 < encoded.length()) {
            int val = stoi(encoded.substr(i + 1, 2), nullptr, 16); // Convert hex to int
            result += static_cast<char>(val); // Add decoded character
            i += 2; // Skip next two characters
        } else {
            result += encoded[i]; // Add normal character
        }
    }
    return result;
}

int main() {
    // CGI header so browser treats it as HTML output
    cout << "Content-type: text/html\n\n";

    // Read input length and input data from POST request
    char* clen = getenv("CONTENT_LENGTH");
    int len = clen ? atoi(clen) : 0;
    string data(len, '\0');
    cin.read(&data[0], len); // Read the input into 'data'

    // Extract and parse matrix string from form data
    string matrixStr = getField(data, "matrix");
    vector<vector<double>> A = parseMatrix(matrixStr);

    // If matrix parsing fails, display error
    if (A.empty()) {
        cout << "<p>Matrix parsing failed.</p>";
        return 1;
    }

    // Perform LU decomposition
    vector<vector<double>> L, U;
    luDecompose(A, L, U);

    // Output results using MathJax for pretty matrix rendering
    cout << R"(
    <html><head>
    <script src="https://polyfill.io/v3/polyfill.min.js?features=es6"></script>
    <script id="MathJax-script" async src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js"></script>
    </head><body>
    <h2>LU Decomposition Result</h2>
    <p>Original Matrix: \( A = )" << latex(A) << R"( \)</p>
    <p>Lower Matrix: \( L = )" << latex(L) << R"( \)</p>
    <p>Upper Matrix: \( U = )" << latex(U) << R"( \)</p>
    </body></html>
    )";

    return 0;
}
