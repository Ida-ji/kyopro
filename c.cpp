#include <bits/stdc++.h>
using namespace std;

struct Target {
    int id;
    long long x, y;
};

struct Operation {
    long long x1, y1, x2, y2;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    if (!(cin >> N)) return 0;

    vector<Target> targets(N);
    for (int i = 0; i < N; i++) {
        targets[i].id = i;
        cin >> targets[i].x >> targets[i].y;
    }

    // 原点に近い順 (x + y が小さい順) にソート
    sort(targets.begin(), targets.end(), [](const Target& a, const Target& b) {
        return (a.x + a.y) < (b.x + b.y);
    });

    // 既に作成された点のリスト（初期値は原点）
    vector<pair<long long, long long>> created;
    created.push_back({0, 0});

    vector<Operation> ops;

    for (int i = 0; i < N; i++) {
        long long tx = targets[i].x;
        long long ty = targets[i].y;

        // created の中から cx <= tx && cy <= ty を満たし、最も距離が近い点を探す
        int best_idx = -1;
        long long min_dist = 4e18; // 十分大きい値で初期化

        for (int j = 0; j < (int)created.size(); j++) {
            auto [cx, cy] = created[j];
            if (cx <= tx && cy <= ty) {
                long long total_dist = (tx - cx) + (ty - cy);
                if (total_dist < min_dist) {
                    min_dist = total_dist;
                    best_idx = j;
                }
            }
        }

        auto [bx, by] = created[best_idx];

        // L字型に伸ばす: (bx, by) -> (tx, by) -> (tx, ty)
        // 1. x方向に伸ばす
        if (bx != tx) {
            ops.push_back({bx, by, tx, by});
            created.push_back({tx, by});
        }
        // 2. y方向に伸ばす
        if (by != ty) {
            ops.push_back({tx, by, tx, ty});
            created.push_back({tx, ty});
        }
    }

    // 出力
    cout << ops.size() << "\n";
    for (const auto& op : ops) {
        cout << op.x1 << " " << op.y1 << " " << op.x2 << " " << op.y2 << "\n";
    }

    return 0;
}