#pragma GCC optimize("O3,unroll-loops")

#include <bits/stdc++.h>
using namespace std;

using pii = pair<int, int>;

// -------------------------------------------------------------
// 1次元座標ヘルパー関数
// -------------------------------------------------------------
inline int to_node(int y, int x) { return y * 20 + x; }
inline int node_y(int p) { return p / 20; }
inline int node_x(int p) { return p % 20; }

// 移動命令保持用構造体
struct Move {
    int y, x, k;
    char dir;
    int l;
};

// -------------------------------------------------------------
// デバッグ統計用構造体
// -------------------------------------------------------------
struct Stats {
    int normal_moves = 0;
    int combo_jumps = 0;
    int taxi_jumps = 0;
    int mini_catapults = 0;
    int hub_catapults = 0;
    int sweep_moves = 0;
    int random_prep_runs = 0;

    void reset() {
        normal_moves = combo_jumps = taxi_jumps = mini_catapults = 0;
        hub_catapults = sweep_moves = random_prep_runs = 0;
    }
} current_stats;

// -------------------------------------------------------------
// 高速乱数生成器
// -------------------------------------------------------------
struct FastRNG {
    using result_type = uint64_t;
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return UINT64_MAX; }

    uint64_t state;

    FastRNG(uint64_t seed = 88172645463325252ULL) {
        set_seed(seed);
    }

    inline void set_seed(uint64_t seed) {
        state = seed ^ 0x9e3779b97f4a7c15ULL;
    }

    inline uint64_t operator()() {
        state += 0xa0761d6478bd642fULL;
        unsigned __int128 mum = (unsigned __int128)state * (state ^ 0xe7037ed1a0b428dbULL);
        return (uint64_t)mum ^ (uint64_t)(mum >> 64);
    }

    inline uint64_t rand() {
        return (*this)();
    }

    inline int rand_int(int mod) {
        if (mod <= 1) return 0;
        return (int)(((unsigned __int128)rand() * (uint64_t)mod) >> 64);
    }

    inline double rand_double() {
        return (double)rand() / (double)UINT64_MAX;
    }
} rng;

int N, K;
bool iswall[20][20];
pii nests[12];
int nest_color[20][20];
int cell_wall_cnt[20][20];

// -------------------------------------------------------------
// リスト管理機能付き FastBoard
// -------------------------------------------------------------
struct FastBoard {
    int data[400][8];
    int sz[400];

    // 最上段が色 c であるマスの一覧 (0 <= c < 12)
    int top_slimes[12][400];
    int top_slimes_cnt[12];
    int top_slime_pos[12][400];

    // スライムが存在するマスの一覧
    int active_cells[400];
    int active_cnt;
    int active_pos[400];

    inline void init() {
        memset(sz, 0, sizeof(sz));
        memset(top_slimes_cnt, 0, sizeof(top_slimes_cnt));
        memset(top_slime_pos, -1, sizeof(top_slime_pos));
        active_cnt = 0;
        memset(active_pos, -1, sizeof(active_pos));
    }

    inline bool empty(int p) const { return sz[p] == 0; }
    inline bool empty(int y, int x) const { return sz[to_node(y, x)] == 0; }

    inline int size(int p) const { return sz[p]; }
    inline int size(int y, int x) const { return sz[to_node(y, x)]; }

    inline int back(int p) const { return data[p][sz[p] - 1]; }
    inline int back(int y, int x) const { return data[to_node(y, x)][sz[to_node(y, x)] - 1]; }

    inline void add_top_slime(int c, int p) {
        top_slime_pos[c][p] = top_slimes_cnt[c];
        top_slimes[c][top_slimes_cnt[c]++] = p;
    }

    inline void remove_top_slime(int c, int p) {
        int idx = top_slime_pos[c][p];
        if (idx == -1) return;
        int last_p = top_slimes[c][--top_slimes_cnt[c]];
        top_slimes[c][idx] = last_p;
        top_slime_pos[c][last_p] = idx;
        top_slime_pos[c][p] = -1;
    }

    inline void add_active(int p) {
        if (active_pos[p] != -1) return;
        active_pos[p] = active_cnt;
        active_cells[active_cnt++] = p;
    }

    inline void remove_active(int p) {
        int idx = active_pos[p];
        if (idx == -1) return;
        int last_p = active_cells[--active_cnt];
        active_cells[idx] = last_p;
        active_pos[last_p] = idx;
        active_pos[p] = -1;
    }

    inline void push_back(int p, int val) {
        if (sz[p] > 0) {
            remove_top_slime(back(p), p);
        } else {
            add_active(p);
        }
        data[p][sz[p]++] = val;
        add_top_slime(val, p);
    }

    inline void push_back(int y, int x, int val) {
        push_back(to_node(y, x), val);
    }

    inline void pop_back(int p) {
        if (sz[p] == 0) return;
        remove_top_slime(back(p), p);
        sz[p]--;
        if (sz[p] > 0) {
            add_top_slime(back(p), p);
        } else {
            remove_active(p);
        }
    }

    inline void pop_back(int y, int x) {
        pop_back(to_node(y, x));
    }
};

FastBoard initial_board;

int dy[] = {-1, 1, 0, 0};
int dx[] = {0, 0, -1, 1};
char dir_char[] = {'U', 'D', 'L', 'R'};

inline int get_dy(char dir) {
    if (dir == 'U') return -1;
    if (dir == 'D') return 1;
    return 0;
}

inline int get_dx(char dir) {
    if (dir == 'L') return -1;
    if (dir == 'R') return 1;
    return 0;
}

struct Path {
    uint8_t len;
    char dirs[400];
};

// 1次元化された距離・経路メモテーブル
int dist_memo[400][400];
Path path_memo[400][400];

void precompute_paths() {
    for (int cy = 0; cy < N; cy++) {
        for (int cx = 0; cx < N; cx++) {
            int wall_cnt = 0;
            for (int d = 0; d < 4; d++) {
                int wy = cy + dy[d];
                int wx = cx + dx[d];
                if (wy < 0 || N <= wy || wx < 0 || N <= wx || iswall[wy][wx]) {
                    wall_cnt++;
                }
            }
            cell_wall_cnt[cy][cx] = wall_cnt;
        }
    }

    for (int u = 0; u < 400; u++) {
        for (int v = 0; v < 400; v++) {
            dist_memo[u][v] = 1e9;
            path_memo[u][v].len = 0;
        }
    }

    for (int sy = 0; sy < N; sy++) {
        for (int sx = 0; sx < N; sx++) {
            if (iswall[sy][sx]) continue;
            int u = to_node(sy, sx);

            static int d[20][20];
            static pii parent[20][20];
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    d[i][j] = 1e9;
                    parent[i][j] = {-1, -1};
                }
            }

            static pii q[400];
            int q_head = 0, q_tail = 0;

            d[sy][sx] = 0;
            q[q_tail++] = {sy, sx};

            while (q_head < q_tail) {
                auto [cy, cx] = q[q_head++];

                for (int dir = 0; dir < 4; dir++) {
                    int ny = cy + dy[dir];
                    int nx = cx + dx[dir];

                    if (ny < 0 || N <= ny || nx < 0 || N <= nx || iswall[ny][nx]) continue;
                    if (d[ny][nx] > d[cy][cx] + 1) {
                        d[ny][nx] = d[cy][cx] + 1;
                        parent[ny][nx] = {cy, cx};
                        q[q_tail++] = {ny, nx};
                    }
                }
            }

            for (int ty = 0; ty < N; ty++) {
                for (int tx = 0; tx < N; tx++) {
                    if (d[ty][tx] == 1e9) continue;
                    int v = to_node(ty, tx);
                    dist_memo[u][v] = d[ty][tx];

                    char temp_dirs[400];
                    int t_len = 0;
                    int cy = ty, cx = tx;
                    while (cy != sy || cx != sx) {
                        auto [py, px] = parent[cy][cx];
                        if (cy - py == -1 && cx == px) temp_dirs[t_len++] = 'U';
                        else if (cy - py == 1 && cx == px) temp_dirs[t_len++] = 'D';
                        else if (cy == py && cx - px == -1) temp_dirs[t_len++] = 'L';
                        else if (cy == py && cx - px == 1) temp_dirs[t_len++] = 'R';
                        cy = py; cx = px;
                    }

                    path_memo[u][v].len = t_len;
                    for (int k = 0; k < t_len; k++) {
                        path_memo[u][v].dirs[k] = temp_dirs[t_len - 1 - k];
                    }
                }
            }
        }
    }
}

inline const Path& getpath(int u, int v) { return path_memo[u][v]; }
inline const Path& getpath(int sy, int sx, int ty, int tx) { return path_memo[to_node(sy, sx)][to_node(ty, tx)]; }

inline int getdist(int u, int v) { return dist_memo[u][v]; }
inline int getdist(int sy, int sx, int ty, int tx) { return dist_memo[to_node(sy, sx)][to_node(ty, tx)]; }

inline void check_nest(int y, int x, FastBoard& board) {
    int target_color = nest_color[y][x];
    if (target_color == -1) return;

    int p = to_node(y, x);
    while (!board.empty(p) && board.back(p) == target_color) {
        board.pop_back(p);
    }
}

pii get_hub_position(int color, const FastBoard& board, FastRNG& hub_rng, int sim_mode) {
    auto [ny, nx] = nests[color];
    int nest_p = to_node(ny, nx);

    static pii self_slimes[400];
    int slime_cnt = 0;

    for (int idx = 0; idx < board.active_cnt; idx++) {
        int p = board.active_cells[idx];
        if (p == nest_p) continue;

        for (int k = 0; k < board.sz[p]; k++) {
            if (board.data[p][k] == color) {
                self_slimes[slime_cnt++] = {node_y(p), node_x(p)};
                break;
            }
        }
    }

    if (slime_cnt == 0) return {ny, nx};

    const int WALL_WEIGHT = 2;
    int min_score = 1e9;
    pii best_hub = {-1, -1};

    for (int pass = 0; pass < 2; pass++) {
        for (int cy = 0; cy < N; cy++) {
            for (int cx = 0; cx < N; cx++) {
                if (iswall[cy][cx]) continue;
                if (nest_color[cy][cx] != -1) continue;

                int cp = to_node(cy, cx);
                int nest_dist = dist_memo[cp][nest_p];
                if (nest_dist >= 1e8) continue;
                if (pass == 0 && nest_dist > 3) continue;

                bool has_other = false;
                for (int k = 0; k < board.sz[cp]; k++) {
                    if (board.data[cp][k] != color) {
                        has_other = true;
                        break;
                    }
                }
                if (has_other) continue;

                int wall_cnt = cell_wall_cnt[cy][cx];

                int sum_dist = 0;
                bool reachable_all = true;
                for (int idx = 0; idx < slime_cnt; idx++) {
                    auto [sy, sx] = self_slimes[idx];
                    int d = getdist(sy, sx, cy, cx);
                    if (d >= 1e8) {
                        reachable_all = false;
                        break;
                    }
                    sum_dist += d;
                }
                if (!reachable_all) continue;

                int noise = (sim_mode == 0) ? 0 : hub_rng.rand_int(100);
                int score = (sum_dist + wall_cnt * WALL_WEIGHT) * 100 + noise;

                if (score < min_score) {
                    min_score = score;
                    best_hub = {cy, cx};
                }
            }
        }

        if (best_hub.first != -1) break;
    }

    if (best_hub.first == -1) return {ny, nx};

    return best_hub;
}

// -------------------------------------------------------------
// 初期盤面のランダム整備関数 (orderを考慮した着地判定を追加)
// -------------------------------------------------------------
int apply_random_prep(FastBoard& board, FastRNG& prep_rng, const int* order, vector<Move>* prep_moves = nullptr) {
    // orderから順位テーブルを作成 (0: 最も先〜 K-1: 最も後)
    static int order_pos[12];
    for (int i = 0; i < K; i++) order_pos[order[i]] = i;

    int num_ops = 1 + prep_rng.rand_int(8);
    int steps = 0;

    for (int op = 0; op < num_ops; op++) {
        int cell_cnt = board.active_cnt;
        if (cell_cnt == 0) break;

        int p1 = board.active_cells[prep_rng.rand_int(cell_cnt)];
        int y1 = node_y(p1), x1 = node_x(p1);

        int c1 = board.back(y1, x1);

        pii hub = get_hub_position(c1, board, prep_rng, 0);
        int hy = hub.first;
        int hx = hub.second;

        int valid_dirs[4];
        int dir_cnt = 0;
        for (int d = 0; d < 4; d++) {
            int ny = y1 + dy[d];
            int nx = x1 + dx[d];
            if (ny >= 0 && ny < N && nx >= 0 && nx < N && !board.empty(ny, nx)) {
                int direct_dist = getdist(y1, x1, hy, hx);
                int next_dist = getdist(ny, nx, hy, hx);
                if (next_dist >= direct_dist) continue;
                valid_dirs[dir_cnt++] = d;
            }
        }

        if (dir_cnt == 0) continue;

        int d = valid_dirs[prep_rng.rand_int(dir_cnt)];
        int y2 = y1 + dy[d];
        int x2 = x1 + dx[d];

        int c2 = board.back(y2, x2);

        if (c1 == c2) {
            auto [ny, nx] = nests[c1];
            int d1 = getdist(y1, x1, ny, nx);
            int d2 = getdist(y2, x2, ny, nx);

            int src_y = y1, src_x = x1, dst_y = y2, dst_x = x2;
            if (d1 < d2) {
                swap(src_y, dst_y);
                swap(src_x, dst_x);
            }

            int move_cnt = board.size(src_y, src_x);
            if (board.size(dst_y, dst_x) + move_cnt <= 8) {
                char dir = ' ';
                if (dst_y - src_y == -1) dir = 'U';
                else if (dst_y - src_y == 1) dir = 'D';
                else if (dst_x - src_x == -1) dir = 'L';
                else if (dst_x - src_x == 1) dir = 'R';

                int k = 0;

                if (prep_moves) {
                    prep_moves->push_back({src_y, src_x, k, dir, 1});
                }

                static int tmp[8];
                for (int i = 0; i < move_cnt; i++) {
                    tmp[i] = board.back(src_y, src_x);
                    board.pop_back(src_y, src_x);
                }
                for (int i = move_cnt - 1; i >= 0; i--) {
                    board.push_back(dst_y, dst_x, tmp[i]);
                }
                check_nest(dst_y, dst_x, board);
                check_nest(src_y, src_x, board);
                steps++;
            }
        } else {
            pair<pii, pii> candidates[2] = { {{y1, x1}, {y2, x2}}, {{y2, x2}, {y1, x1}} };
            bool executed = false;

            for (auto& [from, via] : candidates) {
                if (executed) break;

                int fy = from.first, fx = from.second;
                int vy = via.first, vx = via.second;

                if (board.size(vy, vx) + 1 > 8) continue;

                int val = board.back(fy, fx);

                int dir1_idx = -1;
                for (int dir_i = 0; dir_i < 4; dir_i++) {
                    if (fy + dy[dir_i] == vy && fx + dx[dir_i] == vx) {
                        dir1_idx = dir_i;
                        break;
                    }
                }
                if (dir1_idx == -1) continue;
                char dir1 = dir_char[dir1_idx];

                int k1 = board.size(fy, fx) - 1;
                board.pop_back(fy, fx);
                board.push_back(vy, vx, val);

                for (int l : {2, 1}) {
                    if (executed) break;

                    for (int d2 = 0; d2 < 4; d2++) {
                        char dir2 = dir_char[d2];
                        int jy = vy + dy[d2] * l;
                        int jx = vx + dx[d2] * l;

                        if (jy < 0 || jy >= N || jx < 0 || jx >= N || iswall[jy][jx]) continue;

                        if (l == 2) {
                            int my = vy + dy[d2];
                            int mx = vx + dx[d2];
                            if (my < 0 || my >= N || mx < 0 || mx >= N || iswall[my][mx]) continue;

                            int current_hub_dist = getdist(vy, vx, hy, hx);
                            int land_hub_dist    = getdist(jy, jx, hy, hx);
                            if (land_hub_dist >= current_hub_dist) continue;
                        }

                        // 着地判定: 空マス または 着地点の最上段スライムの順位が同等以上（orderが後）なら着地可能
                        bool can_land = false;
                        if (board.empty(jy, jx)) {
                            can_land = true;
                        } else {
                            int landing_top = board.back(jy, jx);
                            if (order_pos[landing_top] >= order_pos[val]) {
                                can_land = true;
                            }
                        }

                        if (can_land && board.size(jy, jx) + 1 <= 8) {
                            int k2 = board.size(vy, vx) - 1;

                            if (prep_moves) {
                                prep_moves->push_back({fy, fx, k1, dir1, 1});
                                prep_moves->push_back({vy, vx, k2, dir2, l});
                            }

                            board.pop_back(vy, vx);
                            board.push_back(jy, jx, val);

                            check_nest(jy, jx, board);
                            check_nest(vy, vx, board);
                            check_nest(fy, fx, board);

                            steps += 2;
                            executed = true;
                            break;
                        }
                    }
                }

                if (!executed) {
                    board.pop_back(vy, vx);
                    board.push_back(fy, fx, val);
                }
            }
        }
    }
    return steps;
}

bool can_reach_hub(int scy, int scx, int hy, int hx, int moving_color, const FastBoard& board) {
    const Path& path = getpath(scy, scx, hy, hx);
    if (path.len == 0 && (scy != hy || scx != hx)) return false;

    int cy = scy, cx = scx;
    for (int idx = 0; idx < path.len; idx++) {
        char dir = path.dirs[idx];
        int ny = cy + get_dy(dir);
        int nx = cx + get_dx(dir);

        if ((ny != hy || nx != hx) && nest_color[ny][nx] == moving_color) return false;
        if (board.size(ny, nx) + 1 > 8) return false;

        cy = ny;
        cx = nx;
    }

    return true;
}

void execute_jump(int cy, int cx, char dir, int l, int num_move, bool output_flag, FastBoard& board, int& step_count) {
    step_count++;
    int ny = cy + get_dy(dir) * l;
    int nx = cx + get_dx(dir) * l;

    int cp = to_node(cy, cx);
    int np = to_node(ny, nx);

    int h = board.size(cp);
    int k = h - num_move;

    if (output_flag) {
        cout << cy << " " << cx << " " << k << " " << dir << " " << l << "\n";
    }

    static int moving_slimes[8];
    for (int i = 0; i < num_move; i++) {
        moving_slimes[i] = board.back(cp);
        board.pop_back(cp);
    }

    for (int i = 0; i < num_move; i++) {
        board.push_back(np, moving_slimes[i]);
    }

    check_nest(cy, cx, board);
    check_nest(ny, nx, board);
}

bool try_mini_catapult_jump(int cat_y, int cat_x, int current_color, int hub_y, int hub_x, bool output_flag, FastBoard& board, int& step_count) {
    int cat_p = to_node(cat_y, cat_x);
    int h = board.size(cat_p);
    if (h < 2) return false;
    if (board.back(cat_p) != current_color) return false;

    const Path& path_direct = getpath(cat_y, cat_x, hub_y, hub_x);
    if (path_direct.len == 0 && (cat_y != hub_y || cat_x != hub_x)) return false;
    int direct_cost = path_direct.len;

    if (direct_cost <= 1) return false;

    int M = 0;
    for (int k = h - 1; k >= 0; k--) {
        if (board.data[cat_p][k] == current_color) M++;
        else break;
    }
    if (M == 0 || M == h) return false;

    int k_base = h - M;
    if (k_base < 1) return false;

    int best_cost_diff = 0;
    char best_dir = ' ';
    int best_l = -1;
    int best_num_move = 0;

    int max_l = k_base + 1; 

    for (int d = 0; d < 4; d++) {
        char dir = dir_char[d];

        for (int l = 1; l <= max_l; l++) {
            bool path_ok = true;
            for (int step = 1; step < l; step++) {
                int my = cat_y + get_dy(dir) * step;
                int mx = cat_x + get_dx(dir) * step;
                if (my < 0 || N <= my || mx < 0 || N <= mx || iswall[my][mx]) {
                    path_ok = false;
                    break;
                }
            }
            if (!path_ok) break;

            int jy = cat_y + get_dy(dir) * l;
            int jx = cat_x + get_dx(dir) * l;
            if (jy < 0 || N <= jy || jx < 0 || jx >= N || iswall[jy][jx]) continue;

            int g = board.size(jy, jx);
            int max_allowed = 8 - g;
            if (jy == hub_y && jx == hub_x) {
                max_allowed = min(max_allowed, 7 - board.size(hub_y, hub_x));
            }

            int num_move = min(M, max_allowed);
            if (num_move <= 0) continue;

            const Path& path_rem = getpath(jy, jx, hub_y, hub_x);
            if (path_rem.len == 0 && (jy != hub_y || jx != hub_x)) continue;

            int rem_cost = path_rem.len;
            int total_via_cost = 1 + rem_cost;
            int diff = direct_cost - total_via_cost;

            if (diff > best_cost_diff) {
                best_cost_diff = diff;
                best_dir = dir;
                best_l = l;
                best_num_move = num_move;
            }
        }
    }

    if (best_cost_diff > 0) {
        execute_jump(cat_y, cat_x, best_dir, best_l, best_num_move, output_flag, board, step_count);
        current_stats.mini_catapults++;
        return true;
    }

    return false;
}

// 共通ポンピング関数
bool try_execute_combo_jump(int cy, int cx, int c1, int hub1_y, int hub1_x, const int* order_pos, bool output_flag, FastBoard& board, int& step_count) {
    int cp = to_node(cy, cx);
    int h = board.size(cp);
    if (h < 2) return false;
    if (board.back(cp) != c1) return false;

    int m1 = 0;
    int k = h - 1;
    while (k >= 0 && board.data[cp][k] == c1) {
        m1++;
        k--;
    }
    if (k < 0) return false;

    int c2 = board.data[cp][k];
    int m2 = 0;
    while (k >= 0 && board.data[cp][k] == c2) {
        m2++;
        k--;
    }

    auto [c2_ny, c2_nx] = nests[c2];

    int cur_y = cy, cur_x = cx;
    int pumped_steps = 0;

    while (true) {
        int best_dir_idx = -1;
        int min_c1_dist = getdist(cur_y, cur_x, hub1_y, hub1_x);
        int c2_dist_before = getdist(cur_y, cur_x, c2_ny, c2_nx);

        int moving_cnt = min(m1 + m2, board.size(cur_y, cur_x));
        if (moving_cnt < 2) break;

        for (int d = 0; d < 4; d++) {
            int ny = cur_y + dy[d];
            int nx = cur_x + dx[d];

            if (ny < 0 || N <= ny || nx < 0 || N <= nx || iswall[ny][nx]) continue;

            if (nest_color[ny][nx] != -1) {
                if (nest_color[ny][nx] != c1 && nest_color[ny][nx] != c2) continue;
            }

            int c2_dist_after = getdist(ny, nx, c2_ny, c2_nx);
            if (c2_dist_after > c2_dist_before) continue;

            int c1_dist_after = getdist(ny, nx, hub1_y, hub1_x);
            if (c1_dist_after >= min_c1_dist) continue;

            if (board.size(ny, nx) + moving_cnt > 8) continue;

            if (!board.empty(ny, nx)) {
                //次のマスの一番上
                int top_clr = board.back(ny, nx);
                if (top_clr != c1 && top_clr != c2) continue;
                
                //今のマスの一番上と色が違うならダメ
                //(色のサンドイッチが起こるから)
                int cur_top_clr = board.back(cur_y, cur_x);
                if (top_clr != cur_top_clr) continue;
            }

            best_dir_idx = d;
            min_c1_dist = c1_dist_after;
            break; 
        }

        if (best_dir_idx == -1) break;

        char dir = dir_char[best_dir_idx];
        int ny = cur_y + get_dy(dir);
        int nx = cur_x + get_dx(dir);

        execute_jump(cur_y, cur_x, dir, 1, moving_cnt, output_flag, board, step_count);
        pumped_steps++;
        cur_y = ny;
        cur_x = nx;

        m1 = 0;
        m2 = 0;

        int cur_p = to_node(cur_y, cur_x);
        int h_now = board.size(cur_p);
        if (h_now >= 1) {
            int top_c = board.back(cur_p);
            int top_m = 0;
            int idx_k = h_now - 1;

            while (idx_k >= 0 && board.data[cur_p][idx_k] == top_c) {
                top_m++;
                idx_k--;
            }

            int bot_c = (idx_k >= 0) ? board.data[cur_p][idx_k] : -1;
            int bot_m = 0;
            while (idx_k >= 0 && board.data[cur_p][idx_k] == bot_c) {
                bot_m++;
                idx_k--;
            }

            if (top_c == c1) m1 = top_m;
            else if (top_c == c2) m2 = top_m;

            if (bot_c == c1) m1 = bot_m;
            else if (bot_c == c2) m2 = bot_m;
        }

        if ((cur_y == hub1_y && cur_x == hub1_x) || (cur_y == c2_ny && cur_x == c2_nx)) {
            break;
        }
    }

    if (pumped_steps == 0) return false;

    int cur_p = to_node(cur_y, cur_x);
    int h_final = board.size(cur_p);
    if (h_final < 2) {
        current_stats.combo_jumps++;
        return true;
    }

    int top_color = board.back(cur_p);
    int fly_cnt = 0;
    int k_idx = h_final - 1;

    while (k_idx >= 0 && board.data[cur_p][k_idx] == top_color) {
        fly_cnt++;
        k_idx--;
    }

    int base_cnt = h_final - fly_cnt;
    if (base_cnt <= 0 || fly_cnt <= 0) {
        current_stats.combo_jumps++;
        return true;
    }

    int max_l = base_cnt + 1;

    int fly_target_y = (top_color == c1) ? hub1_y : nests[top_color].first;
    int fly_target_x = (top_color == c1) ? hub1_x : nests[top_color].second;
    int direct_cost = getdist(cur_y, cur_x, fly_target_y, fly_target_x);

    int best_diff = -1;
    char best_dir = ' ';
    int best_l = -1;

    for (int d = 0; d < 4; d++) {
        char dir = dir_char[d];

        for (int l = 1; l <= max_l; l++) {
            bool path_ok = true;
            for (int step = 1; step < l; step++) {
                int my = cur_y + get_dy(dir) * step;
                int mx = cur_x + get_dx(dir) * step;
                if (my < 0 || N <= my || mx < 0 || N <= mx || iswall[my][mx]) {
                    path_ok = false;
                    break;
                }
            }
            if (!path_ok) break;

            int jy = cur_y + get_dy(dir) * l;
            int jx = cur_x + get_dx(dir) * l;

            if (jy < 0 || N <= jy || jx < 0 || jx >= N || iswall[jy][jx]) continue;
            if (board.size(jy, jx) + fly_cnt > 8) continue;
            if (nest_color[jy][jx] != -1 && nest_color[jy][jx] != top_color) continue;

            int rem_cost = getdist(jy, jx, fly_target_y, fly_target_x);
            if (rem_cost >= 1e8) continue;

            if (!board.empty(jy, jx)) {
                int landing_top_color = board.back(jy, jx);
                if (order_pos[landing_top_color] < order_pos[top_color]) continue;
            }

            int diff = direct_cost - rem_cost;
            if (diff > best_diff) {
                best_diff = diff;
                best_dir = dir;
                best_l = l;
            }
        }
    }

    if (best_l != -1 && best_diff >= 0) {
        execute_jump(cur_y, cur_x, best_dir, best_l, fly_cnt, output_flag, board, step_count);
    }

    current_stats.combo_jumps++;
    return true;
}

bool try_taxi_jump(int cy, int cx, int c, int ny, int nx, const int* order_pos, bool output_flag, FastBoard& board, int& step_count) {
    int cp = to_node(cy, cx);
    if (board.empty(cp)) return false;
    if (board.back(cp) != c) return false;

    // メインマス (cy, cx) の最上段で連続している自色スライム数
    int m_c = 0;
    for (int k = board.size(cp) - 1; k >= 0; k--) {
        if (board.data[cp][k] == c) m_c++;
        else break;
    }
    if (m_c == 0) return false;

    const Path& path = getpath(cy, cx, ny, nx);
    if (path.len == 0) return false;

    char dir1 = path.dirs[0];
    int ty = cy + get_dy(dir1);
    int tx = cx + get_dx(dir1);

    if (ty < 0 || N <= ty || tx < 0 || N <= tx || iswall[ty][tx]) return false;
    if (nest_color[ty][tx] == c) return false;

    char side_dirs[2];
    if (dir1 == 'U' || dir1 == 'D') {
        side_dirs[0] = 'L'; side_dirs[1] = 'R';
    } else {
        side_dirs[0] = 'U'; side_dirs[1] = 'D';
    }

    for (int s = 0; s < 2; s++) {
        char side_dir = side_dirs[s];
        int sy = cy + get_dy(side_dir);
        int sx = cx + get_dx(side_dir);

        if (sy < 0 || N <= sy || sx < 0 || N <= sx || iswall[sy][sx]) continue;
        if (board.empty(sy, sx)) continue;

        int other_color = board.back(sy, sx);

        // 側面マス (sy, sx) からメインマス (cy, cx) への移動方向
        char dir_onto = ' ';
        if (cy - sy == -1) dir_onto = 'U';
        else if (cy - sy == 1) dir_onto = 'D';
        else if (cx - sx == -1) dir_onto = 'L';
        else if (cx - sx == 1) dir_onto = 'R';

        if (other_color == c) {
            // --------------------------------------------------
            // パターン1: 同色タクシー
            // --------------------------------------------------
            int sp = to_node(sy, sx);
            int m_c2 = 0;
            for (int k = board.size(sp) - 1; k >= 0; k--) {
                if (board.data[sp][k] == c) m_c2++;
                else break;
            }
            if (m_c2 == 0) continue;

            if (board.size(cp) + m_c2 > 8) continue;

            int total_mc = m_c + m_c2;
            int h_after = board.size(cp) + m_c2;
            int k_base = h_after - total_mc;
            if (k_base < 0) continue;

            int max_l = k_base + 1;
            int direct_cost = getdist(cy, cx, ny, nx);
            if (direct_cost >= 1e8) continue;

            int best_cost_diff = 0;
            char best_dir = ' ';
            int best_l = -1;

            for (int d = 0; d < 4; d++) {
                char dir = dir_char[d];
                for (int l = 1; l <= max_l; l++) {
                    bool path_ok = true;
                    for (int step = 1; step < l; step++) {
                        int my = cy + get_dy(dir) * step;
                        int mx = cx + get_dx(dir) * step;
                        if (my < 0 || N <= my || mx < 0 || N <= mx || iswall[my][mx]) {
                            path_ok = false;
                            break;
                        }
                    }
                    if (!path_ok) break;

                    int jy = cy + get_dy(dir) * l;
                    int jx = cx + get_dx(dir) * l;
                    if (jy < 0 || N <= jy || jx < 0 || jx >= N || iswall[jy][jx]) continue;
                    if (nest_color[jy][jx] != -1 && nest_color[jy][jx] != c) continue;

                    int g = board.size(jy, jx);
                    int max_allowed = (jy == ny && jx == nx) ? 7 : 8;
                    if (g + total_mc > max_allowed) continue;

                    if (!board.empty(jy, jx)) {
                        int landing_top = board.back(jy, jx);
                        if (landing_top != c && order_pos[landing_top] < order_pos[c]) continue;
                    }

                    int rem_cost = getdist(jy, jx, ny, nx);
                    if (rem_cost >= 1e8) continue;

                    int diff = direct_cost - rem_cost;
                    if (diff > best_cost_diff) {
                        best_cost_diff = diff;
                        best_dir = dir;
                        best_l = l;
                    }
                }
            }

            if (best_cost_diff > 0) {
                execute_jump(sy, sx, dir_onto, 1, m_c2, output_flag, board, step_count);
                execute_jump(cy, cx, best_dir, best_l, total_mc, output_flag, board, step_count);
                current_stats.taxi_jumps++;
                return true;
            }

        } else {
            // --------------------------------------------------
            // パターン2: 他色タクシー（事前チェック後に安全実行）
            // --------------------------------------------------

            // 1. 処理順のチェック（他色の処理順が自分より後であること）
            if (order_pos[other_color] <= order_pos[c]) continue;

            // 2. スタック容量と制限のチェック
            if (nest_color[cy][cx] == other_color) continue;
            if (board.size(cy, cx) + 1 > 8) continue;
            if (board.size(ty, tx) + m_c + 1 > 8) continue;

            // 3. 着地点の最上段カラーチェック
            if (!board.empty(ty, tx)) {
                int landing_top_color = board.back(ty, tx);
                if (order_pos[landing_top_color] < order_pos[other_color]) continue;
            }

            // 4. 他色スライムの巣への距離悪化チェック
            auto [other_ny, other_nx] = nests[other_color];
            int dist_before = getdist(sy, sx, other_ny, other_nx);
            int dist_after = getdist(ty, tx, other_ny, other_nx);
            if (dist_after > dist_before) continue;

            // --- 事前チェッククリア：ここから実際の移動処理 ---

            // Step A: 他色を自色の上に乗せる
            execute_jump(sy, sx, dir_onto, 1, 1, output_flag, board, step_count);

            // Step B: 1マス進んで他色を台座にする ((ty, tx) へ)
            execute_jump(cy, cx, dir1, 1, m_c + 1, output_flag, board, step_count);

            // Step C: (ty, tx) からポンピング走行を試みる
            bool pumped = try_execute_combo_jump(ty, tx, c, ny, nx, order_pos, output_flag, board, step_count);

            // もしポンピング前進ができなければ、単発の大ジャンプでフォロー
            if (!pumped) {
                const Path& path_rem = getpath(ty, tx, ny, nx);
                char dir2 = (path_rem.len > 0) ? path_rem.dirs[0] : dir1;

                int best_l = -1;
                int h_final = board.size(ty, tx); // すでに乗っているので台座含む高さ
                int fly_cnt = 0;
                int k_idx = h_final - 1;

                int cur_p = to_node(ty, tx);
                int top_color = board.back(cur_p);
                while (k_idx >= 0 && board.data[cur_p][k_idx] == top_color) {
                    fly_cnt++;
                    k_idx--;
                }
                int max_l = h_final - fly_cnt + 1;

                for (int l = max_l; l >= 1; l--) {
                    int r_jy = ty + get_dy(dir2) * l;
                    int r_jx = tx + get_dx(dir2) * l;

                    if (r_jy < 0 || N <= r_jy || r_jx < 0 || r_jx >= N || iswall[r_jy][r_jx]) continue;

                    bool path_ok = true;
                    for (int step = 1; step < l; step++) {
                        int my = ty + get_dy(dir2) * step;
                        int mx = tx + get_dx(dir2) * step;
                        if (my < 0 || N <= my || mx < 0 || N <= mx || iswall[my][mx]) {
                            path_ok = false;
                            break;
                        }
                    }
                    if (!path_ok) continue;

                    if (board.size(r_jy, r_jx) + m_c > 8) continue;

                    best_l = l;
                    break;
                }

                if (best_l != -1) {
                    execute_jump(ty, tx, dir2, best_l, m_c, output_flag, board, step_count);
                }
            }

            current_stats.taxi_jumps++;
            return true;
        }
    }    

    return false;
}

bool try_catapult_jump_at(int cat_y, int cat_x, int current_color, const int* order_pos, bool output_flag, FastBoard& board, int& step_count, int sim_mode, int min_diff) {
    int cat_p = to_node(cat_y, cat_x);
    int h_catapult = board.size(cat_p);
    if (h_catapult < 4) return false;

    for (int k = 0; k < h_catapult; k++) {
        if (board.data[cat_p][k] != current_color) return false;
    }

    int best_diff = min_diff - 1;
    pii best_slime = {-1, -1};
    char best_dir = ' ';
    int best_l = -1;
    int best_dist_to_cat = 1e9;

    pii target_hub_cache[12];
    for (int i = 0; i < K; i++) target_hub_cache[i] = {-1, -1};

    for (int cy = max(0, cat_y - 3); cy <= min(N - 1, cat_y + 3); cy++) {
        for (int cx = max(0, cat_x - 3); cx <= min(N - 1, cat_x + 3); cx++) {
            if (cy == cat_y && cx == cat_x) continue;
            if (board.empty(cy, cx)) continue;

            int other_color = board.back(cy, cx);
            if (order_pos[other_color] <= order_pos[current_color]) continue;

            if (abs(cy - cat_y) + abs(cx - cat_x) > 3) continue;

            const Path& path_to_cat = getpath(cy, cx, cat_y, cat_x);
            int dist_to_cat = path_to_cat.len;
            if (dist_to_cat < 1 || dist_to_cat > 3) continue;

            if (!can_reach_hub(cy, cx, cat_y, cat_x, other_color, board)) continue;

            pii target_hub = nests[other_color];

            int direct_cost = getdist(cy, cx, target_hub.first, target_hub.second);
            if (direct_cost >= 1e8) continue;

            for (int d = 0; d < 4; d++) {
                char dir = dir_char[d];

                for (int l = 1; l <= h_catapult; l++) {
                    int jy = cat_y + get_dy(dir) * l;
                    int jx = cat_x + get_dx(dir) * l;

                    if (jy < 0 || N <= jy || jx < 0 || jx >= N || iswall[jy][jx]) break;
                    if (l < 2) continue;
                    if (board.size(jy, jx) >= 8) continue;

                    bool is_target_hub = (jy == target_hub.first && jx == target_hub.second);
                    bool is_valid_landing = false;

                    if (board.empty(jy, jx) || is_target_hub) {
                        is_valid_landing = true;
                    } else {
                        int top_clr = board.back(jy, jx);
                        if (order_pos[top_clr] >= order_pos[other_color]) {
                            is_valid_landing = true;
                        }
                    }

                    if (!is_valid_landing) continue;

                    int rem_cost = getdist(jy, jx, target_hub.first, target_hub.second);
                    if (rem_cost >= 1e8) continue;

                    int total_via_cost = dist_to_cat + 1 + rem_cost;
                    int diff = direct_cost - total_via_cost;

                    if (diff > best_diff || (diff == best_diff && diff >= min_diff && dist_to_cat < best_dist_to_cat)) {
                        best_diff = diff;
                        best_slime = {cy, cx};
                        best_dir = dir;
                        best_l = l;
                        best_dist_to_cat = dist_to_cat;
                    }
                }
            }
        }
    }

    if (best_diff >= min_diff) {
        int scy = best_slime.first;
        int scx = best_slime.second;

        const Path& path_to_cat = getpath(scy, scx, cat_y, cat_x);
        for (int p_idx = 0; p_idx < path_to_cat.len; p_idx++) {
            if (board.empty(scy, scx)) return false;

            char dir = path_to_cat.dirs[p_idx];
            execute_jump(scy, scx, dir, 1, 1, output_flag, board, step_count);
            scy += get_dy(dir);
            scx += get_dx(dir);
        }

        if (board.empty(cat_y, cat_x)) return false;

        execute_jump(cat_y, cat_x, best_dir, best_l, 1, output_flag, board, step_count);
        current_stats.hub_catapults++;

        return true;
    }

    return false;
}

// -------------------------------------------------------------
// 盤面上のスライムを可能な限り巣へ送り切る（即時帰巣関数）
// target_color == -1 の場合は全色、指定された場合はその色のみを対象とする
// -------------------------------------------------------------
void flush_to_nests(FastBoard& board, int& step_count, bool output_flag, int max_score, int target_color = -1) {
    bool moved_any = true;
    while (moved_any) {
        moved_any = false;
        if (step_count >= max_score) return;

        for (int c_idx = 0; c_idx < K; c_idx++) {
            int clr = (target_color != -1) ? target_color : c_idx;
            auto [ny, nx] = nests[clr];
            int nest_p = to_node(ny, nx);

            for (int t_idx = 0; t_idx < board.top_slimes_cnt[clr]; t_idx++) {
                int p = board.top_slimes[clr][t_idx];
                if (p == nest_p) continue; // 既に巣にあるものはスキップ

                int cy = node_y(p);
                int cx = node_x(p);

                const Path& path = getpath(cy, cx, ny, nx);
                if (path.len == 0) continue;

                char dir = path.dirs[0];
                int target_y = cy + get_dy(dir);
                int target_x = cx + get_dx(dir);

                int M = 0;
                for (int k = board.size(p) - 1; k >= 0; k--) {
                    if (board.data[p][k] == clr) M++;
                    else break;
                }

                int g = board.size(target_y, target_x);
                int num_move = min(M, 8 - g);

                if (num_move > 0) {
                    execute_jump(cy, cx, dir, 1, num_move, output_flag, board, step_count);
                    current_stats.normal_moves++;
                    moved_any = true;
                    break; // 盤面が変わるため、最初から再走査
                }
            }
            if (moved_any) break;
            if (target_color != -1) break; // 特定色の処理完了時は抜ける
        }
    }
}

struct Cand {
    int score;
    int p;
};

// -------------------------------------------------------------
// 高速化版 simulate 関数
// -------------------------------------------------------------
int simulate(const FastBoard& start_board, const int* order, int sim_mode, 
        bool output_flag, int initial_steps = 0, int max_score = 1e9, bool enable_hub_homing = true, int iter = 0) {
    current_stats.reset();

    FastRNG sim_rng(1337ULL + (uint64_t)sim_mode * 10007ULL);
    FastRNG hub_rng(7777ULL + (uint64_t)sim_mode * 20011ULL);

    static int order_pos[12];
    for (int i = 0; i < K; i++) order_pos[order[i]] = i;

    FastBoard board = start_board;
    int step_count = initial_steps;

    for (int idx = 0; idx < K; idx++) {
        int c = order[idx];
        int ny = nests[c].first;
        int nx = nests[c].second;
        int nest_p = to_node(ny, nx);

        bool can_catapult = (idx < K - 1);

        // ==========================================
        // Phase 0: 巣の隣接マスからの事前回収
        // ==========================================
        for (int dir = 0; dir < 4; dir++) {
            int cy = ny + dy[dir];
            int cx = nx + dx[dir];
            if (cy < 0 || N <= cy || cx < 0 || N <= cx || iswall[cy][cx]) continue;

            int cp = to_node(cy, cx);
            while (!board.empty(cp) && board.back(cp) == c) {
                if (step_count >= max_score) return 1e9;

                int M = 0;
                for (int k = board.size(cp) - 1; k >= 0; k--) {
                    if (board.data[cp][k] == c) M++;
                    else break;
                }

                int num_move = min(M, 8 - board.size(nest_p));
                if (num_move == 0) break;

                char d_char = getpath(cy, cx, ny, nx).dirs[0];
                execute_jump(cy, cx, d_char, 1, num_move, output_flag, board, step_count);
                current_stats.normal_moves++;
            }
        }

        // 自色スライムがすでに盤上に存在しない場合は次の色へ
        int self_cnt = 0;
        for (int t_idx = 0; t_idx < board.top_slimes_cnt[c]; t_idx++) {
            int p = board.top_slimes[c][t_idx];
            if (p == nest_p) continue;
            for (int k = board.sz[p] - 1; k >= 0; k--) {
                if (board.data[p][k] == c) self_cnt++;
                else break;
            }
        }
        if (self_cnt == 0) continue;

        // ==========================================
        // Phase 1: 集結 (ハブへスライムを集める)
        // ==========================================
        while (true) {
            if (step_count >= max_score) return 1e9;

            bool moved_any = false;

            // ★ 改善: std::vector の生成・動的割り当てを排除し、静的配列を使用
            static Cand candidates[400];
            int cand_cnt = 0;

            for (int t_idx = 0; t_idx < board.top_slimes_cnt[c]; t_idx++) {
                int p = board.top_slimes[c][t_idx];
                if (p == nest_p) continue;

                int d = getdist(p, nest_p);
                if (d < 1e8) {
                    int noise = (sim_mode == 0) ? 0 : sim_rng.rand_int(100);
                    candidates[cand_cnt++] = {d * 1000 + noise, (int)p};
                }
            }

            if (cand_cnt == 0) break;

            sort(candidates, candidates + cand_cnt, [](const Cand& a, const Cand& b) {
                return a.score > b.score;
            });

            for (int c_i = 0; c_i < cand_cnt; c_i++) {
                int p = candidates[c_i].p;
                int cy = node_y(p);
                int cx = node_x(p);

                //1.【カタパルト試行】
                //巣との距離が2マス以内かどうか調べる
                int after_dist = getdist(cy, cx, ny, nx);
                if (after_dist <= 3) {
                    //高さが4以上か調べる
                    if (board.size(p) >= 3) {
                        //そこをカタパルトにする！
                        if (try_catapult_jump_at(cy, cx, c, order_pos, output_flag, board, step_count, sim_mode, 3)) {
                            moved_any = true;
                            break;
                        }
                    }
                }

                if (try_execute_combo_jump(cy, cx, c, ny, nx, order_pos, output_flag, board, step_count)) {
                    moved_any = true;
                    break;
                }

                // 2. ミニカタパルトの試行
                if (try_mini_catapult_jump(cy, cx, c, ny, nx, output_flag, board, step_count)) {
                    moved_any = true;
                    break;
                }

                // 3. タクシーの試行
                if (try_taxi_jump(cy, cx, c, ny, nx, order_pos, output_flag, board, step_count)) {
                    moved_any = true;
                    break;
                }

                // ★ 改善2: 通常移動 (1方向固定ではなく、ハブに近づける移動可能な方向を4方向から探索)
                int current_dist = getdist(cy, cx, ny, nx);
                int best_d = -1;
                int min_next_dist = 1e9;

                for (int d = 0; d < 4; d++) {
                    int target_y = cy + dy[d];
                    int target_x = cx + dx[d];

                    if (target_y < 0 || N <= target_y || target_x < 0 || N <= target_x || iswall[target_y][target_x]) continue;

                    int next_dist = getdist(target_y, target_x, ny, nx);
                    // ハブから遠ざからない方向（距離が短くなる、あるいは変わらない横移動）を許容
                    if (next_dist > current_dist) continue;

                    int M = 0;
                    for (int k = board.size(p) - 1; k >= 0; k--) {
                        if (board.data[p][k] == c) M++;
                        else break;
                    }

                    int g = board.size(target_y, target_x);
                    int max_cap = (target_y == ny && target_x == nx) ? 7 : 8;
                    int num_move = min(M, max_cap - g);

                    //移動前と移動後が共に同じ色で、ある程度高いならcontinue;
                    if(board.back(cy, cx) == board.back(target_y, target_x)
                        && board.size(cy, cx) >= 7 && board.size(target_y, target_x) >= 7) continue;


                    if (num_move > 0) {
                        // よりハブに近づける方向を優先
                        if (next_dist < min_next_dist) {
                            min_next_dist = next_dist;
                            best_d = d;
                        }
                    }
                }

                // 移動可能な方向が見つかった場合、実行
                if (best_d != -1) {
                    char dir = dir_char[best_d];
                    int target_y = cy + get_dy(dir);
                    int target_x = cx + get_dx(dir);

                    int M = 0;
                    for (int k = board.size(p) - 1; k >= 0; k--) {
                        if (board.data[p][k] == c) M++;
                        else break;
                    }

                    int g = board.size(target_y, target_x);
                    int max_cap = (target_y == ny && target_x == nx) ? 7 : 8;
                    int num_move = min(M, max_cap - g);

                    execute_jump(cy, cx, dir, 1, num_move, output_flag, board, step_count);
                    current_stats.normal_moves++;
                    moved_any = true;
                    break;
                }
            }

            if (!moved_any) break;
        }

        // ==========================================
        // Phase 2: 逐一帰巣 (enable_hub_homing == true の場合)
        // 盤上に残っている色 c のスライムをすべて巣 (ny, nx) へ送り切る
        // ==========================================
        if (enable_hub_homing) {
            bool moved_any = true;
            while (moved_any) {
                moved_any = false;
                if (step_count >= max_score) return 1e9;

                // 色 c のスライムが存在するすべてのセルを走査
                for (int t_idx = 0; t_idx < board.top_slimes_cnt[c]; t_idx++) {
                    int p = board.top_slimes[c][t_idx];
                    if (p == nest_p) continue; // すでに巣にあるものはスキップ

                    int cy = node_y(p);
                    int cx = node_x(p);

                    // そのマスの現在地から巣へのパスを取得
                    const Path& path = getpath(cy, cx, ny, nx);
                    if (path.len == 0) continue;

                    char dir = path.dirs[0];
                    int target_y = cy + get_dy(dir);
                    int target_x = cx + get_dx(dir);

                    // 最上段にある色 c のスライム数をカウント
                    int M = 0;
                    for (int k = board.size(p) - 1; k >= 0; k--) {
                        if (board.data[p][k] == c) M++;
                        else break;
                    }

                    int g = board.size(target_y, target_x);
                    int num_move = min(M, 8 - g);

                    if (num_move > 0) {
                        execute_jump(cy, cx, dir, 1, num_move, output_flag, board, step_count);
                        current_stats.normal_moves++;
                        moved_any = true;
                        break; // 1回移動したら盤面情報が更新されるため、最初から再走査
                    }
                }
            }
        }
    }

    // 最終スイープ (active_cells リストを活用)
    int sweep_guard = 0;
    while (sweep_guard++ < 300) {
        if (enable_hub_homing) break;
        if (step_count >= max_score) return 1e9;

        bool any_moved = false;

        for (int a_idx = 0; a_idx < board.active_cnt; a_idx++) {
            int p = board.active_cells[a_idx];
            int i = node_y(p);
            int j = node_x(p);

            int clr = board.back(p);
            auto [ny, nx] = nests[clr];

            if (i == ny && j == nx) continue;

            const Path& path = getpath(i, j, ny, nx);
            if (path.len == 0) continue;

            char dir = path.dirs[0];
            int target_y = i + get_dy(dir);
            int target_x = j + get_dx(dir);

            int M = 0;
            for (int k = board.size(p) - 1; k >= 0; k--) {
                if (board.data[p][k] == clr) M++;
                else break;
            }

            int g = board.size(target_y, target_x);
            int num_move = min(M, 8 - g);

            if (num_move > 0) {
                execute_jump(i, j, dir, 1, num_move, output_flag, board, step_count);
                current_stats.sweep_moves++;
                any_moved = true;
                break;
            }
        }

        if (!any_moved) break;
    }

    for (int a_idx = 0; a_idx < board.active_cnt; a_idx++) {
        int p = board.active_cells[a_idx];
        auto [ny, nx] = nests[board.back(p)];
        if (node_y(p) != ny || node_x(p) != nx) return 1e9;
    }

    return step_count;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N >> K)) return 0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            nest_color[i][j] = -1;
        }
    }

    initial_board.init();

    for (int i = 0; i < N; i++) {
        string s;
        cin >> s;
        for (int j = 0; j < N; j++) {
            char c = s[j];
            if (c == '#') {
                iswall[i][j] = true;
            } else if (c >= 'a' && c <= 'l') {
                int color = c - 'a';
                initial_board.push_back(i, j, color);
            } else if (c >= 'A' && c <= 'L') {
                int color = c - 'A';
                nests[color] = {i, j};
                nest_color[i][j] = color;
            }
        }
    }

    precompute_paths();

    for (int c = 0; c < K; c++) {
        auto [ny, nx] = nests[c];
        check_nest(ny, nx, initial_board);
    }

    int current_order[12];
    for (int i = 0; i < K; i++) current_order[i] = i;

    int current_sim_mode = 0;

    int best_order[12];
    memcpy(best_order, current_order, sizeof(current_order));
    int best_sim_mode = current_sim_mode;
    bool best_enable_homing = true;

    FastBoard best_initial_board = initial_board;
    vector<Move> best_prep_moves;

    int best_score = simulate(initial_board, best_order, best_sim_mode, false, 0, 1e9);
    int current_score = best_score;

    auto start_time = chrono::high_resolution_clock::now();
    double time_limit = 1.85;

    const double start_temp = 15.0;
    const double end_temp = 0.05;

    int iter = 0;
    int phase2_prep_count = 0;

    // 時間・温度関連変数をループ外で保持
    double elapsed = 0.0;
    double progress = 0.0;
    double temp = start_temp;
    bool enable_homing = true;
    int best_iter = 0;

    while (true) {
        // 時刻取得・温度計算を128回に1回に制限
        if ((iter & 127) == 0) {
            auto now = chrono::high_resolution_clock::now();
            elapsed = chrono::duration<double>(now - start_time).count();
            if (elapsed > time_limit) break;

            progress = elapsed / time_limit;
            temp = start_temp * pow(end_temp / start_temp, progress);
        }
        iter++;

        int new_order[12];
        memcpy(new_order, current_order, sizeof(current_order));
        int new_sim_mode = current_sim_mode;

        uint32_t op = rng.rand_int(100);

        if (op < 60) {
            int i = rng.rand_int(K);
            int j = rng.rand_int(K);
            swap(new_order[i], new_order[j]);
        } else if (op < 85) {
            int i = rng.rand_int(K);
            int j = rng.rand_int(K);
            int val = new_order[i];
            if (i < j) {
                for (int k = i; k < j; k++) new_order[k] = new_order[k + 1];
            } else {
                for (int k = i; k > j; k--) new_order[k] = new_order[k - 1];
            }
            new_order[j] = val;
        } else if (op < 90) {
            for (int i = K - 1; i > 0; i--) {
                int j = rng.rand_int(i + 1);
                swap(new_order[i], new_order[j]);
            }
        } else {
            new_sim_mode = rng.rand_int(32);
        }

        if (rng.rand_int(100) < 30) {
            new_sim_mode = rng.rand_int(32);
        }

        FastBoard working_initial_board = initial_board;
        vector<Move> current_prep_moves;
        int prep_steps = 0;

        if (elapsed > 0.5) {
            prep_steps = apply_random_prep(working_initial_board, rng, new_order, &current_prep_moves);
            phase2_prep_count++;
        }

        int max_allowed_score = current_score + 10;
        int new_score = simulate(working_initial_board, new_order, new_sim_mode, false, prep_steps, max_allowed_score, true, iter);

        int score_diff = current_score - new_score;

        bool accept = false;
        if (score_diff >= 0) {
            accept = true;
        } else {
            double rand_real = rng.rand_double(); 
            double prob = exp((double)score_diff / temp);
            
            if (rand_real < prob) {
                accept = true;
            }
        }

        if (accept) {
            current_score = new_score;
            current_sim_mode = new_sim_mode;
            memcpy(current_order, new_order, sizeof(new_order));

            if (new_score < best_score) {
                best_score = new_score;
                best_sim_mode = new_sim_mode;
                best_iter = iter;
                memcpy(best_order, new_order, sizeof(new_order));
                best_initial_board = working_initial_board;
                best_prep_moves = current_prep_moves;
                cerr << "[UPDATE] Iter: " << iter << " | Time: " << fixed << setprecision(3) << elapsed << "s | Mode: " << (enable_homing ? "Seq" : "Batch") << " | New Best: " << best_score << endl;
            }
        }
    }

    cerr << "\n========== DETAILED LOGS FOR BEST RUN ==========\n";
    for (const auto& m : best_prep_moves) {
        cout << m.y << " " << m.x << " " << m.k << " " << m.dir << " " << m.l << "\n";
    }

    int final_score = simulate(best_initial_board, best_order, best_sim_mode, true, (int)best_prep_moves.size(), 1e9, best_iter);

    cerr << "\n========== DEBUG SUMMARY ==========\n";
    cerr << "Total Iterations   : " << iter << "\n";
    cerr << "Phase2 Prep Runs   : " << phase2_prep_count << "\n";
    cerr << "Best Score (Steps) : " << final_score << "\n";
    cerr << "Prep Steps Used    : " << best_prep_moves.size() << "\n";
    cerr << "Best Sim Mode      : " << best_sim_mode << "\n";
    cerr << "Best Processing Order: ";
    for (int i = 0; i < K; i++) cerr << (char)('A' + best_order[i]) << " ";
    cerr << "\n-----------------------------------\n";
    cerr << "Action Breakdowns:\n";
    cerr << "  - Normal Moves     : " << current_stats.normal_moves << "\n";
    cerr << "  - Combo Jumps      : " << current_stats.combo_jumps << "\n";
    cerr << "  - Taxi Jumps       : " << current_stats.taxi_jumps << "\n";
    cerr << "  - Mini Catapults   : " << current_stats.mini_catapults << "\n";
    cerr << "  - Hub Catapults    : " << current_stats.hub_catapults << "\n";
    cerr << "  - Sweep Moves      : " << current_stats.sweep_moves << "\n";
    cerr << "===================================\n\n";

    return 0;
}