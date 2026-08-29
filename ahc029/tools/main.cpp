//#pragma GCC optimize("O3,unroll-loops")

#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using vi = vector<int>;
using vpii = vector<pair<int, int>>;

#define rep(i, s, t) for (int i = s; i < t; i++)

struct FastRNG {
    uint64_t state = 88172645463325252ULL;

    inline uint64_t rand() {
        state += 0xa0761d6478bd642fULL;
        unsigned __int128 mum = (unsigned __int128)state * (state ^ 0xe7037ed1a0b428dbULL);
        return (uint64_t)mum ^ (uint64_t)(mum >> 64);
    }

    inline int rand_int(int mod) {
        if (mod <= 1) return 0;
        return (int)(((unsigned __int128)rand() * (uint64_t)mod) >> 64);
    }

    inline double rand_double() { // [0.0, 1.0) の実数乱数
        return (rand() >> 11) * (1.0 / 9007199254740992.0);
    }
} rng;

int N; // 手札の枚数
int M; // プロジェクト数
int K; // 補充カード候補数
int T; // ターン数 (1000)

vpii hand;    // {種類, 労働力}
vpii project; // {残務量 h, 価値 v}

struct replenish {
    int type;  // 種類
    ll w;      // 労働力
    ll cost;   // コスト
};

// 効率 (v / h) が一番「高い」プロジェクトのインデックスを求める (通常用)
int get_best_efficiency_project_idx(const vpii& p) {
    int max_idx = 0;
    double max_eff = -1.0;
    rep (i, 0, M) {
        double eff = (double)p[i].second / p[i].first;
        if (eff > max_eff) {
            max_eff = eff;
            max_idx = i;
        }
    }
    return max_idx;
}

// 効率 (v / h) が一番「低い」プロジェクトのインデックスを求める (キャンセル用)
int get_worst_efficiency_project_idx(const vpii& p) {
    int min_idx = 0;
    double min_eff = 1e18;
    rep (i, 0, M) {
        double eff = (double)p[i].second / p[i].first;
        if (eff < min_eff) {
            min_eff = eff;
            min_idx = i;
        }
    }
    return min_idx;
}

// 【終盤用】 h / (2^L) <= (残りターン数) を満たすプロジェクトの中で一番報酬(v)が高いものを選択
int get_late_game_target_project_idx(const vpii& p, int L, int rem_turns) {
    int best_idx = -1;
    ll max_val = -1;

    rep (i, 0, M) {
        // 条件: h / (2^L) <= rem_turns
        double scaled_h = (double)p[i].first / (1LL << L);
        if (scaled_h <= (double)rem_turns) {
            if (p[i].second > max_val) {
                max_val = p[i].second;
                best_idx = i;
            }
        }
    }

    // もし条件を満たすプロジェクトが1つも無ければ、一番残務量 h が少ない案件を選ぶ（削りきれる可能性が最も高いため）
    if (best_idx == -1) {
        int min_h = 2e9;
        rep (i, 0, M) {
            if (p[i].first < min_h) {
                min_h = p[i].first;
                best_idx = i;
            }
        }
    }

    return best_idx;
}

// 使用するカードと対象プロジェクトを出力し、その後のプロジェクト状況を受け取る
vpii use_interact(int c, int m) {
    cout << c << " " << m << endl;
    vpii ret(M);
    rep (i, 0, M) {
        cin >> ret[i].first >> ret[i].second;
    }
    return ret;
}

// 補充カードの情報の入力を受け取る
vector<replenish> reple_interact() {
    vector<replenish> ret(K);
    rep(i, 0, K) {
        cin >> ret[i].type >> ret[i].w >> ret[i].cost;
    }
    return ret;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const double CANCEL_THRESHOLD = 0.9;
    const int LATE_GAME_TURNS = 100; // 終盤モードに切り替える残りターン数

    // 1. 初期設定入力受取
    cin >> N >> M >> K >> T;
    
    hand.resize(N);
    rep(i, 0, N) {
        cin >> hand[i].first >> hand[i].second;
    }
    
    project.resize(M);
    rep(i, 0, M) {
        cin >> project[i].first >> project[i].second;
    }

    // 2. ターン開始
    ll money = 0;
    int current_L = 0; // 増資レベル管理 (0 ~ 20)
    
    rep (turn, 0, T) {
        int rem_turns = T - turn; // 残りターン数

        // --- [カード使用フェーズ] ---
        vi idx_type4, idx_type1, idx_type0, idx_type2, idx_type3;
        
        rep (i, 0, N) {
            if (hand[i].first == 4) idx_type4.push_back(i);      // 増資
            else if (hand[i].first == 1) idx_type1.push_back(i); // 全体労働
            else if (hand[i].first == 0) idx_type0.push_back(i); // 通常労働
            else if (hand[i].first == 2) idx_type2.push_back(i); // キャンセル
            else if (hand[i].first == 3) idx_type3.push_back(i); // 業務転換
        }

        int use_c = 0;
        int use_m = 0;

        if (rem_turns <= LATE_GAME_TURNS) {
            // ==========================================
            // 【終盤フェーズ】現金回収モード
            // ==========================================
            int target_m = get_late_game_target_project_idx(project, current_L, rem_turns);

            if (!idx_type1.empty()) {
                // 1. 全体労働を使う
                use_c = idx_type1[0];
                use_m = 0;
            } else if (!idx_type0.empty()) {
                // 2. 通常労働を使う (達成可能なターゲットプロジェクトに集中)
                use_c = idx_type0[0];
                use_m = target_m;
            } else {
                // 3. 労働カードが無ければ手札の0番目を使い捨て（ターゲット案件に打つ）
                use_c = 0;
                use_m = target_m;
            }

        } else {
            // ==========================================
            // 【通常フェーズ】エンジンビルド ＆ 盤面整理
            // ==========================================
            int worst_m = get_worst_efficiency_project_idx(project);
            double worst_eff = (double)project[worst_m].second / project[worst_m].first;

            if (!idx_type4.empty() && rng.rand_double() < 0.9) {
                // 1. 増資カード（最優先）
                use_c = idx_type4[0];
                use_m = 0;
            } else if (!idx_type2.empty() && worst_eff < CANCEL_THRESHOLD) {
                // 2. 効率悪案件があればキャンセル即実行
                use_c = idx_type2[0];
                use_m = worst_m;
            } else if (!idx_type1.empty() && rng.rand_double() < 0.9) {
                // 3. 全体労働
                use_c = idx_type1[0];
                use_m = 0;
            } else if (!idx_type0.empty() && rng.rand_double() < 0.9) {
                // 4. 通常労働 (一番効率 v/h が良いプロジェクトを選択)
                use_c = idx_type0[0];
                use_m = get_best_efficiency_project_idx(project);
            } else if (!idx_type2.empty()) {
                // 5. 通常キャンセル
                use_c = idx_type2[0];
                use_m = worst_m;
            } else if (!idx_type3.empty()) {
                // 6. 業務転換
                use_c = idx_type3[0];
                use_m = 0;
            } else {
                // フォールバック
                if (!idx_type0.empty()) {
                    use_c = idx_type0[0];
                    use_m = get_best_efficiency_project_idx(project);
                } else if (!idx_type1.empty()) {
                    use_c = idx_type1[0];
                    use_m = 0;
                } else {
                    use_c = 0;
                    use_m = get_best_efficiency_project_idx(project);
                }
            }
        }

        // 増資カードを使用した場合はレベルを記録
        if (hand[use_c].first == 4) {
            current_L = min(20, current_L + 1);
        }

        // カード使用のインタラクション
        project = use_interact(use_c, use_m);
        cin >> money;

        // --- [カード補充フェーズ] ---
        vector<replenish> reple = reple_interact();

        int choice_r = 0;
        double max_score = -1e18;

        rep(i, 0, K) {
            if (money < reple[i].cost) continue; // 所持金不足

            double score = 0;
            int type = reple[i].type;
            ll w = reple[i].w;
            ll cost = reple[i].cost;

            if (rem_turns <= LATE_GAME_TURNS) {
                // 【終盤の補充】労働カードのみ購入する
                if (type == 4 || type == 2 || type == 3) {
                    score = -1e9; // 投資や整理カードは一切買わない
                } else if (type == 1) {
                    // 全体労働
                    ll eff_w = 0;
                    rep(j, 0, M) eff_w += min(w, (ll)project[j].first);
                    score = 1000.0 + (double)eff_w / max(1LL, cost);
                } else if (type == 0) {
                    // 通常労働
                    score = 1000.0 + (double)w / max(1LL, cost);
                }
            } else {
                // 【通常の補充】
                if (type == 4) {
                    score = 1e9;
                } else if (type == 1) {
                    if (cost > money * 0.7 && money > 0) {
                        score = -1e8; 
                    } else {
                        ll eff_w = 0;
                        rep(j, 0, M) eff_w += min(w, (ll)project[j].first);
                        score = 1000.0 + (double)eff_w / max(1LL, cost);
                    }
                } else if (type == 0) {
                    int best_m = get_best_efficiency_project_idx(project);
                    ll eff_w = min(w, (ll)project[best_m].first);
                    score = 1000.0 + (double)eff_w / max(1LL, cost);
                } else if (type == 2) {
                    score = 1200.0 - cost; 
                } else if (type == 3) {
                    score = 300.0 - cost;
                }
            }

            if (score > max_score) {
                max_score = score;
                choice_r = i;
            }
        }

        // 補充カードの決定を出力
        cout << choice_r << endl;

        // 手札を更新
        hand[use_c] = {reple[choice_r].type, (int)reple[choice_r].w};
    }

    return 0;
}