#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <unordered_map>
#include <cstdlib>

using namespace std;

struct Node {
    int x, y, g, h;
    Node* parent;
    int f() const { return g + h; }
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->f() > b->f();
    }
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

    string gridData;
    size_t pos = content.find("gridData=");
    if (pos != string::npos)
        gridData = content.substr(pos + 9);

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

    priority_queue<Node*, vector<Node*>, Compare> open;
    vector<vector<bool>> closed(N, vector<bool>(N, false));
    Node* start = new Node{sx, sy, 0, heuristic(sx, sy, ex, ey), nullptr};
    open.push(start);

    Node* endNode = nullptr;
    vector<pair<int, int>> dirs = {{1,0},{-1,0},{0,1},{0,-1}};

    while (!open.empty()) {
        Node* current = open.top(); open.pop();
        if (closed[current->x][current->y]) continue;
        closed[current->x][current->y] = true;

        if (current->x == ex && current->y == ey) {
            endNode = current;
            break;
        }

        for (auto [dx, dy] : dirs) {
            int nx = current->x + dx;
            int ny = current->y + dy;
            if (!isValid(nx, ny, N) || grid[nx][ny] == 'W' || closed[nx][ny]) continue;

            Node* neighbor = new Node{nx, ny, current->g + 1, heuristic(nx, ny, ex, ey), current};
            open.push(neighbor);
        }
    }

    // Print base grid
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

    // Output path as coordinates in hidden div
    cout << "<div id='path-output' style='display:none;'>";

    if (endNode) {
        Node* path = endNode->parent;
        while (path && !(path->x == sx && path->y == sy)) {
            cout << (path->x * N + path->y) << ",";
            path = path->parent;
        }
    }

    cout << "</div>";
    return 0;
}
