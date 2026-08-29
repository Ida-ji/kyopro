#include <bits/stdc++.h>
using ll = long long;
using namespace std;

int N;
int M;
double eps;

vector<int> di = {-1, 0, 1, 0};
vector<int> dj = {0, -1, 0, 1};

int excave(int i, int j) {
    cout << "q " << 1 << " " << i << " " << j << endl;
    int res; cin >> res;
    return res;
}

int oracle(vector<pair<int, int>> S) {
    cout << "q " << (int)S.size();
    for (auto [i, j] : S) {
        cout << " " << i << " " << j;
    }
    cout << endl;
    int res; cin >> res;
    return res;
}

double normal_cdf(double mean, double std_dev, double x) {
    return 0.5 * (1.0 + erf((x - mean) / (std_dev * sqrt(2.0))));
}

// 配置の対数尤度を計算する関数
double calc_total_log_likelihood(const vector<vector<int>> &grid, 
                                 const vector<vector<vector<bool>>> &o, 
                                 const vector<vector<pair<int, int>>> &S, 
                                 const vector<int> &s, int query_num) {
    static const double LOG_2PI = log(2.0 * M_PI);
    static const double OUTLIER_WEIGHT = 0.02;

    double total_log_prob = 0.0;
    for (int n = 0; n < query_num; n++) {
        int est = 0;
        for (auto [i, j] : S[n]) {
            est += grid[i][j];
        }

        if (s[n] == 0 && est > 0) {
            total_log_prob -= 100.0 * est; 
            continue;
        }
        
        int k = (int)S[n].size();
        double mu = (k - est) * eps + est * (1 - eps);
        double var = k * eps * (1 - eps);
        if (var < 1e-6) var = 1e-6;

        double diff = (double)s[n] - mu;
        double log_p_gauss = -0.5 * LOG_2PI - 0.5 * log(var) - (diff * diff) / (2.0 * var);
        double p_gauss = exp(log_p_gauss);
        double p_outlier = 1.0 / (double)(k + 1);

        double mix = (1.0 - OUTLIER_WEIGHT) * p_gauss + OUTLIER_WEIGHT * p_outlier;
        total_log_prob += log(max(mix, 1e-300));
    }
    return total_log_prob;
}

random_device rd;
mt19937 gen(rd());

int main() {
    cin >> N >> M >> eps;
    vector<vector<pair<int, int>>> d(M, vector<pair<int, int>>(0));
    for (int idx=0; idx<M; idx++) {
        int di_num; cin >> di_num;
        for (int jdx=0; jdx<di_num; jdx++) {
            int i, j; cin >> i >> j;
            d[idx].push_back({i, j});
        }
    }

    // -------------------------------------------------------------
    // 2. 占い（クエリ）パート
    // -------------------------------------------------------------
    int QUERY_NUM = 2*N*N * 0.55; 
    vector<vector<vector<bool>>> o(QUERY_NUM, vector<vector<bool>>(N, vector<bool>(N, 0)));
    vector<vector<pair<int, int>>> S(QUERY_NUM, vector<pair<int, int>>(0)); 
    vector<int> s(QUERY_NUM); 
    
    for (int n=0; n<QUERY_NUM; n++) {
        int W, H;
        bool preferLarge = (n < QUERY_NUM / 2);
        while (true) {
            W = uniform_int_distribution<int>(1, min(N, 64))(gen);
            H = uniform_int_distribution<int>(1, min(N, 64))(gen);
            if (W * H <= 64) {
                if (preferLarge && W * H < 25) continue;
                break;
            }
        }
        int i1 = uniform_int_distribution<int>(0, N - W)(gen);
        int j1 = uniform_int_distribution<int>(0, N - H)(gen);
        int i2 = i1 + W - 1;
        int j2 = j1 + H - 1;

        for (int i = i1; i <= i2; i++) {
            for (int j = j1; j <= j2; j++) {
                o[n][i][j] = 1;
                S[n].push_back({i, j});
            }
        }
        s[n] = oracle(S[n]);
    }

    auto rebuild_grid = [&](const vector<int> &si, const vector<int> &sj) {
        vector<vector<int>> g(N, vector<int>(N, 0));
        for (int idx = 0; idx < M; idx++) {
            for (auto [pi, pj] : d[idx]) {
                g[pi + si[idx]][pj + sj[idx]]++;
            }
        }
        return g;
    };

    // -------------------------------------------------------------
    // 3. 焼きなまし法 パート
    // -------------------------------------------------------------
    vector<int> current_shift_i(M, 0), current_shift_j(M, 0);
    for (int idx = 0; idx < M; idx++) {
        while (true) {
            int si = uniform_int_distribution<int>(-(N-1), N-1)(gen);
            int sj = uniform_int_distribution<int>(-(N-1), N-1)(gen);
            bool ok = true;
            for (auto [pi, pj] : d[idx]) {
                if (pi + si < 0 || pi + si >= N || pj + sj < 0 || pj + sj >= N) ok = false;
            }
            if (ok) {
                current_shift_i[idx] = si;
                current_shift_j[idx] = sj;
                break;
            }
        }
    }
    vector<vector<int>> current_grid = rebuild_grid(current_shift_i, current_shift_j);
    double current_log_prob = calc_total_log_likelihood(current_grid, o, S, s, QUERY_NUM);

    double final_log_prob = current_log_prob; 
    vector<vector<int>> final_grid = current_grid; 

    double start_temp = 30.0;
    double end_temp = 0.1;

    auto start = chrono::steady_clock::now();
    while (true) {
        auto now = chrono::steady_clock::now();
        double elapsed_time = chrono::duration_cast<chrono::milliseconds>(now - start).count();
        if (elapsed_time > 2850) break;

        double progress = elapsed_time / 2850.0;
        double temp = start_temp + (end_temp - start_temp) * progress;

        vector<int> next_shifts_i = current_shift_i;
        vector<int> next_shifts_j = current_shift_j;
        bool ok = true;
        int move_type_roll = uniform_int_distribution<int>(0, 99)(gen);

        if (move_type_roll < 40) {
            int target_idx = uniform_int_distribution<int>(0, M - 1)(gen);
            int move_dir = uniform_int_distribution<int>(0, 3)(gen);
            int next_si = current_shift_i[target_idx] + di[move_dir];
            int next_sj = current_shift_j[target_idx] + dj[move_dir];
            for (auto [pi, pj] : d[target_idx]) {
                int ni = pi + next_si; int nj = pj + next_sj;
                if (ni < 0 || ni >= N || nj < 0 || nj >= N) { ok = false; break; }
            }
            if (ok) { next_shifts_i[target_idx] = next_si; next_shifts_j[target_idx] = next_sj; }
        } else if (move_type_roll < 70) {
            int target_idx = uniform_int_distribution<int>(0, M - 1)(gen);
            int next_si = 0, next_sj = 0; bool found = false;
            for (int tryCount = 0; tryCount < 20; tryCount++) {
                int cand_si = uniform_int_distribution<int>(-(N - 1), N - 1)(gen);
                int cand_sj = uniform_int_distribution<int>(-(N - 1), N - 1)(gen);
                bool valid = true;
                for (auto [pi, pj] : d[target_idx]) {
                    int ni = pi + cand_si; int nj = pj + cand_sj;
                    if (ni < 0 || ni >= N || nj < 0 || nj >= N) { valid = false; break; }
                }
                if (valid) { next_si = cand_si; next_sj = cand_sj; found = true; break; }
            }
            if (found) { next_shifts_i[target_idx] = next_si; next_shifts_j[target_idx] = next_sj; } else { ok = false; }
        } else {
            if (M >= 2) {
                int idx1 = uniform_int_distribution<int>(0, M - 1)(gen);
                int idx2 = uniform_int_distribution<int>(0, M - 1)(gen);
                if (idx1 == idx2) { ok = false; } else {
                    int si1 = current_shift_i[idx2], sj1 = current_shift_j[idx2];
                    int si2 = current_shift_i[idx1], sj2 = current_shift_j[idx1];
                    bool valid = true;
                    for (auto [pi, pj] : d[idx1]) {
                        int ni = pi + si1, nj = pj + sj1;
                        if (ni < 0 || ni >= N || nj < 0 || nj >= N) { valid = false; break; }
                    }
                    if (valid) {
                        for (auto [pi, pj] : d[idx2]) {
                            int ni = pi + si2, nj = pj + sj2;
                            if (ni < 0 || ni >= N || nj < 0 || nj >= N) { valid = false; break; }
                        }
                    }
                    if (valid) { next_shifts_i[idx1] = si1; next_shifts_j[idx1] = sj1; next_shifts_i[idx2] = si2; next_shifts_j[idx2] = sj2; } else { ok = false; }
                }
            } else { ok = false; }
        }

        if (!ok) continue;

        vector<vector<int>> next_grid = rebuild_grid(next_shifts_i, next_shifts_j);
        double next_log_prob = calc_total_log_likelihood(next_grid, o, S, s, QUERY_NUM);
        double delta = next_log_prob - current_log_prob;

        if (delta > 0 || uniform_real_distribution<double>(0.0, 1.0)(gen) < exp(delta / temp)) {
            current_log_prob = next_log_prob; current_shift_i = next_shifts_i; current_shift_j = next_shifts_j; current_grid = next_grid;
            if (current_log_prob > final_log_prob) { final_log_prob = current_log_prob; final_grid = current_grid; }
        }
    }

    // -------------------------------------------------------------
    // 4. 発掘・探索パート
    // -------------------------------------------------------------
    queue<pair<int, int>> Q;
    vector<vector<bool>> queued(N, vector<bool>(N, false));
    vector<pair<int, int>> confirmed_oil; // 実際に掘って >0 だった確定マス
    int remaining_queries = 2 * N * N - QUERY_NUM - 10; // クエリ安全ストッパー（回答2回分も考慮）

    // ① SAの予測マスをすべて初期キューへ登録
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (final_grid[i][j] > 0) {
                if (!queued[i][j]) {
                    queued[i][j] = true;
                    Q.push({i, j});
                }
            }
        }
    }

    // ② 予測ベースの動的BFSループ
    while (!Q.empty() && remaining_queries > 0) {
        auto [i, j] = Q.front();
        Q.pop();

        int real_val = excave(i, j);
        remaining_queries--;

        if (real_val > 0) {
            confirmed_oil.push_back({i, j});
            for (int dir = 0; dir < 4; dir++) {
                int ni = i + di[dir];
                int nj = j + dj[dir];
                if (ni >= 0 && ni < N && nj >= 0 && nj < N) {
                    if (!queued[ni][nj]) {
                        queued[ni][nj] = true;
                        Q.push({ni, nj});
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------
    // 5. 最初の回答（1回目）
    // -------------------------------------------------------------
    cout << "a " << (int)confirmed_oil.size() << " ";
    for (auto [i, j] : confirmed_oil) { cout << i << " " << j << " "; }
    cout << endl;

    int r; cin >> r;
    if (r == 1) return 0; // 見事的中したら正常終了

    // -------------------------------------------------------------
    // 6. 外れた場合の保険：市松模様スキャンフェーズ
    // -------------------------------------------------------------
    // 面積による打ち切りを撤廃し、残クエリ全力で隠れた油田を探す
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (remaining_queries <= 0) break;

            // まだ未探索かつ市松模様のマス
            if (!queued[i][j] && (i + j) % 2 == 0) {
                queued[i][j] = true;
                int real_val = excave(i, j);
                remaining_queries--;

                if (real_val > 0) {
                    confirmed_oil.push_back({i, j});

                    // 新たな油田を検知したので、ローカルにBFSを展開して芋づる式に全回収
                    queue<pair<int, int>> subQ;
                    subQ.push({i, j});
                    while (!subQ.empty() && remaining_queries > 0) {
                        auto [ci, cj] = subQ.front();
                        subQ.pop();

                        for (int dir = 0; dir < 4; dir++) {
                            int ni = ci + di[dir];
                            int nj = cj + dj[dir];
                            if (ni >= 0 && ni < N && nj >= 0 && nj < N && !queued[ni][nj]) {
                                queued[ni][nj] = true;
                                int r_val = excave(ni, nj);
                                remaining_queries--;

                                if (r_val > 0) {
                                    confirmed_oil.push_back({ni, nj});
                                    subQ.push({ni, nj});
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------
    // 7. 最終回答（2回目）
    // -------------------------------------------------------------
    cout << "a " << (int)confirmed_oil.size() << " ";
    for (auto [i, j] : confirmed_oil) { cout << i << " " << j << " "; }
    cout << endl;

    cin >> r;
    return 0;
}