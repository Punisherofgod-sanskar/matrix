// ✅ Final updated pathfinder.cpp (fully correct for multiple runs)
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <chrono>
#include <cstdlib>
#include <algorithm>

using namespace std;
using namespace chrono;

struct Node {
    int x, y, g, h;
    Node* parent;
    int f() const { return g + h; }
};

struct CompareAStar {
    bool operator()(Node* a, Node* b) { return a->f() > b->f(); }
};
struct CompareDijkstra {
    bool operator()(Node* a, Node* b) { return a->g > b->g; }
};

int heuristic(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

bool isValid(int x, int y, int n) {
    return x >= 0 && y >= 0 && x < n && y < n;
}

pair<vector<int>, long long> runPathfinder(string algo, const vector<string>& grid, int sx, int sy, int ex, int ey) {
    const int N = 10;
    vector<vector<bool>> closed(N, vector<bool>(N, false));
    vector<pair<int, int>> dirs = {{1,0},{-1,0},{0,1},{0,-1}};
    Node* endNode = nullptr;

    auto startTime = high_resolution_clock::now();

    if (algo == "astar") {
        priority_queue<Node*, vector<Node*>, CompareAStar> open;
        open.push(new Node{sx, sy, 0, heuristic(sx, sy, ex, ey), nullptr});

        while (!open.empty()) {
            Node* curr = open.top(); open.pop();
            if (closed[curr->x][curr->y]) continue;
            closed[curr->x][curr->y] = true;
            if (curr->x == ex && curr->y == ey) { endNode = curr; break; }

            for (auto [dx, dy] : dirs) {
                int nx = curr->x + dx, ny = curr->y + dy;
                if (!isValid(nx, ny, N) || grid[nx][ny] == 'W' || closed[nx][ny]) continue;
                open.push(new Node{nx, ny, curr->g + 1, heuristic(nx, ny, ex, ey), curr});
            }
        }
    } else if (algo == "dijkstra") {
        priority_queue<Node*, vector<Node*>, CompareDijkstra> open;
        open.push(new Node{sx, sy, 0, 0, nullptr});

        while (!open.empty()) {
            Node* curr = open.top(); open.pop();
            if (closed[curr->x][curr->y]) continue;
            closed[curr->x][curr->y] = true;
            if (curr->x == ex && curr->y == ey) { endNode = curr; break; }

            for (auto [dx, dy] : dirs) {
                int nx = curr->x + dx, ny = curr->y + dy;
                if (!isValid(nx, ny, N) || grid[nx][ny] == 'W' || closed[nx][ny]) continue;
                open.push(new Node{nx, ny, curr->g + 1, 0, curr});
            }
        }
    } else if (algo == "bfs") {
        queue<Node*> q;
        q.push(new Node{sx, sy, 0, 0, nullptr});
        closed[sx][sy] = true;

        while (!q.empty()) {
            Node* curr = q.front(); q.pop();
            if (curr->x == ex && curr->y == ey) { endNode = curr; break; }

            for (auto [dx, dy] : dirs) {
                int nx = curr->x + dx, ny = curr->y + dy;
                if (!isValid(nx, ny, N) || grid[nx][ny] == 'W' || closed[nx][ny]) continue;
                closed[nx][ny] = true;
                q.push(new Node{nx, ny, 0, 0, curr});
            }
        }
    }

    auto endTime = high_resolution_clock::now();
    long long duration = duration_cast<microseconds>(endTime - startTime).count();

    vector<int> path;
    if (endNode) {
        Node* p = endNode;
        while (p && !(p->x == sx && p->y == sy)) {
            path.push_back(p->x * N + p->y);
            p = p->parent;
        }
        reverse(path.begin(), path.end());
    }

    return {path, duration};
}

int main() {
    cout << "Content-Type: text/html\n\n";
    cout << "<div style='display: flex; gap: 2rem; color: #0f0; font-family: monospace;'>";

    string content;
    char* lenStr = getenv("CONTENT_LENGTH");
    int len = lenStr ? stoi(lenStr) : 0;
    content.resize(len);
    cin.read(&content[0], len);

    string gridData;
    size_t gpos = content.find("gridData=");
    if (gpos != string::npos) gridData = content.substr(gpos + 9);

    if (gridData.length() != 100) {
        cout << "Invalid grid data.</div>";
        return 0;
    }

    const int N = 10;
    vector<string> grid(N, string(N, '.'));
    int sx = -1, sy = -1, ex = -1, ey = -1;

    for (int i = 0; i < 100; ++i) {
        int x = i / N, y = i % N;
        char c = gridData[i];
        grid[x][y] = c;
        if (c == 'S') { sx = x; sy = y; }
        if (c == 'E') { ex = x; ey = y; }
    }

    if (sx == -1 || ex == -1) {
        cout << "Start or End not set.</div>";
        return 0;
    }

    vector<string> algos = {"astar", "bfs", "dijkstra"};
    for (const string& algo : algos) {
        auto [path, time] = runPathfinder(algo, grid, sx, sy, ex, ey);
        cout << "<div><h3>" << algo << "</h3>";
        cout << "<div id='path-" << algo << "' style='display:none;'>";
        for (int id : path) cout << id << ",";
        cout << "</div>";
        cout << "<p><strong>Path length:</strong> " << path.size() << "</p>";
        cout << "<p><strong>Time taken:</strong> " << time << " µs</p></div>";
    }

    cout << "</div>";
    return 0;
}
