#pragma GCC optimize("O3,unroll-loops")

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

// 探索モードの定義
enum class SearchMode {
    NORMAL,
    FAST,
    SUPER_FAST
};

// ==========================================
// 高速かつ高品質な乱数生成器 (wyrand)
// ==========================================
struct FastRNG {
    uint64_t state = 88172645463325252ULL;

    inline uint64_t next() {
        state += 0xa0761d6478bd642fULL;
        unsigned __int128 mum = (unsigned __int128)state * (state ^ 0xe7037ed1a0b428dbULL);
        return (uint64_t)mum ^ (uint64_t)(mum >> 64);
    }

    inline int next_int(int mod) {
        if (mod <= 1) return 0;
        return (int)(((unsigned __int128)next() * (uint64_t)mod) >> 64);
    }

    inline double next_double() { // [0.0, 1.0)
        return (next() >> 11) * (1.0 / 9007199254740992.0);
    }
} rng;

// ==========================================
// パラメータ・定数設定
// ==========================================
double MIN_DENSITY = 0.4;            // 支払額 (vi) / マス数 (pi) のカットオフ閾値
const double LOW_C_THRESHOLD = 0.65;  // これ未満のコンパクト度なら移動シミュレーションを発動
const double EARLY_EXIT_C = 0.95;    // 標準的な探索打ち切り閾値
const int K_TRIALS = 40;             // 決定した枠/エリア内での起点選び直し試行回数
double TOUCH_DEGREE = 0.01;          // touchの係数
double EXPOSED_PENALTY = 0.009;      // 露出周長1マスあたりのペナルティ係数
double SHAPE_MARGIN = 1.2;           // 枠のマージン
const int MAX_P = 150;

// 退去時間の同期配置用パラメータ
double TAU = 2200.0;           // 退去時刻差の許容スケール
double inv_TAU = 1.0 / 2200.0; // TAUの逆数
const int INF_TIME = 1e6;      // 永久障害物の退去時刻表現

// モンテカルロ評価用パラメータ
const int TOP_K_CANDIDATES = 3;   // 残す上位候補数 K
const int MC_N_SIM = 3;           // 1候補あたりのシミュレーション回数
const double MC_ALPHA = 0.0007;   // 未来予測評価の重み

// 即採用ロジック用パラメータ
const int SHORT_DURATION_THRESH = 4000;    // 案① 「滞在時間 (ti - si)」の短さの閾値
const int SMALL_P_THRESH = 55;             // 案② 「マス数 Pi」の小ささの閾値
const double LAWN_USAGE_LOW_THRESH = 0.40; // 案② 芝生利用率の低さの閾値 (55%)

// ==========================================
// グローバル変数・盤面管理 (52x52)
// ==========================================
int N, M;
double R; 
int grid[52][52];             // 0: 芝生, 1: 池/他グループ/一時障害物/壁, 2: 一時保護
int leave_time_grid[52][52];  // 各マスの退去予定時刻
bool in_use_temp[52][52];     // 評価計算用の作業用フラグ配列

int visited_stamp[52][52] = {0};
int current_stamp = 0;

// 二次元累積和用配列
int pref_empty[52][52];

const int dx[] = {0, -1, 0, 1};
const int dy[] = {-1, 0, 1, 0};

// ==========================================
// オンラインパラメータ推定 & 仮想データ生成
// ==========================================
struct OnlineEstimator {
    double sum_l = 0;
    int count = 0;

    void add_sample(int S, int T) {
        sum_l += (T - S - 1);
        count++;
    }

    double get_estimated_theta() const {
        if (count == 0) return 5000.0;
        return clamp(sum_l / count, 2000.0, 8000.0);
    }
} estimator;

struct VirtualGroup {
    int P;
    int duration;
    int V;
};

VirtualGroup generate_virtual_group() {
    double r = 2.0 + (sqrt(150.0) - 2.0) * rng.next_double();
    int P = clamp((int)round(r * r), 2, 150);

    double theta = estimator.get_estimated_theta();
    double u = rng.next_double();
    if (u >= 0.9999) u = 0.9999;
    int l = (int)round(-theta * log(1.0 - u));
    l = min(l, 99999);

    int duration = l + 1;

    double u1 = max(1e-9, rng.next_double());
    double u2 = rng.next_double();
    double gauss = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2) * 0.8;

    double v_raw = (double)P * pow((double)duration, 0.9) * pow(2.0, gauss);
    int V = clamp((int)round(v_raw), 1, 100000000);

    return {P, duration, V};
}

// ==========================================
// 二次元累積和関連関数 (1-based)
// ==========================================
void build_prefix_sum() {
    rep(y, 1, N + 1) {
        rep(x, 1, N + 1) {
            pref_empty[y][x] = pref_empty[y - 1][x] + pref_empty[y][x - 1]
                              - pref_empty[y - 1][x - 1] + (grid[y][x] == 0 ? 1 : 0);
        }
    }
}

inline int get_empty_count(int min_y, int max_y, int min_x, int max_x) {
    return pref_empty[max_y][max_x] - pref_empty[min_y - 1][max_x]
         - pref_empty[max_y][min_x - 1] + pref_empty[min_y - 1][min_x - 1];
}

// ==========================================
// 孤立マス（使用不能領域）の掃除関数
// ==========================================
int clean_isolated_cells(int min_p_limit = 4) {
    static bool visited[52][52];
    memset(visited, false, sizeof(visited));

    static pair<int, int> q[2700];
    int filled_count = 0;

    rep(i, 1, N + 1) {
        rep(j, 1, N + 1) {
            if (grid[i][j] == 0 && !visited[i][j]) {
                int head = 0, tail = 0;
                q[tail++] = {i, j};
                visited[i][j] = true;

                while (head < tail) {
                    auto [cy, cx] = q[head++];
                    rep(d, 0, 4) {
                        int ny = cy + dy[d];
                        int nx = cx + dx[d];
                        if (grid[ny][nx] == 0 && !visited[ny][nx]) {
                            visited[ny][nx] = true;
                            q[tail++] = {ny, nx};
                        }
                    }
                }

                if (tail < min_p_limit) {
                    rep(k, 0, tail) {
                        auto [y, x] = q[k];
                        grid[y][x] = 1;
                        leave_time_grid[y][x] = INF_TIME; // 永久壁扱い
                        filled_count++;
                    }
                }
            }
        }
    }
    return filled_count;
}

// ==========================================
// 補助構造体・関数の定義
// ==========================================

struct EvalResult {
    int L = 0;          // 外周長
    double C = 0.0;     // コンパクト度
    double touch = 0.0; // 接触スコア
    int exposed = 0;    // 露出周長（空きマスと接している辺数）
};

struct GroupState {
    int id;
    int leave_time;
    int P;
    int V;
    vpii cells;
    bool active;
};

struct PlacementResult {
    vpii cells;
    double C = 0.0;
    double score = -1e18;
};

struct GreedyResult {
    vpii cells;
    EvalResult eval;
};

struct CandidateGroup {
    int g_idx;     
    int V;         
    int P;         
    int move_cost; 
};

// ==========================================
// 盤面操作 (Do / Undo)
// ==========================================
inline void do_place(const vpii& cells, int leave_t) {
    for (auto [y, x] : cells) {
        grid[y][x] = 1;
        leave_time_grid[y][x] = leave_t;
    }
}

inline void undo_place(const vpii& cells) {
    for (auto [y, x] : cells) {
        grid[y][x] = 0;
        leave_time_grid[y][x] = 0;
    }
}

// ==========================================
// 評価関数
// ==========================================
double sqrt_4_table[MAX_P + 1];

void init_sqrt_table() {
    for (int p = 1; p <= MAX_P; p++) {
        sqrt_4_table[p] = 4.0 * sqrt((double)p);
    }
}

static int eval_stamp_grid[52][52] = {0};
static int eval_stamp = 0;

EvalResult evaluate_cells(const vpii &cells, int P, int cur_leave_time, SearchMode mode = SearchMode::NORMAL) {
    eval_stamp++;
    for (auto [y, x] : cells) {
        eval_stamp_grid[y][x] = eval_stamp;
    }

    int L = 0;
    double weighted_touch = 0.0;
    int exposed = 0;

    for (auto [y, x] : cells) {
        rep(d, 0, 4) {
            int ny = y + dy[d];
            int nx = x + dx[d];

            if (eval_stamp_grid[ny][nx] != eval_stamp) {
                L++;
                
                if (ny >= 1 && ny <= N && nx >= 1 && nx <= N && grid[ny][nx] == 0) {
                    exposed++;
                } else if (grid[ny][nx] == 1) {
                    int nxt_time = leave_time_grid[ny][nx];
                    if (nxt_time == INF_TIME) {
                        weighted_touch += 1.0;
                    } else if (mode != SearchMode::SUPER_FAST && nxt_time > 0) {
                        int diff = abs(cur_leave_time - nxt_time);
                        if (diff < 2200) {
                            weighted_touch += (double)(2200 - diff) * inv_TAU;
                        }
                    }
                }
            }
        }
    }

    double C = (L > 0) ? (sqrt_4_table[P] / (double)L) : 0.0;
    return {L, C, weighted_touch, exposed};
}

vpii fill_recesses() {
    vpii filled;
    static pair<int, int> q[2700];
    int q_head = 0, q_tail = 0;

    rep(y, 1, N + 1) {
        rep(x, 1, N + 1) {
            if (grid[y][x] == 0) {
                int blocked = 0;
                rep(d, 0, 4) {
                    int ny = y + dy[d];
                    int nx = x + dx[d];
                    if (grid[ny][nx] == 1) blocked++;
                }
                if (blocked >= 3) {
                    q[q_tail++] = {y, x};
                }
            }
        }
    }

    while (q_head < q_tail) {
        auto [y, x] = q[q_head++];
        if (grid[y][x] != 0) continue;

        int blocked = 0;
        rep(d, 0, 4) {
            int ny = y + dy[d];
            int nx = x + dx[d];
            if (grid[ny][nx] == 1) blocked++;
        }

        if (blocked >= 3) {
            grid[y][x] = 1; 
            filled.push_back({y, x});

            rep(d, 0, 4) {
                int ny = y + dy[d];
                int nx = x + dx[d];
                if (grid[ny][nx] == 0) {
                    q[q_tail++] = {ny, nx};
                }
            }
        }
    }

    return filled;
}

// ==========================================
// 起点候補取得 (1パス高速化版)
// ==========================================
vpii get_start_candidates() {
    vpii c2, c1, c0;
    c2.reserve(250); c1.reserve(250); c0.reserve(250);

    rep(y, 1, N + 1) {
        rep(x, 1, N + 1) {
            if (grid[y][x] != 0) continue;

            int touch = (grid[y - 1][x] == 1) + (grid[y + 1][x] == 1) + 
                        (grid[y][x - 1] == 1) + (grid[y][x + 1] == 1);

            if (touch >= 2) c2.push_back({y, x});
            else if (touch == 1) c1.push_back({y, x});
            else c0.push_back({y, x});
        }
    }

    // src からランダムに count 個選んで dest に追加するヘルパー関数
    auto append_samples = [](vpii& dest, vpii& src, int count) {
        if (src.empty() || count <= 0) return;
        if ((int)src.size() <= count) {
            dest.insert(dest.end(), src.begin(), src.end());
        } else {
            // 先頭 count 個分だけ部分シャッフルして抽出
            rep(i, 0, count) {
                int r = i + rng.next_int((int)src.size() - i);
                swap(src[i], src[r]);
                dest.push_back(src[i]);
            }
        }
    };

    if (!c2.empty()) {
        int add_cnt = c2.size() * 0.7;
        append_samples(c2, c1, add_cnt);
        append_samples(c2, c0, add_cnt);
        return c2;
    }
    
    if (!c1.empty()) {
        int add_cnt = c1.size() * 0.7;
        append_samples(c1, c0, add_cnt);
        return c1;
    }

    return c0;
}

// ==========================================
// Region Growing (領域拡張・評価同時計算版)
// ==========================================
GreedyResult grow_region_greedy(
    int ry, int rx, int P,
    int cur_leave_time,
    SearchMode mode = SearchMode::NORMAL,
    int min_y = 1, int max_y = 50,
    int min_x = 1, int max_x = 50
) {
    if (grid[ry][rx] != 0) return {{}, {}};

    vpii cells;
    cells.reserve(P);

    current_stamp++;
    int local_stamp = current_stamp;

    int L = 0;
    double weighted_touch = 0.0;
    int exposed = 0;

    auto add_cell_eval = [&](int y, int x) {
        rep(d, 0, 4) {
            int ny = y + dy[d];
            int nx = x + dx[d];

            if (visited_stamp[ny][nx] == local_stamp) {
                L--;
                exposed--;
            } else {
                L++;
                if (grid[ny][nx] == 0) {
                    exposed++;
                } else if (grid[ny][nx] == 1) {
                    int nxt_time = leave_time_grid[ny][nx];
                    if (nxt_time == INF_TIME) {
                        weighted_touch += 1.0;
                    } else if (mode != SearchMode::SUPER_FAST && nxt_time > 0) {
                        int diff = abs(cur_leave_time - nxt_time);
                        if (diff < 2200) {
                            weighted_touch += (double)(2200 - diff) * inv_TAU;
                        }
                    }
                }
            }
        }
    };

    visited_stamp[ry][rx] = local_stamp;
    cells.push_back({ry, rx});
    add_cell_eval(ry, rx);

    if (P == 1) {
        double C = (L > 0) ? (sqrt_4_table[P] / (double)L) : 0.0;
        return {cells, {L, C, weighted_touch, exposed}};
    }

    static vector<pair<int, int>> rect_candidates;
    rect_candidates.clear();
    int side = (int)round(sqrt(P));
    for (int w = max(1, side - 3); w <= side + 4; w++) {
        int h = (P + w - 1) / w;
        if (w * h >= P) {
            rect_candidates.push_back({w, h});
            rect_candidates.push_back({h, w});
        }
    }
    if (rect_candidates.empty()) rect_candidates.push_back({side, (P + side - 1) / side});

    auto [W, H] = rect_candidates[rng.next_int((int)rect_candidates.size())];

    int offset_y = rng.next_int(H);
    int offset_x = rng.next_int(W);

    int box_min_y = max(min_y, ry - offset_y);
    int box_max_y = min(max_y, box_min_y + H - 1);
    box_min_y = max(min_y, box_max_y - H + 1);

    int box_min_x = max(min_x, rx - offset_x);
    int box_max_x = min(max_x, box_min_x + W - 1);
    box_min_x = max(min_x, box_max_x - W + 1);

    static pair<int, int> bucket_data[205][250];
    static int bucket_sz[205] = {0};
    static int active_keys[205];
    static int active_keys_cnt = 0;
    static bool in_active[205] = {false};

    rep(k, 0, active_keys_cnt) {
        int key = active_keys[k];
        bucket_sz[key] = 0;
        in_active[key] = false;
    }
    active_keys_cnt = 0;

    auto calc_key = [&](int y, int x) -> int {
        bool in_box = (y >= box_min_y && y <= box_max_y && x >= box_min_x && x <= box_max_x);
        int adj_in_S = 0;
        int wall_touch = 0;

        rep(d, 0, 4) {
            int ny = y + dy[d];
            int nx = x + dx[d];

            if (ny < 1 || ny > N || nx < 1 || nx > N) continue;

            if (visited_stamp[ny][nx] == local_stamp) {
                adj_in_S++;
            } else if (grid[ny][nx] == 1) {
                wall_touch++;
            }
        }
        return (in_box ? 100 : 0) + adj_in_S * 10 + wall_touch;
    };

    int max_key = -1;

    auto push_cand = [&](int y, int x) {
        int key = calc_key(y, x);
        if (key < 0 || key >= 205) return;
        if (bucket_sz[key] >= 245) return;

        if (bucket_sz[key] == 0 && !in_active[key]) {
            in_active[key] = true;
            active_keys[active_keys_cnt++] = key;
        }
        bucket_data[key][bucket_sz[key]++] = {y, x};
        if (key > max_key) max_key = key;
    };

    rep(d, 0, 4) {
        int ny = ry + dy[d];
        int nx = rx + dx[d];
        if (ny >= min_y && ny <= max_y && nx >= min_x && nx <= max_x) {
            if (grid[ny][nx] == 0 && visited_stamp[ny][nx] != local_stamp) {
                push_cand(ny, nx);
            }
        }
    }

    while (max_key >= 0 && (int)cells.size() < P) {
        while (max_key >= 0 && bucket_sz[max_key] == 0) {
            max_key--;
        }
        if (max_key < 0) break;

        auto [cy, cx] = bucket_data[max_key][--bucket_sz[max_key]];

        if (visited_stamp[cy][cx] == local_stamp) continue;

        visited_stamp[cy][cx] = local_stamp;
        cells.push_back({cy, cx});
        add_cell_eval(cy, cx);

        if ((int)cells.size() == P) break;

        rep(d, 0, 4) {
            int ny = cy + dy[d];
            int nx = cx + dx[d];

            if (ny < min_y || ny > max_y || nx < min_x || nx > max_x) continue;
            if (grid[ny][nx] != 0 || visited_stamp[ny][nx] == local_stamp) continue;

            push_cand(ny, nx);
        }
    }

    if ((int)cells.size() < P) return {{}, {}};

    double C = (L > 0) ? (sqrt_4_table[P] / (double)L) : 0.0;
    return {cells, {L, C, weighted_touch, exposed}};
}

vector<vpii> target_shapes_table;

void init_target_shapes() {
    target_shapes_table.resize(MAX_P + 1);
    for (int p = 1; p <= MAX_P; p++) {
        int target_area = p * SHAPE_MARGIN; 
        int base = sqrt(target_area);
        
        for (int w = max(1, base - 3); w <= base + 4; w++) {
            int h = (target_area + w - 1) / w;
            target_shapes_table[p].push_back({w, h});
        }
    }
}

inline const vpii& get_target_shapes(int P) {
    if (P > MAX_P) P = MAX_P;
    return target_shapes_table[P];
}

double calc_cell_support(int y, int x, int local_stamp, const int in_shape_stamp[52][52]) {
    int self_adj = 0;
    double wall_touch = 0.0;

    rep(d, 0, 4) {
        int ny = y + dy[d];
        int nx = x + dx[d];
        
        if (ny < 1 || ny > N || nx < 1 || nx > N) continue;

        if (in_shape_stamp[ny][nx] == local_stamp) {
            self_adj++;
        } else if (grid[ny][nx] == 1) {
            if (leave_time_grid[ny][nx] == INF_TIME) {
                wall_touch += 1.0;
            } else if (leave_time_grid[ny][nx] > 0) {
                wall_touch += 0.8;
            }
        }
    }
    return (double)self_adj + wall_touch;
}

// ==========================================
// 連結性判定関数（汎用版）
// ==========================================
bool check_connectivity(const vpii& cells, int P, int skip_idx = -1) {
    int target_cnt = (skip_idx >= 0) ? P - 1 : P;
    if (target_cnt <= 1) return true;

    static int conn_stamp[52][52] = {0};
    static int stamp = 0;

    stamp += 2;
    int target_stamp = stamp;
    int visited_stamp_val = stamp + 1;

    int start_idx = -1;
    rep(i, 0, P) {
        if (i == skip_idx) continue;
        conn_stamp[cells[i].first][cells[i].second] = target_stamp;
        if (start_idx == -1) start_idx = i;
    }

    static pair<int, int> q[160];
    int head = 0, tail = 0;
    q[tail++] = cells[start_idx];
    conn_stamp[cells[start_idx].first][cells[start_idx].second] = visited_stamp_val;

    int visited_cnt = 0;
    while (head < tail) {
        auto [cy, cx] = q[head++];
        visited_cnt++;

        rep(d, 0, 4) {
            int ny = cy + dy[d];
            int nx = cx + dx[d];

            if (conn_stamp[ny][nx] == target_stamp) {
                conn_stamp[ny][nx] = visited_stamp_val;
                q[tail++] = {ny, nx};
            }
        }
    }

    return visited_cnt == target_cnt;
}

// ==========================================
// 平滑化関数 (Tarjan Lowlink)
// ==========================================
vpii smooth_shape_wall_aware(vpii cells, int P) {
    if ((int)cells.size() != P || P <= 3) return cells;

    static int in_shape_stamp[52][52] = {0};
    static int cell_idx[52][52];
    static int local_stamp = 0;
    local_stamp++;

    for (auto [y, x] : cells) in_shape_stamp[y][x] = local_stamp;

    static int adj[MAX_P + 1][4];
    static int adj_deg[MAX_P + 1];
    static int ord[MAX_P + 1];
    static int low[MAX_P + 1];
    static bool is_cut[MAX_P + 1];

    bool updated = true;
    int max_loops = 10;

    while (updated && max_loops-- > 0) {
        updated = false;

        rep(i, 0, P) {
            cell_idx[cells[i].first][cells[i].second] = i;
            adj_deg[i] = 0;
            ord[i] = -1;
            low[i] = -1;
            is_cut[i] = false;
        }

        rep(i, 0, P) {
            auto [y, x] = cells[i];
            rep(d, 0, 4) {
                int ny = y + dy[d];
                int nx = x + dx[d];
                if (ny >= 1 && ny <= N && nx >= 1 && nx <= N && in_shape_stamp[ny][nx] == local_stamp) {
                    adj[i][adj_deg[i]++] = cell_idx[ny][nx];
                }
            }
        }

        int timer = 0;

        auto dfs = [&](auto self, int u, int p) -> void {
            ord[u] = low[u] = timer++;
            int children = 0;
            rep(i, 0, adj_deg[u]) {
                int v = adj[u][i];
                if (v == p) continue;
                if (ord[v] != -1) {
                    low[u] = min(low[u], ord[v]);
                } else {
                    children++;
                    self(self, v, u);
                    low[u] = min(low[u], low[v]);
                    if (p != -1 && low[v] >= ord[u]) {
                        is_cut[u] = true;
                    }
                }
            }
            if (p == -1 && children > 1) {
                is_cut[u] = true;
            }
        };

        dfs(dfs, 0, -1);

        int rem_idx = -1;
        double min_support = 1e9;

        rep(i, 0, P) {
            if (is_cut[i]) continue;

            auto [y, x] = cells[i];
            double sup = calc_cell_support(y, x, local_stamp, in_shape_stamp);

            if (sup < min_support) {
                min_support = sup;
                rem_idx = i;
            }
        }

        if (rem_idx == -1 || min_support >= 1.99) break;

        if (!check_connectivity(cells, P, rem_idx)) {
            break;
        }

        auto rem_cell = cells[rem_idx];
        in_shape_stamp[rem_cell.first][rem_cell.second] = 0;

        double max_add_support = -1.0;
        pair<int, int> add_cell = {-1, -1};

        for (auto [cy, cx] : cells) {
            if (make_pair(cy, cx) == rem_cell) continue;

            rep(d, 0, 4) {
                int ny = cy + dy[d];
                int nx = cx + dx[d];

                if (ny < 1 || ny > N || nx < 1 || nx > N) continue;
                if (in_shape_stamp[ny][nx] == local_stamp) continue;
                if (grid[ny][nx] != 0) continue;

                double sup = calc_cell_support(ny, nx, local_stamp, in_shape_stamp);
                if (sup > max_add_support) {
                    max_add_support = sup;
                    add_cell = {ny, nx};
                }
            }
        }

        if (add_cell.first != -1 && max_add_support > min_support + 0.1) {
            auto old_cell = cells[rem_idx];
            cells[rem_idx] = add_cell;

            if (check_connectivity(cells, P)) {
                in_shape_stamp[add_cell.first][add_cell.second] = local_stamp;
                updated = true;
            } else {
                cells[rem_idx] = old_cell;
                in_shape_stamp[rem_cell.first][rem_cell.second] = local_stamp;
            }
        } else {
            in_shape_stamp[rem_cell.first][rem_cell.second] = local_stamp;
        }
    }

    return cells;
}

// ==========================================
// 焼きなまし法 (local_optimize_swap)
// ==========================================
PlacementResult local_optimize_swap(
    vpii cells,
    int P,
    int cur_leave_time,
    int trials
) {
    if ((int)cells.size() != P || trials <= 0) {
        EvalResult ev = evaluate_cells(cells, P, cur_leave_time, SearchMode::NORMAL);
        return {cells, ev.C, ev.C + TOUCH_DEGREE * ev.touch - EXPOSED_PENALTY * ev.exposed};
    }

    vpii smoothed_cells = smooth_shape_wall_aware(cells, P);
    if ((int)smoothed_cells.size() == P) {
        cells = smoothed_cells;
    }

    static int in_area_stamp[52][52] = {0};
    static int local_stamp = 0;
    local_stamp++;

    for (auto [y, x] : cells) {
        in_area_stamp[y][x] = local_stamp;
    }

    EvalResult cur_eval = evaluate_cells(cells, P, cur_leave_time, SearchMode::NORMAL);
    
    int cur_L = cur_eval.L;
    double cur_touch = cur_eval.touch;
    int cur_exposed = cur_eval.exposed;

    double cur_score = cur_eval.C + TOUCH_DEGREE * cur_eval.touch - EXPOSED_PENALTY * cur_eval.exposed;

    vpii best_cells = cells;
    double best_score = cur_score;
    double best_C = cur_eval.C;

    const double start_temp = 0.03;
    const double end_temp   = 0.001;

    const double double_sqrt_P_4 = sqrt_4_table[P];

    static vi removable;
    static vpii add_candidates;
    removable.reserve(P);
    add_candidates.reserve(P * 4);

    rep(iter, 0, trials) {
        double temp = start_temp + (end_temp - start_temp) * ((double)iter / trials);

        removable.clear();
        rep(i, 0, (int)cells.size()) {
            auto [y, x] = cells[i];
            int deg = 0;
            rep(d, 0, 4) {
                int ny = y + dy[d];
                int nx = x + dx[d];
                if (ny >= 1 && ny <= N && nx >= 1 && nx <= N && in_area_stamp[ny][nx] == local_stamp) {
                    deg++;
                }
            }
            if (deg <= 1) removable.push_back(i);
        }

        if (removable.empty()) break;

        int rem_idx = removable[rng.next_int((int)removable.size())];
        auto removed_cell = cells[rem_idx];

        in_area_stamp[removed_cell.first][removed_cell.second] = 0;

        add_candidates.clear();
        for (auto [y, x] : cells) {
            if (make_pair(y, x) == removed_cell) continue;
            rep(d, 0, 4) {
                int ny = y + dy[d];
                int nx = x + dx[d];
                if (ny < 1 || ny > N || nx < 1 || nx > N) continue;
                if (in_area_stamp[ny][nx] == local_stamp) continue;
                if (grid[ny][nx] != 0) continue;
                
                add_candidates.push_back({ny, nx});
            }
        }

        if (add_candidates.empty()) {
            in_area_stamp[removed_cell.first][removed_cell.second] = local_stamp;
            continue;
        }

        auto add_cell = add_candidates[rng.next_int((int)add_candidates.size())];

        int L_diff = 0;
        double touch_diff = 0.0;
        int exposed_diff = 0;

        auto [ry, rx] = removed_cell;
        auto [ay, ax] = add_cell;

        // 1. 削除セルによる差分計算
        rep(d, 0, 4) {
            int ny = ry + dy[d];
            int nx = rx + dx[d];
            if (ny >= 1 && ny <= N && nx >= 1 && nx <= N) {
                if (in_area_stamp[ny][nx] == local_stamp) {
                    L_diff += 1;
                    exposed_diff += 1;
                } else {
                    L_diff -= 1;
                    if (grid[ny][nx] == 1) {
                        int nxt_time = leave_time_grid[ny][nx];
                        if (nxt_time == INF_TIME) {
                            touch_diff -= 1.0;
                        } else if (nxt_time > 0) {
                            int diff = abs(cur_leave_time - nxt_time);
                            if (diff < 2200) {
                                touch_diff -= (double)(2200 - diff) * inv_TAU;
                            }
                        }
                    } else if (grid[ny][nx] == 0) {
                        exposed_diff -= 1;
                    }
                }
            }
        }

        in_area_stamp[ay][ax] = local_stamp;

        // 2. 追加セルによる差分計算
        rep(d, 0, 4) {
            int ny = ay + dy[d];
            int nx = ax + dx[d];
            if (ny >= 1 && ny <= N && nx >= 1 && nx <= N) {
                if (in_area_stamp[ny][nx] == local_stamp) {
                    L_diff -= 1;
                    exposed_diff -= 1;
                } else {
                    L_diff += 1;
                    if (grid[ny][nx] == 1) {
                        int nxt_time = leave_time_grid[ny][nx];
                        if (nxt_time == INF_TIME) {
                            touch_diff += 1.0;
                        } else if (nxt_time > 0) {
                            int diff = abs(cur_leave_time - nxt_time);
                            if (diff < 2200) {
                                touch_diff += (double)(2200 - diff) * inv_TAU;
                            }
                        }
                    } else if (grid[ny][nx] == 0) {
                        exposed_diff += 1;
                    }
                }
            }
        }

        int new_L = cur_L + L_diff;
        double new_touch = cur_touch + touch_diff;
        int new_exposed = cur_exposed + exposed_diff;

        double new_C = (new_L > 0) ? (double_sqrt_P_4 / (double)new_L) : 0.0;
        double score = new_C + TOUCH_DEGREE * new_touch - EXPOSED_PENALTY * new_exposed;
        double delta = score - cur_score;

        if (delta > 0.0) {
            cur_score = score;
            cur_L = new_L;
            cur_touch = new_touch;
            cur_exposed = new_exposed;
            cells[rem_idx] = add_cell;

            if (score > best_score) {
                best_score = score;
                best_cells = cells;
                best_C = new_C;
                if (new_C >= EARLY_EXIT_C) return {best_cells, best_C, best_score};
            }
        } else {
            double prob_arg = delta / temp;
            if (prob_arg > -8.0 && exp(prob_arg) > rng.next_double()) {
                cur_score = score;
                cur_L = new_L;
                cur_touch = new_touch;
                cur_exposed = new_exposed;
                cells[rem_idx] = add_cell;

                if (score > best_score) {
                    best_score = score;
                    best_cells = cells;
                    best_C = new_C;
                    if (new_C >= EARLY_EXIT_C) return {best_cells, best_C, best_score};
                }
            } else {
                in_area_stamp[ay][ax] = 0;
                in_area_stamp[ry][rx] = local_stamp;
            }
        }
    }

    return {best_cells, best_C, best_score};
}

// ==========================================
// 上位 K 個候補の動的挿入・維持処理
// ==========================================
inline void add_candidate(vector<PlacementResult>& top_k, const PlacementResult& cand, int K = TOP_K_CANDIDATES) {
    for (const auto& item : top_k) {
        if (abs(item.score - cand.score) < 1e-9 && item.cells == cand.cells) {
            return;
        }
    }

    int pos = 0;
    while (pos < (int)top_k.size() && top_k[pos].score >= cand.score) {
        pos++;
    }

    if (pos < K) {
        top_k.insert(top_k.begin() + pos, cand);
        if ((int)top_k.size() > K) {
            top_k.pop_back();
        }
    }
}

// ==========================================
// 上位 K 個の配置候補取得処理
// ==========================================
vector<PlacementResult> find_top_k_placements(int P, int cur_leave_time, int K = TOP_K_CANDIDATES, SearchMode mode = SearchMode::NORMAL) {
    vpii filled_recesses = fill_recesses();
    build_prefix_sum();

    vpii start_candidates = get_start_candidates();

    vector<PlacementResult> top_k;
    top_k.reserve(K);

    int max_trials = 700;
    if (P < 20) max_trials = 600;
    else if (P < 50) max_trials = 750;

    int k_trials_count = K_TRIALS;

    if (mode == SearchMode::SUPER_FAST) {
        max_trials = min(50, max_trials);
        k_trials_count = min(10, K_TRIALS);
    } else if (mode == SearchMode::FAST) {
        max_trials = (max_trials * 2) / 3;
        k_trials_count = min(30, K_TRIALS);
    }

    int best_min_y = -1, best_max_y = -1, best_min_x = -1, best_max_x = -1;
    double stage1_best_score = -1e18;

    if (!start_candidates.empty()) {
        const auto& shapes = get_target_shapes(P);

        rep(trial, 0, max_trials) {
            auto [ry, rx] = start_candidates[rng.next_int((int)start_candidates.size())];
            auto [target_w, target_h] = shapes[rng.next_int((int)shapes.size())];

            int offset_y = rng.next_int(target_h);
            int offset_x = rng.next_int(target_w);

            int min_y = max(1, ry - offset_y);
            int max_y = min(N, min_y + target_h - 1);
            int min_x = max(1, rx - offset_x);
            int max_x = min(N, min_x + target_w - 1);

            if (get_empty_count(min_y, max_y, min_x, max_x) < P) continue;

            GreedyResult g_res = grow_region_greedy(ry, rx, P, cur_leave_time, mode, min_y, max_y, min_x, max_x);
            if ((int)g_res.cells.size() != P) continue;

            double score = g_res.eval.C + TOUCH_DEGREE * g_res.eval.touch - EXPOSED_PENALTY * g_res.eval.exposed;

            add_candidate(top_k, {g_res.cells, g_res.eval.C, score}, K);

            if (score > stage1_best_score) {
                stage1_best_score = score;
                best_min_y = min_y; best_max_y = max_y;
                best_min_x = min_x; best_max_x = max_x;

                if (g_res.eval.C >= EARLY_EXIT_C && trial > max_trials/2) break;
            }
        }

        if (!top_k.empty() && k_trials_count > 0 && best_min_y != -1) {
            vpii box_valid_starts;
            rep(y, best_min_y, best_max_y + 1) {
                rep(x, best_min_x, best_max_x + 1) {
                    if (grid[y][x] == 0) {
                        box_valid_starts.push_back({y, x});
                    }
                }
            }

            if (!box_valid_starts.empty() && get_empty_count(best_min_y, best_max_y, best_min_x, best_max_x) >= P) {
                rep(k, 0, k_trials_count) {
                    auto [ry, rx] = box_valid_starts[rng.next_int((int)box_valid_starts.size())];
                    GreedyResult g_res = grow_region_greedy(ry, rx, P, cur_leave_time, mode, best_min_y, best_max_y, best_min_x, best_max_x);
                    if ((int)g_res.cells.size() != P) continue;

                    double score = g_res.eval.C + TOUCH_DEGREE * g_res.eval.touch - EXPOSED_PENALTY * g_res.eval.exposed;

                    add_candidate(top_k, {g_res.cells, g_res.eval.C, score}, K);
                }
            }
        }
    }

    if (top_k.size() < (size_t)K || top_k[0].C < EARLY_EXIT_C) {
        int fallback_trials = (P < 20) ? 150 : 400;
        if (mode == SearchMode::SUPER_FAST) fallback_trials = 30;
        else if (mode == SearchMode::FAST) fallback_trials = (fallback_trials * 2) / 3;

        if (!start_candidates.empty() && get_empty_count(1, N, 1, N) >= P) {
            rep(trial, 0, fallback_trials) {
                auto [ry, rx] = start_candidates[rng.next_int((int)start_candidates.size())];
                GreedyResult g_res = grow_region_greedy(ry, rx, P, cur_leave_time, mode, 1, N, 1, N);
                if ((int)g_res.cells.size() != P) continue;

                double score = g_res.eval.C + TOUCH_DEGREE * g_res.eval.touch - EXPOSED_PENALTY * g_res.eval.exposed;

                add_candidate(top_k, {g_res.cells, g_res.eval.C, score}, K);
                if (g_res.eval.C >= EARLY_EXIT_C && trial > fallback_trials/2) break;
            }
        }
    }

    for (auto [y, x] : filled_recesses) {
        grid[y][x] = 0;
    }
    build_prefix_sum();

    if (top_k.empty()) {
        const auto& fallback_starts = start_candidates;
        if (!fallback_starts.empty() && get_empty_count(1, N, 1, N) >= P) {
            int fb_trials = 100;
            if (mode == SearchMode::SUPER_FAST) fb_trials = 15;
            else if (mode == SearchMode::FAST) fb_trials = 50;

            rep(trial, 0, fb_trials) {
                auto [ry, rx] = fallback_starts[rng.next_int((int)fallback_starts.size())];
                GreedyResult g_res = grow_region_greedy(ry, rx, P, cur_leave_time, mode, 1, N, 1, N);
                if ((int)g_res.cells.size() != P) continue;

                double score = g_res.eval.C + TOUCH_DEGREE * g_res.eval.touch - EXPOSED_PENALTY * g_res.eval.exposed;

                add_candidate(top_k, {g_res.cells, g_res.eval.C, score}, K);
                if (g_res.eval.C >= EARLY_EXIT_C && trial > fb_trials/2) break;
            }
        }
    }

    return top_k;
}

PlacementResult find_best_placement(int P, int cur_leave_time, SearchMode mode = SearchMode::NORMAL) {
    auto top_k = find_top_k_placements(P, cur_leave_time, 1, mode);
    if (top_k.empty()) return PlacementResult();

    if (mode == SearchMode::NORMAL) {
        int sa_trials = (P < 50) ? 80 : 150;
        top_k[0] = local_optimize_swap(top_k[0].cells, P, cur_leave_time, sa_trials);
    }

    return top_k[0];
}

PlacementResult find_best_placement_mc(int P, int cur_leave_time, int cur_start_time, SearchMode mode = SearchMode::NORMAL) {
    vector<PlacementResult> candidates = find_top_k_placements(P, cur_leave_time, TOP_K_CANDIDATES, mode);

    if (candidates.empty()) return PlacementResult();

    int best_idx = 0;

    PlacementResult winner = candidates[best_idx];
    if (mode == SearchMode::NORMAL && winner.C < EARLY_EXIT_C) {
        int sa_trials = (P < 50) ? 60 : 120;
        winner = local_optimize_swap(winner.cells, P, cur_leave_time, sa_trials);
    }

    return winner;
}

// ==========================================
// 全空きマス数と最大空き連結成分の取得（一括走査版）
// ==========================================
tuple<int, int, vpii> get_empty_component_info() {
    static bool visited[52][52];
    memset(visited, false, sizeof(visited));

    int total_empty_count = 0;
    int max_size = 0;
    vpii max_cells;

    static pair<int, int> q[2700];

    rep(i, 1, N + 1) {
        rep(j, 1, N + 1) {
            if (grid[i][j] == 0) {
                total_empty_count++;
                if (!visited[i][j]) {
                    int head = 0, tail = 0;
                    q[tail++] = {i, j};
                    visited[i][j] = true;

                    while (head < tail) {
                        auto [cy, cx] = q[head++];
                        rep(d, 0, 4) {
                            int ny = cy + dy[d];
                            int nx = cx + dx[d];
                            if (grid[ny][nx] == 0 && !visited[ny][nx]) {
                                visited[ny][nx] = true;
                                q[tail++] = {ny, nx};
                            }
                        }
                    }

                    if (tail > max_size) {
                        max_size = tail;
                        max_cells.assign(q, q + tail);
                    }
                }
            }
        }
    }

    return {total_empty_count, max_size, max_cells};
}

// ==========================================
// 池（初期障害物）の平均クラスタサイズ計算関数
// ==========================================
pair<int, double> get_avg_pond_cluster_size() {
    static bool visited[52][52];
    memset(visited, false, sizeof(visited));

    int total_pond_cells = 0;
    int cluster_count = 0;

    static pair<int, int> q[52*52];

    rep(i, 1, N + 1) {
        rep(j, 1, N + 1) {
            if (grid[i][j] == 1 && leave_time_grid[i][j] == INF_TIME && !visited[i][j]) {
                cluster_count++;
                int head = 0, tail = 0;
                q[tail++] = {i, j};
                visited[i][j] = true;

                while (head < tail) {
                    auto [cy, cx] = q[head++];
                    rep(d, 0, 4) {
                        int ny = cy + dy[d];
                        int nx = cx + dx[d];
                        if (ny >= 1 && ny <= N && nx >= 1 && nx <= N) {
                            if (grid[ny][nx] == 1 && leave_time_grid[ny][nx] == INF_TIME && !visited[ny][nx]) {
                                visited[ny][nx] = true;
                                q[tail++] = {ny, nx};
                            }
                        }
                    }
                }
                total_pond_cells += tail;
            }
        }
    }

    if (cluster_count == 0) return {0, 0.0};
    return {total_pond_cells, (double)total_pond_cells / cluster_count};
}

// ==========================================
// 動的 EARLY_EXIT_C の計算
// ==========================================
double pond_panelty = 0.0;
double get_dynamic_early_exit_c(int current_empty, int initial_empty, int pi, double density) {
    double empty_ratio = (double)current_empty / (double)initial_empty;
    double target_C = 0.65 + 0.30 * empty_ratio - pond_panelty;

    if (pi > 80) {
        target_C -= 0.12;
    } else if (pi > 40) {
        target_C -= 0.07;
    } else if (pi > 20) {
        target_C -= 0.03;
    }

    return max(0.60, target_C);
}

// ==========================================
// メイン処理
// ==========================================
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N >> M >> R;

    rep(i, 0, 52) {
        rep(j, 0, 52) {
            grid[i][j] = 1;
            leave_time_grid[i][j] = INF_TIME;
        }
    }
    memset(in_use_temp, false, sizeof(in_use_temp));

    init_target_shapes();
    init_sqrt_table();

    rep(i, 1, N + 1) {
        string s;
        cin >> s;
        rep(j, 1, N + 1) {
            if (s[j - 1] == '#') {
                grid[i][j] = 1;
                leave_time_grid[i][j] = INF_TIME;
            } else {
                grid[i][j] = 0;
                leave_time_grid[i][j] = 0;
            }
        }
    }

    clean_isolated_cells(4);

    auto [initial_pond_count, pond_avg_cluster_size] = get_avg_pond_cluster_size();
    int initial_empty_count = 2500 - initial_pond_count;
    if (pond_avg_cluster_size < 3.0) pond_panelty = 0.02;

    vector<GroupState> active_groups;

    int early_adopted_count = 0;   // 改善案①用: 即採用したグループ数のカウンター
    int small_p_boost_counter = 0; // 改善案②用: 審査グループ数の残りカウンター

    rep(t, 0, M) {
        int cur_i, si, ti, pi, vi_val;
        cin >> cur_i >> si >> ti >> pi >> vi_val;
        double density = (double)vi_val / (pi * (ti - si));

        estimator.add_sample(si, ti);

        for (auto &g : active_groups) {
            if (g.active && si > g.leave_time) {
                for (auto [y, x] : g.cells) {
                    grid[y][x] = 0;
                    leave_time_grid[y][x] = 0;
                }
                g.active = false;
            }
        }

        auto [current_empty_count, max_empty_size, max_empty_cells] = get_empty_component_info();

        // 芝生マスの利用率計算 (使用中芝生マス数 / 初期芝生マス数)
        double current_lawn_usage = 1.0 - ((double)current_empty_count / (double)initial_empty_count);

        // 改善案②: 芝生マスの利用率が一定値を下回ったら、フラグ（カウンター3）をリセット/発動
        if (current_lawn_usage < LAWN_USAGE_LOW_THRESH) {
            small_p_boost_counter = 6;
        }

        // ------------------------------------------------------------------
        // 改善案①：最初の3グループ採用までの「退去時刻ti（滞在時間）が短かったら即採用」
        // ------------------------------------------------------------------
        if (early_adopted_count < 3 && (ti - si) <= SHORT_DURATION_THRESH && max_empty_size >= pi) {
            PlacementResult fast_place = find_best_placement(pi, ti, SearchMode::FAST);
            if (!fast_place.cells.empty() && fast_place.C >= 0.65) {
                cout << 0 << "\n";
                cout << "Yes" << "\n";
                for (auto [y, x] : fast_place.cells) {
                    grid[y][x] = 1;
                    leave_time_grid[y][x] = ti;
                    cout << y - 1 << " " << x - 1 << "\n";
                }
                active_groups.push_back({cur_i, ti, pi, vi_val, fast_place.cells, true});
                early_adopted_count++; // 「採用」された場合のみカウントアップ
                cout.flush();
                continue;
            }
        }

        // ------------------------------------------------------------------
        // 改善案②：利用率低下時の「小マス数Pの即採用」ブースト
        // ------------------------------------------------------------------
        if (small_p_boost_counter > 0) {
            small_p_boost_counter--; // 審査グループ数を1減らす

            if (pi <= SMALL_P_THRESH && max_empty_size >= pi) {
                PlacementResult fast_place = find_best_placement(pi, ti, SearchMode::FAST);
                if (!fast_place.cells.empty() && fast_place.C >= 0.60) {
                    cout << 0 << "\n";
                    cout << "Yes" << "\n";
                    for (auto [y, x] : fast_place.cells) {
                        grid[y][x] = 1;
                        leave_time_grid[y][x] = ti;
                        cout << y - 1 << " " << x - 1 << "\n";
                    }
                    active_groups.push_back({cur_i, ti, pi, vi_val, fast_place.cells, true});
                    cout.flush();
                    continue;
                }
            }
        }

        // ------------------------------------------------------------------
        // 既存の空きマス割合基準による高速採用分岐
        // ------------------------------------------------------------------
        if (current_empty_count > initial_empty_count * 0.35 && max_empty_size >= pi) {
            PlacementResult fast_place = find_best_placement(pi, ti, SearchMode::FAST);
            
            double dynamic_early_exit_C = get_dynamic_early_exit_c(
                current_empty_count, 
                initial_empty_count, 
                pi, 
                density
            );
            
            if (!fast_place.cells.empty() && fast_place.C >= dynamic_early_exit_C) {
                cout << 0 << "\n";
                cout << "Yes" << "\n";
                for (auto [y, x] : fast_place.cells) {
                    grid[y][x] = 1;
                    leave_time_grid[y][x] = ti;
                    cout << y - 1 << " " << x - 1 << "\n";
                }
                active_groups.push_back({cur_i, ti, pi, vi_val, fast_place.cells, true});
                
                cout.flush();
                continue;
            }
        }

        bool moved = false;
        vpii pre_calculated_i_cells;

        PlacementResult direct_place = {{}, 0.0, -1.0};
        double direct_profit = 0.0;

        if (si > 92000 && initial_empty_count > 2250) MIN_DENSITY = 0.3;
        else if (si > 92000) MIN_DENSITY = 0.25;

        if (si > 95000 && initial_empty_count > 2250) MIN_DENSITY = 0.2;
        else if (si > 95000) MIN_DENSITY = 0.15;

        if (max_empty_size >= pi && density >= MIN_DENSITY) {
            direct_place = find_best_placement_mc(pi, ti, si, SearchMode::NORMAL);
            if (!direct_place.cells.empty()) {
                direct_profit = vi_val * direct_place.C;
            }
        }

        bool need_move_search = (max_empty_size < pi || direct_place.cells.empty() || direct_place.C < LOW_C_THRESHOLD);

        if (need_move_search && density >= MIN_DENSITY) {
            bool is_max_component[52][52];
            memset(is_max_component, false, sizeof(is_max_component));
            for (auto [y, x] : max_empty_cells) {
                is_max_component[y][x] = true;
            }

            vector<CandidateGroup> candidates;

            rep(g_idx, 0, (int)active_groups.size()) {
                auto &g = active_groups[g_idx];
                if (!g.active) continue;

                bool is_adjacent = false;
                for (auto [gy, gx] : g.cells) {
                    rep(d, 0, 4) {
                        int ny = gy + dy[d];
                        int nx = gx + dx[d];
                        if (is_max_component[ny][nx]) {
                            is_adjacent = true;
                            break;
                        }
                    }
                    if (is_adjacent) break;
                }

                int move_cost = round((double)g.V * R);
                if (is_adjacent && (max_empty_size + g.P >= pi) && (vi_val > move_cost)) {
                    candidates.push_back({g_idx, g.V, g.P, move_cost});
                }
            }

            double A = 17 * (double)pi;

            sort(candidates.begin(), candidates.end(), [&](const CandidateGroup &a, const CandidateGroup &b) {
                double score_a = (double)a.V - A * (double)a.P;
                double score_b = (double)b.V - A * (double)b.P;
                return score_a < score_b;
            });

            int test_count = min((int)candidates.size(), 5);

            rep(c_i, 0, test_count) {
                auto &cand_info = candidates[c_i];
                auto &g = active_groups[cand_info.g_idx];
                int move_cost = cand_info.move_cost;

                double g_old_C = evaluate_cells(g.cells, g.P, g.leave_time).C;

                for (auto [y, x] : max_empty_cells) grid[y][x] = 2;
                for (auto [y, x] : g.cells) {
                    grid[y][x] = 2;
                    leave_time_grid[y][x] = 0;
                }

                PlacementResult g_place = find_best_placement(g.P, g.leave_time, SearchMode::SUPER_FAST);

                if (!g_place.cells.empty()) {
                    for (auto [y, x] : max_empty_cells) grid[y][x] = 0;
                    for (auto [y, x] : g.cells) grid[y][x] = 0;
                    
                    for (auto [y, x] : g_place.cells) {
                        grid[y][x] = 1;
                        leave_time_grid[y][x] = g.leave_time;
                    }

                    PlacementResult i_place = find_best_placement(pi, ti, SearchMode::SUPER_FAST);

                    double loss_g_C = max(0.0, g_old_C - g_place.C);
                    double move_profit = (vi_val * i_place.C) - move_cost - (g.V * loss_g_C);

                    if (!i_place.cells.empty() && move_profit > direct_profit) {
                        for (auto [y, x] : g_place.cells) {
                            grid[y][x] = 0;
                            leave_time_grid[y][x] = 0;
                        }
                        for (auto [y, x] : max_empty_cells) grid[y][x] = 2;
                        for (auto [y, x] : g.cells) grid[y][x] = 2;

                        PlacementResult g_place_real = find_best_placement(g.P, g.leave_time, SearchMode::NORMAL);
                        
                        for (auto [y, x] : max_empty_cells) grid[y][x] = 0;
                        for (auto [y, x] : g.cells) grid[y][x] = 0;
                        for (auto [y, x] : g_place_real.cells) {
                            grid[y][x] = 1;
                            leave_time_grid[y][x] = g.leave_time;
                        }

                        PlacementResult i_place_real = find_best_placement_mc(pi, ti, si, SearchMode::NORMAL);

                        cout << 1 << "\n";
                        cout << g.id << "\n";
                        for (auto [y, x] : g_place_real.cells) {
                            cout << y - 1 << " " << x - 1 << "\n";
                        }
                        g.cells = g_place_real.cells;
                        pre_calculated_i_cells = i_place_real.cells;
                        moved = true;
                        break;
                    } else {
                        for (auto [y, x] : g_place.cells) {
                            grid[y][x] = 0;
                            leave_time_grid[y][x] = 0;
                        }
                        for (auto [y, x] : max_empty_cells) grid[y][x] = 0;
                        for (auto [y, x] : g.cells) {
                            grid[y][x] = 1;
                            leave_time_grid[y][x] = g.leave_time;
                        }
                    }
                } else {
                    for (auto [y, x] : max_empty_cells) grid[y][x] = 0;
                    for (auto [y, x] : g.cells) {
                        grid[y][x] = 1;
                        leave_time_grid[y][x] = g.leave_time;
                    }
                }
            }
        }

        if (!moved) {
            cout << 0 << "\n";
        }

        if (density < MIN_DENSITY) {
            cout << "No" << "\n";
            cout.flush();
            continue;
        }

        vpii best_cells;

        if (moved && !pre_calculated_i_cells.empty()) {
            best_cells = pre_calculated_i_cells;
        } else if (!direct_place.cells.empty()) {
            best_cells = direct_place.cells;
        } else {
            PlacementResult res = find_best_placement_mc(pi, ti, si, SearchMode::NORMAL);
            best_cells = res.cells;
        }

        if (best_cells.empty()) {
            cout << "No" << "\n";
        } else {
            cout << "Yes" << "\n";
            for (auto [y, x] : best_cells) {
                grid[y][x] = 1;
                leave_time_grid[y][x] = ti;
                cout << y - 1 << " " << x - 1 << "\n";
            }
            active_groups.push_back({cur_i, ti, pi, vi_val, best_cells, true});
        }

        cout.flush();
    }

    return 0;
}