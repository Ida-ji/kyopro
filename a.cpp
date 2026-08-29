#include <bits/stdc++.h>
using namespace std;

using vi = vector<int>;
using vvi = vector<vector<int>>;

#define rep(i, s, t) for (int i = s; i < t; i++)

int W, D, N;
vvi a;

void solve_day(int d) {
    vi h(N);
    int total_h = 0;

    // 1. 各予約に対して要求を満たす最小の高さ h_i を計算
    rep(i, 0, N) {
        // ceil(a[d][i] / W) の切り上げ整数計算
        h[i] = (a[d][i] + W - 1) / W;
        if (h[i] == 0) h[i] = 1; // 最低でも高さ1は確保
        total_h += h[i];
    }

    // 2. 合計高さが W(1000) を超えている場合、高さを削る
    while (total_h > W) {
        int best_idx = -1;
        int max_surplus = -1e9;

        // h_i > 1 の中で「余剰面積 (W * h_i - A)」が最大のものを探す
        rep(i, 0, N) {
            if (h[i] <= 1) continue; // 高さ1未満には削れない

            int surplus = W * h[i] - a[d][i];
            if (surplus > max_surplus) {
                max_surplus = surplus;
                best_idx = i;
            }
        }

        // これ以上削れるものがない場合は打ち切り
        if (best_idx == -1) break;

        // 高さを 1 削る
        h[best_idx]--;
        total_h--;
    }

    // 3. 座標の割り当てと出力
    int current_r = 0;
    rep(i, 0, N) {
        int r0 = current_r;
        int r1 = (i == N - 1) ? W : min(W, r0 + h[i]); // 最後の予約は W まで伸ばす
        if (r1 <= r0) r1 = r0 + 1; // 念のため幅0防止

        // [r0, c0, r1, c1) -> 横幅は 0 から W までフルで使う
        cout << r0 << " " << 0 << " " << r1 << " " << W << "\n";
        current_r = r1;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> W >> D >> N)) return 0;
    a.resize(D, vi(N));
    rep(i, 0, D) {
        rep(j, 0, N) cin >> a[i][j];
    }

    rep(d, 0, D) {
        solve_day(d);
    }

    return 0;
}