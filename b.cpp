#pragma GCC optimize("O3,unroll-loops")
#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using vi = vector<int>;
using vvi = vector<vector<int>>;

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
} rng;

struct beam {
    ll score;
    vvi order;
    vector<vector<ll>> a;

    // multiset 内でスコア昇順に並べるための比較演算子
    bool operator<(const beam &other) const {
        return score < other.score;
    }
};

int N, M, K;
const int MOD = 998244353;
vector<vector<ll>> inita;
vector<vvi> stamps;

ll scorecalc(const vector<vector<ll>> &A) {
    ll ret = 0;
    rep(i, 0, N) rep(j, 0, N) ret += A[i][j];
    return ret;
}

vi mpq() {
    return {rng.rand_int(M), rng.rand_int(N - 2), rng.rand_int(N - 2)};
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    cin >> N >> M >> K;
    inita.resize(N, vector<ll>(N, 0));
    rep(i, 0, N) rep(j, 0, N) cin >> inita[i][j];

    stamps.resize(M, vvi(3, vi(3, 0)));
    rep(i, 0, M) rep(j, 0, 3) rep(k, 0, 3) cin >> stamps[i][j][k];

    const int MAX_DEPTH = K;
    const int CHOKUDAI_WIDTH = 1;
    const size_t MAX_BEAM_SIZE = 30; // 各深さに保持する最大要素数（MLE防止）
    int width = 5;

    // 各深さの状態を multiset（優先度付き集合）で管理
    vector<multiset<beam>> beams(MAX_DEPTH + 1);

    beam initbeam;
    initbeam.score = scorecalc(inita);
    initbeam.order = {};
    initbeam.a = inita;
    beams[0].insert(initbeam);

    auto start = chrono::steady_clock::now();
    while (true) {
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(now - start).count() > 1900) break;

        rep(depth, 0, MAX_DEPTH) {
            if (beams[depth].empty()) continue;

            rep(w, 0, CHOKUDAI_WIDTH) {
                if (beams[depth].empty()) break;

                // 改善2：コピーなしで最高スコアの状態を取り出し
                auto node = beams[depth].extract(prev(beams[depth].end()));
                beam cb = move(node.value());

                rep(wid, 0, width) {
                    beam nb = cb;
                    vi nextorder = mpq();
                    int nm = nextorder[0], np = nextorder[1], nq = nextorder[2];

                    rep(i, 0, 3) rep(j, 0, 3) {
                        nb.a[np + i][nq + j] = (nb.a[np + i][nq + j] + stamps[nm][i][j]) % MOD;
                    }

                    nb.score = scorecalc(nb.a);
                    nb.order.push_back({nm, np, nq});

                    if (depth + 1 <= MAX_DEPTH) {
                        // 改善1：不要な挿入を事前に弾く
                        if (beams[depth + 1].size() >= MAX_BEAM_SIZE && nb.score <= beams[depth + 1].begin()->score) {
                            continue;
                        }

                        beams[depth + 1].insert(nb);
                        if (beams[depth + 1].size() > MAX_BEAM_SIZE) {
                            beams[depth + 1].erase(beams[depth + 1].begin());
                        }   
                    }
                }
            }
        }
    }

    // 全層の中で最もスコアが高い状態を出力
    ll best_score = -1;
    beam bestbeam;

    rep(d, 0, MAX_DEPTH + 1) {
        if (!beams[d].empty()) {
            beam top_beam = *prev(beams[d].end());
            if (top_beam.score > best_score) {
                best_score = top_beam.score;
                bestbeam = top_beam;
            }
        }
    }

    int L = bestbeam.order.size();
    cout << L << "\n";
    rep(k, 0, L) {
        cout << bestbeam.order[k][0] << " "
             << bestbeam.order[k][1] << " "
             << bestbeam.order[k][2] << "\n";
    }

    return 0;
}