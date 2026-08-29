#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using vi = vector<int>;
using vb = vector<bool>;
using vvi = vector<vector<int>>;
using vvb = vector<vector<bool>>;
using vpii = vector<pair<int, int>>;

#define rep(i, s, t) for (int i = s; i < t; i++)

int N;
vvb hwall;   // hwall[i][j]: (i,j) と (i+1,j) の間が通れるか (true: 通れる, false: 壁)
vvb vwall;   // vwall[i][j]: (i,j) と (i,j+1) の間が通れるか

int dy[] = {-1, 1, 0, 0};
int dx[] = {0, 0, -1, 1};

random_device rd;

// マス(sy, sx)から(ty, tx)への最短経路文字列を取得
string getpath(int sy, int sx, int ty, int tx) {
    if (sy == ty && sx == tx) return "";

    vector<vector<pair<int, int>>> parent(N, vpii(N));
    vvb seen(N, vb(N, false));
    queue<pair<int, int>> q;

    q.push({sy, sx});
    seen[sy][sx] = true;

    while (!q.empty()) {
        auto [cy, cx] = q.front();
        q.pop();

        if (cy == ty && cx == tx) break;

        rep(dir, 0, 4) {
            int ny = cy + dy[dir];
            int nx = cx + dx[dir];

            if (ny < 0 || N <= ny || nx < 0 || N <= nx || seen[ny][nx]) continue;

            // 壁チェックの修正
            if (dir == 0 && !hwall[cy - 1][cx]) continue; // 上
            if (dir == 1 && !hwall[cy][cx]) continue;     // 下
            if (dir == 2 && !vwall[cy][cx - 1]) continue; // 左
            if (dir == 3 && !vwall[cy][cx]) continue;     // 右

            parent[ny][nx] = {cy, cx};
            seen[ny][nx] = true;
            q.push({ny, nx});
        }
    }

    // 経路復元
    string ret = "";
    int cy = ty, cx = tx;
    while (cy != sy || cx != sx) { 
        auto [py, px] = parent[cy][cx];
        if (cy - py == -1 && cx == px) ret += 'U';
        else if (cy - py == 1 && cx == px) ret += 'D';
        else if (cy == py && cx - px == -1) ret += 'L';
        else if (cy == py && cx - px == 1) ret += 'R';

        cy = py;
        cx = px;
    }

    reverse(ret.begin(), ret.end());
    return ret;
}