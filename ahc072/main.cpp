//#pragma GCC optimize("O3,unroll-loops")

#include <bits/stdc++.h>
using namespace std;

using pii = pair<int, int>;
using vpii = vector<pair<int, int>>;
using vvb = vector<vector<bool>>;

int N, K;
vvb iswall;
vector<pii> nests;                 // nests[color] = {y, x}
vector<vector<int>> nest_color;    // nest_color[y][x] = color (無ければ-1)
vector<vector<vector<int>>> board; // board[y][x] : マス(y,x)のスライムスタック

int dy[] = {-1, 1, 0, 0};
int dx[] = {0, 0, -1, 1};
char dir_char[] = {'U', 'D', 'L', 'R'};

int get_dy(char dir) {
    if (dir == 'U') return -1;
    if (dir == 'D') return 1;
    return 0;
}

int get_dx(char dir) {
    if (dir == 'L') return -1;
    if (dir == 'R') return 1;
    return 0;
}

// 最短経路を取得するBFS
string getpath(int sy, int sx, int ty, int tx) {
    if (sy == ty && sx == tx) return "";

    vector<vector<pii>> parent(N, vpii(N, {-1, -1}));
    vvb seen(N, vector<bool>(N, false));
    queue<pii> q;

    q.push({sy, sx});
    seen[sy][sx] = true;

    while (!q.empty()) {
        auto [cy, cx] = q.front();
        q.pop();

        if (cy == ty && cx == tx) break;

        for (int d = 0; d < 4; d++) {
            int ny = cy + dy[d];
            int nx = cx + dx[d];

            if (ny < 0 || N <= ny || nx < 0 || N <= nx || seen[ny][nx]) continue;
            if (iswall[ny][nx]) continue;

            parent[ny][nx] = {cy, cx};
            seen[ny][nx] = true;
            q.push({ny, nx});
        }
    }

    if (!seen[ty][tx]) return "";

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

// 巣の帰巣チェック
void check_nest(int y, int x) {
    int target_color = nest_color[y][x];
    if (target_color == -1) return;

    while (!board[y][x].empty() && board[y][x].back() == target_color) {
        board[y][x].pop_back();
    }
}

// 指定色のトップ連続数を取得
int get_color_top_count(int y, int x, int color) {
    int cnt = 0;
    for (int i = (int)board[y][x].size() - 1; i >= 0; i--) {
        if (board[y][x][i] == color) cnt++;
        else break;
    }
    return cnt;
}

// 現在の盤面の中で最も巣に近い「純粋な自色スライムの位置」をハブとして決定
pii get_hub_position(int color) {
    auto [ny, nx] = nests[color];
    int min_dist = 1e9;
    pii best_hub = {ny, nx};

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (board[i][j].empty()) continue;

            bool has_self = false;
            bool has_other = false;
            for (int clr : board[i][j]) {
                if (clr == color) has_self = true;
                else has_other = true;
            }

            // 自色を含み、他色が一切混ざっていないマスのみ選択
            if (!has_self || has_other) continue;

            string path = getpath(i, j, ny, nx);
            if (path.empty() && (i != ny || j != nx)) continue;

            int dist = path.length();
            if (dist < min_dist) {
                min_dist = dist;
                best_hub = {i, j};
            }
        }
    }

    return best_hub;
}

// ジャンプ実行関数
void execute_jump(int cy, int cx, char dir, int l, int num_move) {
    int ny = cy + get_dy(dir) * l;
    int nx = cx + get_dx(dir) * l;

    int h = board[cy][cx].size();
    int k = h - num_move; // 下に残す匹数

    cout << cy << " " << cx << " " << k << " " << dir << " " << l << "\n";

    vector<int> moving_slimes;
    for (int i = 0; i < num_move; i++) {
        moving_slimes.push_back(board[cy][cx].back());
        board[cy][cx].pop_back();
    }

    for (int color : moving_slimes) {
        board[ny][nx].push_back(color);
    }

    check_nest(cy, cx);
    check_nest(ny, nx);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N >> K)) return 0;

    iswall.assign(N, vector<bool>(N, false));
    nests.resize(K);
    nest_color.assign(N, vector<int>(N, -1));
    board.assign(N, vector<vector<int>>(N));

    for (int i = 0; i < N; i++) {
        string s;
        cin >> s;
        for (int j = 0; j < N; j++) {
            char c = s[j];
            if (c == '#') {
                iswall[i][j] = true;
            } else if (c >= 'a' && c <= 'l') {
                int color = c - 'a';
                board[i][j].push_back(color);
            } else if (c >= 'A' && c <= 'L') {
                int color = c - 'A';
                nests[color] = {i, j};
                nest_color[i][j] = color;
            }
        }
    }

    // 初期状態の帰巣処理
    for (int c = 0; c < K; c++) {
        auto [ny, nx] = nests[c];
        check_nest(ny, nx);
    }

    // =============================================================
    // 色 c ごとに「ハブ作成（上限7）→ 他色発射 → 帰巣」のサイクル
    // =============================================================
    for (int c = 0; c < K; c++) {
        auto [ny, nx] = nests[c];

        while (true) {
            // 盤上に残っている色 c のスライム（巣以外）が存在するか判定
            int self_cnt = 0;
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    if (i == ny && j == nx) continue;
                    self_cnt += get_color_top_count(i, j, c);
                }
            }
            if (self_cnt == 0) break; // この色のスライムは全て帰巣済み

            pii hub = get_hub_position(c);
            int hy = hub.first;
            int hx = hub.second;

            // ---------------------------------------------------------
            // Step 1: ハブの高さ上限 7 で色 c を集結
            // ---------------------------------------------------------
            while (board[hy][hx].size() < 7) {
                vector<pair<int, pii>> candidates;

                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < N; j++) {
                        if (i == hy && j == hx) continue;
                        int cnt = get_color_top_count(i, j, c);
                        if (cnt > 0) {
                            string path = getpath(i, j, hy, hx);
                            if (!path.empty()) {
                                candidates.push_back({(int)path.length(), {i, j}});
                            }
                        }
                    }
                }

                if (candidates.empty()) break;

                sort(candidates.rbegin(), candidates.rend()); // 遠いスライム優先

                bool moved = false;
                for (auto& cand : candidates) {
                    int cy = cand.second.first;
                    int cx = cand.second.second;

                    if (board[cy][cx].empty() || (cy == hy && cx == hx)) continue;

                    string path = getpath(cy, cx, hy, hx);
                    if (path.empty()) continue;

                    char dir = path[0];
                    int target_y = cy + get_dy(dir);
                    int target_x = cx + get_dx(dir);

                    int M = get_color_top_count(cy, cx, c);
                    int g = board[target_y][target_x].size();

                    int max_allowed = 8 - g;
                    if (target_y == hy && target_x == hx) {
                        max_allowed = min(max_allowed, 7 - (int)board[hy][hx].size());
                    }

                    int num_move = min(M, max_allowed);

                    if (num_move > 0) {
                        execute_jump(cy, cx, dir, 1, num_move);
                        moved = true;
                        break;
                    }
                }

                if (!moved) break;
            }

            // ---------------------------------------------------------
            // Step 2: カタパルトジャンプ（安全な発射対象・着地先のみ）
            // ---------------------------------------------------------
            while (board[hy][hx].size() >= 2) {
                bool jump_executed = false;

                int best_cost_diff = 0;
                pii best_slime = {-1, -1};
                char best_dir = ' ';
                int best_l = -1;

                for (int cy = max(0, hy - 3); cy <= min(N - 1, hy + 3); cy++) {
                    for (int cx = max(0, hx - 3); cx <= min(N - 1, hx + 3); cx++) {
                        if (cy == hy && cx == hx) continue;
                        if (board[cy][cx].empty()) continue;

                        int other_color = board[cy][cx].back();
                        
                        // 【変更点1】まだ処理していない未来の色 (other_color > c) のみカタパルト対象にする
                        if (other_color <= c) continue;

                        pii target_hub = get_hub_position(other_color);
                        string path_direct = getpath(cy, cx, target_hub.first, target_hub.second);
                        if (path_direct.empty()) continue;
                        int direct_cost = path_direct.length();

                        string path_to_hub = getpath(cy, cx, hy, hx);
                        if (path_to_hub.empty()) continue;
                        int dist_to_hub = path_to_hub.length();
                        if (dist_to_hub > 3) continue;

                        int h_catapult = board[hy][hx].size();
                        int max_l = h_catapult;

                        for (int d = 0; d < 4; d++) {
                            char dir = dir_char[d];

                            for (int l = 1; l <= max_l; l++) {
                                int jy = hy + get_dy(dir) * l;
                                int jx = hx + get_dx(dir) * l;

                                if (jy < 0 || N <= jy || jx < 0 || N <= jx || iswall[jy][jx]) break;
                                if (l < 2) continue;

                                // 【変更点2】飛び先は「完全な空マス」または「着地スライム自身のターゲットハブ」のみ許可
                                bool is_empty = board[jy][jx].empty();
                                bool is_target_hub = (jy == target_hub.first && jx == target_hub.second);
                                if (!is_empty && !is_target_hub) continue;

                                string path_rem = getpath(jy, jx, target_hub.first, target_hub.second);
                                int rem_cost = path_rem.length();

                                int total_via_cost = dist_to_hub + 1 + rem_cost;
                                int diff = direct_cost - total_via_cost;

                                if (diff > best_cost_diff) {
                                    best_cost_diff = diff;
                                    best_slime = {cy, cx};
                                    best_dir = dir;
                                    best_l = l;
                                }
                            }
                        }
                    }
                }

                if (best_slime.first != -1) {
                    int scy = best_slime.first;
                    int scx = best_slime.second;

                    string path_to_hub = getpath(scy, scx, hy, hx);
                    bool arrived = true;
                    for (char dir : path_to_hub) {
                        int target_y = scy + get_dy(dir);
                        int target_x = scx + get_dx(dir);
                        if (board[target_y][target_x].size() >= 8) {
                            arrived = false;
                            break;
                        }
                        execute_jump(scy, scx, dir, 1, 1);
                        scy = target_y;
                        scx = target_x;
                    }

                    if (arrived && scy == hy && scx == hx) {
                        execute_jump(hy, hx, best_dir, best_l, 1);
                        jump_executed = true;
                    }
                }

                if (!jump_executed) break;
            }

            // ---------------------------------------------------------
            // Step 3: ハブ (hy, hx) の色 c を巣へ帰巣させる
            // ---------------------------------------------------------
            while (get_color_top_count(hy, hx, c) > 0) {
                if (hy == ny && hx == nx) break;
                string path = getpath(hy, hx, ny, nx);
                if (path.empty()) break;

                int cy = hy, cx = hx;
                bool moved = false;
                for (char dir : path) {
                    int M_hub = get_color_top_count(cy, cx, c);
                    if (M_hub == 0) break;

                    int target_y = cy + get_dy(dir);
                    int target_x = cx + get_dx(dir);

                    int g = board[target_y][target_x].size();
                    int num_move = min(M_hub, 8 - g);

                    if (num_move == 0) break;

                    execute_jump(cy, cx, dir, 1, num_move);
                    cy = target_y;
                    cx = target_x;
                    moved = true;

                    if (get_color_top_count(cy, cx, c) == 0) break;
                }

                if (!moved) break;
            }

            if (get_color_top_count(hy, hx, c) > 0 && hy != ny && hx != nx) {
                break;
            }
        }
    }

    // =============================================================
    // 最終スイープ処理（保険として保持）
    // =============================================================
    while (true) {
        bool any_moved = false;

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (board[i][j].empty()) continue;

                int clr = board[i][j].back();
                auto [ny, nx] = nests[clr];

                if (i == ny && j == nx) continue;

                string path = getpath(i, j, ny, nx);
                if (path.empty()) continue;

                char dir = path[0];
                int target_y = i + get_dy(dir);
                int target_x = j + get_dx(dir);

                int M = get_color_top_count(i, j, clr);
                int g = board[target_y][target_x].size();
                int num_move = min(M, 8 - g);

                if (num_move > 0) {
                    execute_jump(i, j, dir, 1, num_move);
                    any_moved = true;
                    break;
                }
            }
            if (any_moved) break;
        }

        if (!any_moved) break;
    }

    return 0;
}