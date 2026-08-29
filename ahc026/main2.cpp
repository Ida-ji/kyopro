#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vvi = vector<vector<int>>;
using vpii = vector<pair<int, int>>;

#define rep(i, s, t) for (int i = s; i < t; i++)

int n, m;
vvi initstacks;
vpii initboxidx;

struct beam {
    vvi stacks;
    vpii output;
    vpii boxidx;
    int cost;

    // cost が小さい方が優先度が高くなるように設定 (Chokudaiサーチ用)
    bool operator>(const beam &other) const {
        return cost > other.cost;
    }
};

// 簡易タイマー
struct Timer {
    chrono::system_clock::time_point start;
    Timer() { start = chrono::system_clock::now(); }
    double get_sec() {
        auto end = chrono::system_clock::now();
        return chrono::duration_cast<chrono::milliseconds>(end - start).count() / 1000.0;
    }
} timer;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n >> m;
    initstacks.resize(m, vi(0));
    initboxidx.resize(n);
    rep(i, 0, m) {
        rep(j, 0, n / m) {
            int b; cin >> b; b--;
            initstacks[i].push_back(b);
            initboxidx[b] = {i, j};
        }
    }

    // 各深さ (0 ~ N) ごとの優先度付きキュー (小さい cost が top に来る)
    vector<priority_queue<beam, vector<beam>, greater<beam>>> queues(n + 1);

    // 初期状態を深さ 0 に入れる
    queues[0].push({initstacks, {}, initboxidx, 0});

    const double TIME_LIMIT = 1.90; // 制限時間（秒）
    const int CHOKUDAI_WIDTH = 1;   // 1周あたり各深さから展開する状態数（基本は1でOK）

    // 時間制限までループ
    while (timer.get_sec() < TIME_LIMIT) {
        for (int t = 0; t < n; ++t) {
            // その深さに探索待ちの状態がなければスキップ
            if (queues[t].empty()) continue;

            // 各深さから CHOKUDAI_WIDTH 個ぶん取り出して展開
            for (int w = 0; w < CHOKUDAI_WIDTH; ++w) {
                if (queues[t].empty()) break;

                beam cb = queues[t].top();
                queues[t].pop();

                int idx = cb.boxidx[t].first;
                int height = cb.boxidx[t].second;

                int idx1 = (t != n - 1) ? cb.boxidx[t + 1].first : -1;
                int idx2 = (t < n - 2) ? cb.boxidx[t + 2].first : -1;

                // --- 遷移処理（提示コードの遷移ロジックをそのまま使用） ---
                if (height != (int)cb.stacks[idx].size() - 1) {
                    int outbox = cb.stacks[idx][height + 1];
                    rep(nextidx, 0, m) {
                        if (nextidx == idx) continue;
                        if (t != n - 1 && nextidx == idx1) continue;
                        if (t < n - 2 && nextidx == idx2) continue;

                        beam nb = cb;
                        nb.output.push_back({outbox + 1, nextidx + 1});

                        int nextidxsize = (int)nb.stacks[nextidx].size();
                        rep(j, height + 1, (int)nb.stacks[idx].size()) {
                            int movebox = nb.stacks[idx][j];
                            nb.stacks[nextidx].push_back(movebox);
                            nb.boxidx[movebox] = {nextidx, (nextidxsize + (j - height - 1))};
                        }
                        nb.stacks[idx].resize(height + 1);

                        nb.output.push_back({t + 1, 0});
                        nb.stacks[idx].pop_back();

                        nb.cost += (int)cb.stacks[idx].size() - height;

                        // 次の深さ t + 1 のキューに追加
                        queues[t + 1].push(nb);
                    }
                } else {
                    beam nb = cb;
                    nb.output.push_back({t + 1, 0});
                    nb.stacks[idx].pop_back();
                    nb.cost++;

                    // 次の深さ t + 1 のキューに追加
                    queues[t + 1].push(nb);
                }
            }
        }
    }

    // 最終深さ queues[n] の中で最もコストが低いものを取得
    if (!queues[n].empty()) {
        beam bestbeam = queues[n].top();
        for (auto p : bestbeam.output) {
            cout << p.first << " " << p.second << "\n";
        }
    }

    return 0;
}