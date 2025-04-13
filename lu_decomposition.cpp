#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>

using namespace std;

// Parses matrix from "1,2,3;4,5,6;7,8,9" into 2D vector
vector<vector<double>> parseMatrix(const string& input) {
    vector<vector<double>> matrix;
    stringstream ss(input);
    string row;
    while (getline(ss, row, ';')) {
        stringstream rs(row);
        string num;
        vector<double> r;
        while (getline(rs, num, ',')) {
            if (!num.empty()) r.push_back(stod(num));
        }
        if (!r.empty()) matrix.push_back(r);
    }
    return matrix;
}

// LU Decomposition without pivoting
void luDecompose(const vector<vector<double>>& A, vector<vector<double>>& L, vector<vector<double>>& U) {
    int n = A.size();
    L.assign(n, vector<double>(n, 0));
    U.assign(n, vector<double>(n, 0));

    for (int i = 0; i < n; ++i) {
        // Upper Triangular
        for (int k = i; k < n; ++k) {
            double sum = 0;
            for (int j = 0; j < i; ++j)
                sum += L[i][j] * U[j][k];
            U[i][k] = A[i][k] - sum;
        }

        // Lower Triangular
        for (int k = i; k < n; ++k) {
            if (i == k) L[i][i] = 1;
            else {
                double sum = 0;
                for (int j = 0; j < i; ++j)
                    sum += L[k][j] * U[j][i];
                L[k][i] = (A[k][i] - sum) / U[i][i];
            }
        }
    }
}

// LaTeX matrix formatting
string latex(const vector<vector<double>>& M) {
    stringstream ss;
    ss << "\\begin{bmatrix}";
    for (size_t i = 0; i < M.size(); ++i) {
        for (size_t j = 0; j < M[i].size(); ++j) {
            ss << fixed << setprecision(2) << M[i][j];
            if (j < M[i].size() - 1) ss << " & ";
        }
        if (i < M.size() - 1) ss << " \\\\ ";
    }
    ss << "\\end{bmatrix}";
    return ss.str();
}

// Extracts and decodes field from form data
string getField(const string& data, const string& field) {
    size_t pos = data.find(field + "=");
    if (pos == string::npos) return "";
    size_t end = data.find("&", pos);
    string encoded = data.substr(pos + field.size() + 1,
        (end == string::npos ? data.length() : end) - pos - field.size() - 1);

    string result;
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '+') result += ' ';
        else if (encoded[i] == '%' && i + 2 < encoded.length()) {
            int val = stoi(encoded.substr(i + 1, 2), nullptr, 16);
            result += static_cast<char>(val);
            i += 2;
        }
        else {
            result += encoded[i];
        }
    }
    return result;
}

int main() {
    cout << "Content-type: text/html\n\n";

    char* clen = getenv("CONTENT_LENGTH");
    int len = clen ? atoi(clen) : 0;
    string data(len, '\0');
    cin.read(&data[0], len);

    string matrixStr = getField(data, "matrix");

    vector<vector<double>> A = parseMatrix(matrixStr);
    if (A.empty()) {
        cout << "<p>Matrix parsing failed.</p>";
        return 1;
    }

    vector<vector<double>> L, U;
    luDecompose(A, L, U);

    cout << R"(
    <html><head>
    <script src="https://polyfill.io/v3/polyfill.min.js?features=es6"></script>
    <script id="MathJax-script" async src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js"></script>
    </head><body>
    <h2>LU Decomposition</h2>
    <p>Matrix: \( A = )" << latex(A) << R"( \)</p>
    <p>Lower \( L = )" << latex(L) << R"( \)</p>
    <p>Upper \( U = )" << latex(U) << R"( \)</p>
    </body></html>
    )";

    return 0;
}
