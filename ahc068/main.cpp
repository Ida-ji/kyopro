#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vb = vector<bool>;
using vvi = vector<vector<int>>;
using vvb = vector<vector<bool>>;

int N;
vvi vwall; 
vvi hwall; 
vvi a;
vector<pair<int, int>> pos; 

vvb cantmove; 

int dy[] = {-1, 0, 0, 1};
int dx[] = {0, -1, 1, 0};
char dir_char[] = {'U', 'L', 'R', 'D'};

vvi dijkstra(int sx, int sy, int gx, int gy) {
    vvi dist(N, vi(N, 1e9)); 
    vvi parent(N, vi(N, -1)); 
    
    priority_queue<pair<int, pair<int, int>>, 
                   vector<pair<int, pair<int, int>>>, 
                   greater<pair<int, pair<int, int>>>> pq;
    
    dist[sy][sx] = 0;
    pq.push({0, {sy, sx}});

    while (!pq.empty()) {
        auto [d, p] = pq.top();
        auto [y, x] = p;
        pq.pop();

        if (d > dist[y][x]) continue;
        if (y == gy && x == gx) break;

        for (int dir = 0; dir < 4; dir++) {
            int ny = y + dy[dir];
            int nx = x + dx[dir];

            if (ny < 0 || ny >= N || nx < 0 || nx >= N) continue;
            
            if (dir == 0 && hwall[ny][nx] == '1') continue;
            if (dir == 1 && vwall[ny][nx] == '1') continue;
            if (dir == 2 && vwall[y][x] == '1') continue;
            if (dir == 3 && hwall[y][x] == '1') continue;

            // 変更後
            int cost = 1;
            if (cantmove[ny][nx]) {
                cost = 10000;
            } else {
                int current_card = a[ny][nx];
                int target_y = current_card / N;
                int target_x = current_card % N;
    
                // そのマスにいるカードが、すでに本来の目的地にいる場合
                if (ny == target_y && nx == target_x) {
                    cost = 30; // 壊したくないので迂回コストを課す
                } else {
                    // [オプション] 目的地にどれくらい近いかに応じてマイルドにコストを乗せるのも有効
                    // 本来の位置に近いカードほど、少しだけ避けるようにする
                    int dist_to_target = abs(ny - target_y) + abs(nx - target_x);
                    if (dist_to_target <= 1) {
                        cost = 5; 
                    }
                }
            }

            if (dist[y][x] + cost < dist[ny][nx]) {
                dist[ny][nx] = dist[y][x] + cost;
                parent[ny][nx] = dir;
                pq.push({dist[ny][nx], {ny, nx}});
            }
        }
    }
    return parent;
}

string get_path(vvi &parent, int sx, int sy, int gx, int gy) {
    string p = "";
    int cx = gx, cy = gy;
    while (cx != sx || cy != sy) {
        int dir = parent[cy][cx];
        if (dir == -1) break;
        p += dir_char[dir];
        cy -= dy[dir];
        cx -= dx[dir];
    }
    reverse(p.begin(), p.end());
    return p;
}

// 評価関数（マンハッタン距離の総和）
int scorecalc(const vector<pair<int, int>>& current_pos) {
    int ret = 0; 
    for (int i = 0; i < N * N; i++) {
        int dy = abs(current_pos[i].first - i / N);
        int dx = abs(current_pos[i].second - i % N);
        // 縦方向のズレを10倍のペナルティとして評価
        ret += dy * 10 + dx; 
    }
    return ret;
}

struct ans_struct {
    char d; int r; int c; int h; int w;
};

int main() {
    if (!(cin >> N)) return 0;
    
    a.resize(N, vi(N, 0));
    pos.resize(N * N);
    cantmove.resize(N, vb(N, false));
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int n; cin >> n;
            a[i][j] = n;
            pos[n] = {i, j};
        }    
    }

    vwall.resize(N, vi(N - 1));
    hwall.resize(N - 1, vi(N));
    for (int i = 0; i < N; i++) {
        string s; cin >> s;
        for (int j = 0; j < N - 1; j++) vwall[i][j] = s[j];
    }
    for (int i = 0; i < N - 1; i++) {
        string s; cin >> s;
        for (int j = 0; j < N; j++) hwall[i][j] = s[j];
    }

    vector<ans_struct> ans;

    // 乱数エンジン
    random_device rd;
    mt19937 gen(rd());
    
    // 時間管理の準備
    auto start = chrono::steady_clock::now();
    const double TIME_LIMIT = 1800.0; // 制限時間 (ms)
    
    // 温度パラメータの設定（マンハッタン距離の増減幅に合わせて調整）
    const double START_TEMP = 300.0; // 30.0 から 300.0 へ引き上げ
    const double END_TEMP = 1.0;     // 0.1 から 1.0 へ引き上げ
    double temp = START_TEMP;
    
    int cur_score = scorecalc(pos);
    int loop_count = 0;

    //0.最初に1800ms、長方形swapでプチ山登りをする
    while (true) {
        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration_cast<chrono::milliseconds>(now - start).count();
        if (elapsed >= TIME_LIMIT) break;
        if (ans.size() >= 100000) break; // 操作回数の上限に達したら終了
            
        // 温度の更新（線形冷却）
        double progress = elapsed / TIME_LIMIT;
        temp = START_TEMP + (END_TEMP - START_TEMP) * progress;
        
        loop_count++;

        // 1. 長方形をランダムに決める (0: V, 1: H)
        int op_type = gen() % 2; 
        int r, c, h, w;
        int current_max_limit = 2 + (int)(12 * (1.0 - progress));
        if (op_type == 0) { // V: 高さが偶数
            // h は 2, 4, 6 のいずれか (かつ N 以下)
            int max_h = min(current_max_limit, (N / 2) * 2);
            if (max_h < 2) continue;
            h = ((gen() % (max_h / 2)) + 1) * 2;
            
            // w は 1 〜 6 (かつ N 以下)
            int max_w = min(current_max_limit, N);
            w = (gen() % max_w) + 1;
            
            r = gen() % (N - h + 1);
            c = gen() % (N - w + 1);
        } else { // H: 幅が偶数
            // h は 1 〜 6 (かつ N 以下)
            int max_h = min(current_max_limit, N);
            h = (gen() % max_h) + 1;
            
            // w は 2, 4, 6 のいずれか (かつ N 以下)
            int max_w = min(current_max_limit, (N / 2) * 2);
            if (max_w < 2) continue;
            w = ((gen() % (max_w / 2)) + 1) * 2;
            
            r = gen() % (N - h + 1);
            c = gen() % (N - w + 1);
        }

        // 長方形の内部に壁が含まれていないかチェック
        bool has_wall = false;
        for (int i = r; i < r + h; i++) {
            for (int j = c; j < c + w; j++) {
                if (j < c + w - 1 && vwall[i][j] == '1') has_wall = true;
                if (i < r + h - 1 && hwall[i][j] == '1') has_wall = true;
            }
        }
        if (has_wall) continue;

        // 2. 入れ替えるマスのペアをリストアップ
        vector<pair<pair<int, int>, pair<int, int>>> swaps;
        if (op_type == 0) {
            for (int x = 0; x < h / 2; x++) {
                for (int y = 0; y < w; y++) {
                    swaps.push_back({{r + x, c + y}, {r + h / 2 + x, c + y}});
                }
            }
        } else {
            for (int x = 0; x < h; x++) {
                for (int y = 0; y < w / 2; y++) {
                    swaps.push_back({{r + x, c + y}, {r + x, c + w / 2 + y}});
                }
            }
        }

        // 実際にスワップを適用しながら差分スコアを計算
        int diff = 0;
        for (auto& p : swaps) {
            int y1 = p.first.first, x1 = p.first.second;
            int y2 = p.second.first, x2 = p.second.second;
            int card1 = a[y1][x1];
            int card2 = a[y2][x2];
            
            // 変更前（orig）の重み付き距離
            int orig_dy1 = abs(y1 - card1 / N);
            int orig_dx1 = abs(x1 - card1 % N);
            int orig_dist1 = orig_dy1 * 10 + orig_dx1;
            
            int orig_dy2 = abs(y2 - card2 / N);
            int orig_dx2 = abs(x2 - card2 % N);
            int orig_dist2 = orig_dy2 * 10 + orig_dx2;
            
            swap(a[y1][x1], a[y2][x2]);
            pos[card1] = {y2, x2};
            pos[card2] = {y1, x1};
            
            // 変更後（new）の重み付き距離
            int new_dy1 = abs(y2 - card1 / N);
            int new_dx1 = abs(x2 - card1 % N);
            int new_dist1 = new_dy1 * 10 + new_dx1;
            
            int new_dy2 = abs(y1 - card2 / N);
            int new_dx2 = abs(x1 - card2 % N);
            int new_dist2 = new_dy2 * 10 + new_dx2;
            
            diff += (new_dist1 + new_dist2) - (orig_dist1 + orig_dist2);
        }

        int next_score = cur_score + diff;
        
        // 3. 受容判定
        bool accept = false;
        if (next_score < cur_score) {
            accept = true;
        } 

        // 4. 状態の確定 or 差し戻し
        if (accept) {
            cur_score = next_score;
            ans.push_back({op_type == 0 ? 'V' : 'H', r, c, h, w});
            if (cur_score == 0) break; // 完璧に揃ったら終了
        } else {
            // 差し戻し（逆順で同じスワップを適用して元に戻す）
            for (auto it = swaps.rbegin(); it != swaps.rend(); ++it) {
                int y1 = it->first.first, x1 = it->first.second;
                int y2 = it->second.first, x2 = it->second.second;
                int card1 = a[y1][x1];
                int card2 = a[y2][x2];
                swap(a[y1][x1], a[y2][x2]);
                pos[card1] = {y2, x2};
                pos[card2] = {y1, x1};
            }
        }
    }

    // 1. 壁の配置から「剥ぎ取り順序（order）」を計算する
    vector<pair<int, int>> order;
    vvi remaining_deg(N, vi(N, 0));
    vvb removed(N, vb(N, false));

    for (int y = 0; y < N; y++) {
        for (int x = 0; x < N; x++) {
            int deg = 0;
            for (int dir = 0; dir < 4; dir++) {
                int ny = y + dy[dir];
                int nx = x + dx[dir];
                if (ny < 0 || ny >= N || nx < 0 || nx >= N) continue;
                if (dir == 0 && hwall[ny][nx] == '1') continue;
                if (dir == 1 && vwall[ny][nx] == '1') continue;
                if (dir == 2 && vwall[y][x] == '1') continue;
                if (dir == 3 && hwall[y][x] == '1') continue;
                deg++;
            }
            remaining_deg[y][x] = deg;
        }
    }

    for (int iter = 0; iter < N * N; iter++) {
        int min_deg = 1e9;
        int buy = -1, bux = -1;
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < N; x++) {
                if (removed[y][x]) continue;
                if (remaining_deg[y][x] < min_deg) {
                    min_deg = remaining_deg[y][x];
                    buy = y; bux = x;
                }
            }
        }
        
        order.push_back({buy, bux});
        removed[buy][bux] = true;
        
        for (int dir = 0; dir < 4; dir++) {
            int ny = buy + dy[dir];
            int nx = bux + dx[dir];
            if (ny < 0 || ny >= N || nx < 0 || nx >= N) continue;
            if (dir == 0 && hwall[ny][nx] == '1') continue;
            if (dir == 1 && vwall[ny][nx] == '1') continue;
            if (dir == 2 && vwall[buy][bux] == '1') continue;
            if (dir == 3 && hwall[buy][bux] == '1') continue;
            if (!removed[ny][nx]) {
                remaining_deg[ny][nx]--;
            }
        }
    }

    // 2. 決定したorderの順（奥から手前）に1マスずつ確定させていく
    for (auto [tr, tc] : order) {
        int target_card = tr * N + tc;
        if (a[tr][tc] == target_card) {
            cantmove[tr][tc] = true;
            continue;
        }

        int cy = pos[target_card].first;
        int cx = pos[target_card].second;

        vvi cur_parent = dijkstra(cx, cy, tc, tr);
        string cur_string = get_path(cur_parent, cx, cy, tc, tr);

        for (char s : cur_string) {
            int ny = cy, nx = cx;
            if (s == 'U') { ans.push_back({'V', cy - 1, cx, 2, 1}); ny = cy - 1; }
            else if (s == 'L') { ans.push_back({'H', cy, cx - 1, 1, 2}); nx = cx - 1; }
            else if (s == 'R') { ans.push_back({'H', cy, cx, 1, 2}); nx = nx + 1; }
            else if (s == 'D') { ans.push_back({'V', cy, cx, 2, 1}); ny = cy + 1; }

            int card1 = a[cy][cx];
            int card2 = a[ny][nx];
            swap(a[cy][cx], a[ny][nx]);
            swap(pos[card1], pos[card2]);

            cantmove[cy][cx] = false;
            cantmove[ny][nx] = false;

            cy = ny;
            cx = nx;
        }
        
        cantmove[tr][tc] = true;
    }

    // 3. 最後に全体を数回だけ一括修復ループにかける（保険）
    int retry_count = 0;
    while (retry_count < 3) {
        bool any_bad = false;
        for (auto [tr, tc] : order) {
            int target_card = tr * N + tc;
            if (a[tr][tc] == target_card) continue;

            any_bad = true;
            int cy = pos[target_card].first;
            int cx = pos[target_card].second;

            vvi cur_parent = dijkstra(cx, cy, tc, tr);
            string cur_string = get_path(cur_parent, cx, cy, tc, tr);

            for (char s : cur_string) {
                int ny = cy, nx = cx;
                if (s == 'U') { ans.push_back({'V', cy - 1, cx, 2, 1}); ny = cy - 1; }
                else if (s == 'L') { ans.push_back({'H', cy, cx - 1, 1, 2}); nx = cx - 1; }
                else if (s == 'R') { ans.push_back({'H', cy, cx, 1, 2}); nx = nx + 1; }
                else if (s == 'D') { ans.push_back({'V', cy, cx, 2, 1}); ny = cy + 1; }

                int card1 = a[cy][cx];
                int card2 = a[ny][nx];
                swap(a[cy][cx], a[ny][nx]);
                swap(pos[card1], pos[card2]);

                cantmove[cy][cx] = false;
                cantmove[ny][nx] = false;

                cy = ny;
                cx = nx;
            }
            cantmove[tr][tc] = true;
        }
        if (!any_bad) break;
        retry_count++;
    }

    for (auto [d, r, c, h, w] : ans) {
        cout << d << " " << r << " " << c << " " << h << " " << w << "\n";
    }
    return 0;
}