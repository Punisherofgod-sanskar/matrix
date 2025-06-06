﻿#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <cctype>

using namespace std;

const int MOD = 26;

int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

int modInverse(int a, int mod) {
    int m0 = mod, t, q;
    int x0 = 0, x1 = 1;
    if (mod == 1) return 0;
    while (a > 1) {
        q = a / mod;
        t = mod;
        mod = a % mod, a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }
    return (x1 + m0) % m0;
}

int determinant(const vector<vector<int>>& mat, int mod) {
    int n = mat.size();
    if (n == 1) return mat[0][0] % mod;

    int det = 0;
    for (int p = 0; p < n; ++p) {
        vector<vector<int>> submat(n - 1, vector<int>(n - 1));
        for (int i = 1; i < n; ++i) {
            int colIndex = 0;
            for (int j = 0; j < n; ++j) {
                if (j == p) continue;
                submat[i - 1][colIndex++] = mat[i][j];
            }
        }
        int sign = (p % 2 == 0) ? 1 : -1;
        det = (det + mod + sign * mat[0][p] * determinant(submat, mod)) % mod;
    }
    return det;
}

vector<vector<int>> adjugate(const vector<vector<int>>& mat, int mod) {
    int n = mat.size();
    vector<vector<int>> adj(n, vector<int>(n));
    if (n == 1) {
        adj[0][0] = 1;
        return adj;
    }

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vector<vector<int>> submat(n - 1, vector<int>(n - 1));
            int r = 0;
            for (int x = 0; x < n; ++x) {
                if (x == i) continue;
                int c = 0;
                for (int y = 0; y < n; ++y) {
                    if (y == j) continue;
                    submat[r][c++] = mat[x][y];
                }
                r++;
            }
            int sign = ((i + j) % 2 == 0) ? 1 : -1;
            adj[j][i] = (mod + sign * determinant(submat, mod)) % mod;
        }
    }
    return adj;
}

vector<vector<int>> modInverseMatrix(const vector<vector<int>>& mat, int mod) {
    int det = determinant(mat, mod);
    int invDet = modInverse(det % mod, mod);
    vector<vector<int>> adj = adjugate(mat, mod);
    int n = mat.size();
    vector<vector<int>> inv(n, vector<int>(n));

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            inv[i][j] = (adj[i][j] * invDet) % mod;

    return inv;
}

vector<vector<int>> parseMatrix(const string& input) {
    vector<vector<int>> matrix;
    stringstream ss(input);
    string row;

    while (getline(ss, row, ';')) {
        vector<int> r;
        stringstream rs(row);
        string val;
        while (getline(rs, val, ',')) {
            r.push_back(stoi(val) % MOD);
        }
        matrix.push_back(r);
    }

    size_t n = matrix.size();
    for (const auto& row : matrix) {
        if (row.size() != n) {
            cerr << "Matrix is not square." << endl;
            return {};
        }
    }

    int det = determinant(matrix, MOD);
    if (gcd(det, MOD) != 1) {
        cerr << "Matrix is not invertible modulo 26." << endl;
        return {};
    }

    return matrix;
}

string cleanMessage(const string& msg) {
    string cleaned;
    for (char c : msg)
        if (isalpha(c)) cleaned += toupper(c);
    return cleaned;
}

string encrypt(const string& msg, const vector<vector<int>>& mat) {
    int n = mat.size();
    string cleaned = cleanMessage(msg);
    while (cleaned.size() % n != 0)
        cleaned += 'X';

    string cipher;
    for (size_t i = 0; i < cleaned.size(); i += n) {
        vector<int> block(n);
        for (int j = 0; j < n; ++j)
            block[j] = cleaned[i + j] - 'A';

        vector<int> result(n);
        for (int r = 0; r < n; ++r) {
            for (int c = 0; c < n; ++c)
                result[r] = (result[r] + mat[r][c] * block[c]) % MOD;
            cipher += (char)(result[r] + 'A');
        }
    }
    return cipher;
}

string decrypt(const string& msg, const vector<vector<int>>& mat) {
    int n = mat.size();
    vector<vector<int>> invMat = modInverseMatrix(mat, MOD);
    return encrypt(msg, invMat);
}

void printHtmlResult(const string& result) {
    cout << "Content-Type: text/plain\n\n";
    cout << result << endl;
}

int main() {
    string data;
    char* contentLengthStr = getenv("CONTENT_LENGTH");
    int contentLength = contentLengthStr ? stoi(contentLengthStr) : 0;
    data.resize(contentLength);
    cin.read(&data[0], contentLength);


    stringstream ss(data);
    string pair, msg, matrixStr, mode;
    while (getline(ss, pair, '&')) {
        size_t eq = pair.find('=');
        if (eq == string::npos) continue;
        string key = pair.substr(0, eq);
        string val = pair.substr(eq + 1);
        replace(val.begin(), val.end(), '+', ' ');

        if (key == "message") msg = val;
        else if (key == "matrix") matrixStr = val;
        else if (key == "mode") mode = val;
    }

    if (msg.empty() || matrixStr.empty() || mode.empty()) {
        printHtmlResult("Missing data.");
        return 1;
    }

    vector<vector<int>> matrix = parseMatrix(matrixStr);
    if (matrix.empty()) {
        printHtmlResult("Invalid or non-invertible matrix.");
        return 1;
    }

    string result = (mode == "encrypt") ? encrypt(msg, matrix) : decrypt(msg, matrix);
    printHtmlResult(result);
    return 0;
}