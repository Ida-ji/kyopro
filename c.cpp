#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vvi = vector<vector<int>>;
using vb = vector<bool>;

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

    inline int rand_int(int mod) {
        if (mod <= 1) return 0;
        return (int)(((unsigned __int128)operator()() * (uint64_t)mod) >> 64);
    }
} rng;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // 1. 入力受付
    int N, M, H; cin >> N >> M >> H;
    vi A(N); for (int i = 0; i < N; i++) cin >> A[i];
    vvi g(N);
    for (int i = 0; i < M; i++) {
        int u, v; cin >> u >> v;
        g[u].push_back(v);
        g[v].push_back(u);
    }
    vvi xy(N, vi(2));
    for (int i = 0; i < N; i++) cin >> xy[i][0] >> xy[i][1];

    // 2. 初期化
    vi p(N, -1);
    vb used(N, false);

    // 作成したい木の最大頂点数（高さ H を超えないように制御）
    // 一直線に繋ぐ場合、高さは K - 1 になるので K <= H + 1 に抑えるのが安全です
    int TARGET_K = 8;

    // 3. ループ処理
    auto start = chrono::steady_clock::now();
    while (true) {
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(now - start).count() > 1900) break;

        // 4. ランダムに起点を選ぶ
        int r = rng.rand_int(N);
        if (used[r]) continue;

        vi point_idx;
        point_idx.push_back(r);
        used[r] = true;

        // 5. K 個まで一本道で伸ばす
        for (int step = 0; step < TARGET_K - 1; step++) {
            int last_node = point_idx.back();
            int next_node = -1;

            for (auto cand : g[last_node]) {
                if (used[cand]) continue;
                // 直前に追加した頂点へ戻るのを防ぐ
                if (point_idx.size() >= 2 && cand == point_idx[point_idx.size() - 2]) continue;
                if (used[cand]) continue;
                used[cand] = true;
                next_node = cand;
                break;
            }

            if (next_node == -1) break; // 途中で行き止まりになったらそこで終了
            point_idx.push_back(next_node);
        }

        // 最低でも2点以上集まらなかったらスキップ
        if (point_idx.size() < 2) continue;

        int sz = point_idx.size();

        // 6. 選ばれた頂点の中で、A[v] が最小となる「根」の位置を探す
        int min_root_pos = 0;
        for (int i = 1; i < sz; i++) {
            if (A[point_idx[i]] < A[point_idx[min_root_pos]]) {
                min_root_pos = i;
            }
        }

        // 7. 根から左右に向かって親を指定する (根に向かって辺を張る)
        // 根より左側 (0 〜 min_root_pos - 1) は右の頂点を親にする
        for (int i = 0; i < min_root_pos; i++) {
            p[point_idx[i]] = point_idx[i + 1];
        }
        // 根より右側 (min_root_pos + 1 〜 sz - 1) は左の頂点を親にする
        for (int i = min_root_pos + 1; i < sz; i++) {
            p[point_idx[i]] = point_idx[i - 1];
        }
        // min_root_pos の頂点は根なので p は -1 のまま（変更なし）
    }

    // 出力
    for (int i = 0; i < N; i++) {
        cout << p[i] << (i == N - 1 ? "" : " ");
    }
    cout << "\n";

    return 0;
}