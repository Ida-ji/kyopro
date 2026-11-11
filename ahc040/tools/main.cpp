#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vb = vector<bool>;
using vvi = vector<vector<int>>;
using vvb = vector<vector<bool>>;
using vpii = vector<pair<int, int>>;

#define rep(i, s, t) for (int i = s; i < t; i++)

struct ans {
    int p;
    int r;
    char d;
    int b;
};

struct FastRNG {
    using result_type = uint64_t;
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return UINT64_MAX; }

    uint64_t state = 88172645463325252ULL;

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

    inline double rand_double() { // [0.0, 1.0)
        return (rand() >> 11) * (1.0 / 9007199254740992.0);
    }
} rng;

int main() {
    // 1. 入力受付
    int N, T, sigma;
    if (!(cin >> N >> T >> sigma)) return 0;

    vpii wh(N);
    rep(i, 0, N) {
        int w, h; cin >> w >> h;
        wh[i] = {w, h};
    }

    // 前ターンのジャッジ結果を保持しておく変数（初回の比較用）
    int prev_w_ret = -1;
    int prev_h_ret = -1;

    // 2. Tターン繰り返す
    rep(t, 0, T) {
        int anssz = N; // 配置長方形の個数はN固定
        vector<ans> ans_t(N);
        int i_h_lastturn = 0; //直前の段で「高さ」が大きい方の長方形のインデックス
        int i_h_thisturn = 0; //今回

        // このターンでの各段（ペア）の最大幅や合計幅の目安を計算するための一時変数
        int max_pair_width = 0;

        // 3. N/2 ペアについて回転状態を選択＆配置決定
        rep(i, 0, N / 2) {
            int idx1 = 2 * i;
            int idx2 = 2 * i + 1;

            int best_WH = 2e9;
            bool is_i1_rotate = false;
            bool is_i2_rotate = false;
            int current_pair_w = 0;

            // 4. 2*i と 2*i+1 で4通りの回転をシミュレート
            rep(simu, 0, 4) {
                int r1 = simu / 2;
                int r2 = simu % 2;

                int w1 = (r1 == 0 ? wh[idx1].first : wh[idx1].second);
                int h1 = (r1 == 0 ? wh[idx1].second : wh[idx1].first);

                int w2 = (r2 == 0 ? wh[idx2].first : wh[idx2].second);
                int h2 = (r2 == 0 ? wh[idx2].second : wh[idx2].first);

                // 横並びに連結した際の外接長方形の幅Wと高さH
                int W = w1 + w2;
                int H = max(h1, h2);

                if (W + H < best_WH) {
                    best_WH = W + H;
                    is_i1_rotate = (r1 == 1);
                    is_i2_rotate = (r2 == 1);
                    current_pair_w = W;
                    //次に使う
                    if (h1 > h2 + 8000) i_h_thisturn = idx1;
                    else if (h2 > h1 + 8000) i_h_thisturn = idx2;
                    else {
                        //乱数で
                        if (rng.rand_int(2) == 0) {
                            i_h_thisturn = idx1;
                        }
                        else i_h_thisturn = idx2;
                    }
                }
            }
            max_pair_width = max(max_pair_width, current_pair_w);

            // 5. 決定した回転状態をもとに操作命令を組み立てる
            if (i == 0) {
                ans_t[idx1] = {idx1, is_i1_rotate ? 1 : 0, 'U', -1};
            } else {
                if (i_h_lastturn == 2*(i-1)) {
                    ans_t[idx1] = {idx1, is_i1_rotate ? 1 : 0, 'L', i_h_lastturn};
                }
                else {
                    ans_t[idx1] = {idx1, is_i1_rotate ? 1 : 0, 'L', i_h_lastturn};
                }
            }

            if (i == 0) {
                ans_t[idx2] = {idx2, is_i2_rotate ? 1 : 0, 'L', -1};
            }
            else {
                ans_t[idx2] = {idx2, is_i2_rotate ? 1 : 0, 'L', i_h_lastturn};
            }
            //上限インデックスを更新
            i_h_lastturn = i_h_thisturn;
        }

        // Nが奇数の場合の端数処理
        if (N % 2 != 0) {
            int idx = N - 1;
            ans_t[idx] = {idx, 0, 'U', 2 * (N / 2 - 1)};
        }

        // 6. 出力する
        cout << anssz << "\n";
        rep(i, 0, N) {
            cout << ans_t[i].p << " "
                 << ans_t[i].r << " "
                 << ans_t[i].d << " "
                 << ans_t[i].b << "\n";
        }
        cout.flush();

        // 7. ジャッジからのフィードバックを受け取る
        int w_ret, h_ret;
        cin >> w_ret >> h_ret;


        // 次回比較用に保存
        prev_w_ret = w_ret;
        prev_h_ret = h_ret;
    }
    return 0;
}