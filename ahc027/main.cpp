//#pragma GCC optimize("O3,unroll-loops") // ※デバッグ時はコメントアウトしておく

#include <bits/stdc++.h>
using namespace std;

// ==========================================
// 型エイリアス・マクロの定義
// ==========================================
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
vvi a;       // 汚れ
vvi d;       // 汚れやすさ
vvb visited; // 訪問済みフラグ（全マス回収用）
vvi last_visited; // 最後にそのマスを訪問したステップ数

int dy[] = {-1, 1, 0, 0};
int dx[] = {0, 0, -1, 1};

// 8方向の移動オフセット（寄り道用）
int dy8[] = {-1, -1, -1,  0, 0,  1, 1, 1};
int dx8[] = {-1,  0,  1, -1, 1, -1, 0, 1};

random_device rd;

// ==========================================
// ペナルティ付きダイクストラ法による経路復元関数
// ==========================================
string getpath_dijkstra(int sy, int sx, int ty, int tx, int current_step) {
    if (sy == ty && sx == tx) return "";

    const int INF = 1e9;
    vvi dist(N, vi(N, INF));
    vector<vector<pair<int, int>>> parent(N, vpii(N));

    // {累積コスト, y, x} を保持する最小ヒープ
    using State = tuple<int, int, int>;
    priority_queue<State, vector<State>, greater<State>> pq;

    dist[sy][sx] = 0;
    pq.push({0, sy, sx});

    while (!pq.empty()) {
        auto [cost, cy, cx] = pq.top();
        pq.pop();

        if (cost > dist[cy][cx]) continue;
        if (cy == ty && cx == tx) break;

        rep(dir, 0, 4) {
            int ny = cy + dy[dir];
            int nx = cx + dx[dir];

            if (ny < 0 || N <= ny || nx < 0 || N <= nx) continue;

            // 壁チェック
            if (dir == 0 && !hwall[cy - 1][cx]) continue; // 上
            if (dir == 1 && !hwall[cy][cx]) continue;     // 下
            if (dir == 2 && !vwall[cy][cx - 1]) continue; // 左
            if (dir == 3 && !vwall[cy][cx]) continue;     // 右

            // 基本コスト = 1歩
            int edge_cost = 1;

            // 直近300ターン以内に踏んだマスを一律避けるためペナルティを加算
            int dt = current_step - last_visited[ny][nx];
            if (dt < 300) {
                edge_cost += 50; // ペナルティ量（約50歩分の迂回まで許容）
            }
            if (d[ny][nx] < 50) edge_cost += 20;

            if (dist[cy][cx] + edge_cost < dist[ny][nx]) {
                dist[ny][nx] = dist[cy][cx] + edge_cost;
                parent[ny][nx] = {cy, cx};
                pq.push({dist[ny][nx], ny, nx});
            }
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

// 経路にしたがって1ステップずつ汚れの更新と掃除を行う
void renewdirt(int &cy, int &cx, const string &path, vvi &a, vvb &visited, vvi &last_visited, int &current_step) {
    for (char nc : path) {
        current_step++;

        if (nc == 'U') cy--;
        else if (nc == 'D') cy++;
        else if (nc == 'L') cx--;
        else if (nc == 'R') cx++;

        // 1ステップ経過：全マスの汚れが増加
        rep(i, 0, N) {
            rep(j, 0, N) {
                a[i][j] += d[i][j];
            }
        }
        // 現在地を清掃・訪問フラグ設定・最終訪問ステップ更新
        a[cy][cx] = 0;
        visited[cy][cx] = true;
        last_visited[cy][cx] = current_step;
    }
}

// 一番汚れが溜まっているマスを特定する
pair<int, int> getmaxa(const vvi &a) {
    int maxa = -1;
    pair<int, int> maxa_cell = {0, 0};
    rep(i, 0, N) {
        rep(j, 0, N) {
            if (maxa < a[i][j]) {
                maxa = a[i][j];
                maxa_cell = {i, j};
            }
        }
    }
    return maxa_cell;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // 1. 入力受付
    if (!(cin >> N)) return 0;

    hwall.assign(N - 1, vb(N));
    vwall.assign(N, vb(N - 1));
    a.assign(N, vi(N, 0));
    d.assign(N, vi(N, 0));
    visited.assign(N, vb(N, false));
    last_visited.assign(N, vi(N, -1000)); // 十分昔のターン数で初期化

    rep(i, 0, N - 1) {
        string h; cin >> h;
        rep(j, 0, N) {
            hwall[i][j] = (h[j] == '0');
        }
    }
    rep(i, 0, N) {
        string v; cin >> v;
        rep(j, 0, N - 1) {
            vwall[i][j] = (v[j] == '0');
        }
    }

    int maxd = -1;
    pair<int, int> maxd_cell = {0, 0};

    rep(i, 0, N) {
        rep(j, 0, N) {
            cin >> d[i][j];
            if (maxd < d[i][j]) {
                maxd = d[i][j];
                maxd_cell = {i, j};
            }
        }
    }

    string ans = "";
    int currenty = 0, currentx = 0;
    int current_step = 0;
    visited[0][0] = true;
    last_visited[0][0] = 0;

    // 2. dが最も高いマスへ移動
    string path = getpath_dijkstra(currenty, currentx, maxd_cell.first, maxd_cell.second, current_step);
    ans += path;
    renewdirt(currenty, currentx, path, a, visited, last_visited, current_step);

    // 3. ansの長さが98000以下の間繰り返す
    while ((int)ans.size() < 98000) {

        // 4. 一番汚れが溜まっているマスを特定
        pair<int, int> cmaxa_cell = getmaxa(a);

        // ダイクストラで経路取得
        path = getpath_dijkstra(currenty, currentx, cmaxa_cell.first, cmaxa_cell.second, current_step);
        if (path.empty()) break;
        ans += path;
        renewdirt(currenty, currentx, path, a, visited, last_visited, current_step);

        // ==========================================
        // 近傍8マスの掃除（寄り道ルーチン）
        // ==========================================
        while ((int)ans.size() < 98000) {
            int best_ny = -1, best_nx = -1;
            int max_neighbor_dirt = -1;

            pair<int, int> cur_max_cell = getmaxa(a);
            int cur_max_dirt = a[cur_max_cell.first][cur_max_cell.second];

            double THRESHOLD_RATIO = 0.3;
            int threshold = (int)(cur_max_dirt * THRESHOLD_RATIO);

            rep(dir, 0, 8) {
                int ny = currenty + dy8[dir];
                int nx = currentx + dx8[dir];

                if (ny < 0 || N <= ny || nx < 0 || N <= nx) continue;

                if (a[ny][nx] >= threshold && a[ny][nx] > max_neighbor_dirt) {
                    max_neighbor_dirt = a[ny][nx];
                    best_ny = ny;
                    best_nx = nx;
                }
            }

            if (best_ny == -1 || max_neighbor_dirt <= 0) break;

            string neighbor_path = getpath_dijkstra(currenty, currentx, best_ny, best_nx, current_step);
            if (neighbor_path.empty()) break;

            ans += neighbor_path;
            renewdirt(currenty, currentx, neighbor_path, a, visited, last_visited, current_step);
        }
    }

    // 5. 未訪問マスを回収
    vi idx(N * N);
    iota(idx.begin(), idx.end(), 0);
    mt19937 rng(rd());
    shuffle(idx.begin(), idx.end(), rng);

    queue<pair<int, int>> q;
    rep(t, 0, N * N) {
        int i = idx[t] / N, j = idx[t] % N;
        if (!visited[i][j]) q.push({i, j});
    }

    while (!q.empty()) {
        auto [ny, nx] = q.front();
        q.pop();
        if (visited[ny][nx]) continue;

        path = getpath_dijkstra(currenty, currentx, ny, nx, current_step);
        ans += path;
        renewdirt(currenty, currentx, path, a, visited, last_visited, current_step);
    }

    // 6. 最後に (0, 0) に戻る
    path = getpath_dijkstra(currenty, currentx, 0, 0, current_step);
    ans += path;
    renewdirt(currenty, currentx, path, a, visited, last_visited, current_step);

    // 7. 出力
    cout << ans << "\n";
    return 0;
}