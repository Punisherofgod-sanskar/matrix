#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include <algorithm>

using namespace std;

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

int main() {
    cout << "Content-Type: text/html\n\n";
    cout << "<style>body{color:#0f0;font-family:monospace;font-size:1rem;}</style>";
    cout << "<pre>";

    string content;
    char* lenStr = getenv("CONTENT_LENGTH");
    int len = lenStr ? stoi(lenStr) : 0;
    content.resize(len);
    cin.read(&content[0], len);

    string gridData, algo = "astar";
    size_t gpos = content.find("gridData=");
    size_t apos = content.find("&algo=");
    if (gpos != string::npos)
        gridData = content.substr(gpos + 9, apos - (gpos + 9));
    if (apos != string::npos)
        algo = content.substr(apos + 6);

    if (gridData.length() != 100) {
        cout << "Invalid grid data.\n</pre>";
        return 0;
    }

    const int N = 10;
    vector<string> grid(N, string(N, '.'));
    int sx = -1, sy = -1, ex = -1, ey = -1;

    for (int i = 0; i < 100; ++i) {
        int x = i / N;
        int y = i % N;
        char c = gridData[i];
        grid[x][y] = c;
        if (c == 'S') sx = x, sy = y;
        if (c == 'E') ex = x, ey = y;
    }

    if (sx == -1 || ex == -1) {
        cout << "Start or End not set.\n</pre>";
        return 0;
    }

    using NodePtr = Node*;
    vector<pair<int, int>> dirs = {{1,0},{-1,0},{0,1},{0,-1}};
    vector<vector<bool>> closed(N, vector<bool>(N, false));
    Node* endNode = nullptr;

    if (algo == "astar") {
        priority_queue<NodePtr, vector<NodePtr>, CompareAStar> open;
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
        priority_queue<NodePtr, vector<NodePtr>, CompareDijkstra> open;
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

    // Print grid view
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            char c = grid[i][j];
            if (c == '.') cout << ". ";
            else if (c == 'W') cout << "# ";
            else cout << c << ' ';
        }
        cout << '\n';
    }
    cout << "</pre>";

    // Output path for animation
    cout << "<div id='path-output' style='display:none;'>";
    if (endNode) {
        Node* p = endNode;
        vector<int> path;
        while (p && !(p->x == sx && p->y == sy)) {
            path.push_back(p->x * N + p->y);
            p = p->parent;
        }
        reverse(path.begin(), path.end());
        for (int id : path) cout << id << ",";
    }
    cout << "</div>";

    return 0;
}
