#pragma GCC optimize("O3,unroll-loops")
#include <bits/stdc++.h>
using namespace std;
using pii = pair<int, int>;
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
} rng;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int N; cin >> N;
    vpii good_fish(N), bad_fish(N);
    rep(i, 0, N) cin >> good_fish[i].first >> good_fish[i].second;
    rep(i, 0, N) cin >> bad_fish[i].first >> bad_fish[i].second;

    // 長方形の境界（座標は 0 から 100000）
    int x1 = rng.rand_int(90000);
    int x2 = x1 + 1 + rng.rand_int(100000 - x1 - 1);
    int y1 = rng.rand_int(90000);
    int y2 = y1 + 1 + rng.rand_int(100000 - y1 - 1);

    // スコア計算関数（長方形内にいる魚の数を直接数える）
    auto calc_score = [&](int lx, int ly, int rx, int ry) {
        int score = 0;
        rep(i, 0, N) {
            if (good_fish[i].first >= lx && good_fish[i].first <= rx &&
                good_fish[i].second >= ly && good_fish[i].second <= ry) {
                score++;
            }
            if (bad_fish[i].first >= lx && bad_fish[i].first <= rx &&
                bad_fish[i].second >= ly && bad_fish[i].second <= ry) {
                score--;
            }
        }
        return score;
    };

    int current_score = calc_score(x1, y1, x2, y2);
    int finalscore = current_score;
    vpii finalnet(4);

    auto update_final = [&]() {
        finalnet[0] = {x1, y1};
        finalnet[1] = {x2, y1};
        finalnet[2] = {x2, y2};
        finalnet[3] = {x1, y2};
    };
    update_final();

    auto start = chrono::steady_clock::now();
    int iter = 0;

    // 山登り（近傍変形：長方形の境界を直接動かす）
    while (true) {
        if ((++iter & 1023) == 0) {
            auto now = chrono::steady_clock::now();
            if (chrono::duration_cast<chrono::milliseconds>(now - start).count() > 1900) break;
        }

        int nx1 = x1, ny1 = y1, nx2 = x2, ny2 = y2;
        int type = rng.rand_int(4);
        int delta = (rng.rand_int(2) == 0 ? 1 : -1) * (1 + rng.rand_int(50)); // 少し大きめに動かすと探索しやすい

        if (type == 0) nx1 += delta;
        else if (type == 1) nx2 += delta;
        else if (type == 2) ny1 += delta;
        else if (type == 3) ny2 += delta;

        // 範囲チェック (0 〜 100000)
        if (nx1 < 0 || nx2 > 100000 || ny1 < 0 || ny2 > 100000) continue;
        if (nx1 >= nx2 || ny1 >= ny2) continue;

        int perimeter = (nx2 - nx1 + ny2 - ny1) * 2;
        if (perimeter > 400000) continue;

        int next_score = calc_score(nx1, ny1, nx2, ny2);

        if (next_score >= current_score) {
            x1 = nx1; y1 = ny1; x2 = nx2; y2 = ny2;
            current_score = next_score;

            if (current_score > finalscore) {
                finalscore = current_score;
                update_final();
            }
        }
    }

    // 出力
    cout << 4 << "\n";
    rep(i, 0, 4) {
        cout << finalnet[i].first << " " << finalnet[i].second << "\n";
    }

    return 0;
}