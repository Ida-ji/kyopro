#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vb = vector<bool>;
using vvi = vector<vector<int>>;
using vvb = vector<vector<bool>>;
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

    inline double rand_double() { // [0.0, 1.0)
        return (rand() >> 11) * (1.0 / 9007199254740992.0);
    }
} rng;

// 幅優先探索 (BFS) で start から target までの最短パスを取得
vi get_bfs_path(int N, const vvi& g, int start, int target) {
    vi dist(N, -1);
    vi parent(N, -1);
    queue<int> q;
    
    dist[start] = 0;
    q.push(start);
    
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        if (u == target) break;
        
        for (int v : g[u]) {
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                parent[v] = u;
                q.push(v);
            }
        }
    }
    
    vi path;
    for (int curr = target; curr != -1; curr = parent[curr]) {
        path.push_back(curr);
    }
    reverse(path.begin(), path.end());
    return path;
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int N, M, T, La, Lb; cin >> N >> M >> T >> La >> Lb;
    vvi g(N);
    rep(i, 0, M) {
        int u, v; cin >> u >> v;
        g[u].push_back(v);
        g[v].push_back(u);
    }
    vi t(T);
    rep(i, 0, T) cin >> t[i];
    vvi pos(N);
    rep(i, 0, N) {
        int x, y; cin >> x >> y;
        pos[i] = {x, y};
    }

    // 0 からスタートして t[0], t[1]... と巡回するルートリストを作る
    vi targets;
    targets.push_back(0);
    for (int v : t) targets.push_back(v);

    // 巡回パスを事前に作成[cite: 2]
    vi full_path;
    rep(i, 0, (int)targets.size() - 1) {
        vi path = get_bfs_path(N, g, targets[i], targets[i+1]);
        for (size_t j = (i == 0 ? 0 : 1); j < path.size(); j++) {
            full_path.push_back(path[j]);
        }
    }

    // --- 前半：A を作成する ---
    vi A;
    vb in_A(N, false);

    // ステップ1: full_path の中で「まだ A に入っていない都市」を最優先で追加
    for (int v : full_path) {
        if ((int)A.size() < La && !in_A[v]) {
            A.push_back(v);
            in_A[v] = true;
        }
    }

    // ステップ2: まだ A に入っていない都市があれば残りの枠に追加（全都市の存在を保証）
    rep(i, 0, N) {
        if ((int)A.size() < La && !in_A[i]) {
            A.push_back(i);
            in_A[i] = true;
        }
    }

    // ステップ3: まだ A に空きがある場合、full_path の要素（重複あり）で埋める
    for (int v : full_path) {
        if ((int)A.size() < La) {
            A.push_back(v);
        } else {
            break;
        }
    }

    // 万が一 La に満たない場合は 0 で埋める
    while ((int)A.size() < La) {
        A.push_back(0);
    }

    // A の出力[cite: 2]
    rep(i, 0, La) {
        cout << A[i] << (i == La - 1 ? "" : " ");
    }
    cout << "\n";

    // --- 後半：移動シミュレーション ---
    vi B(Lb, -1);
    vb in_B(N, false);

    int init_len = min(La, Lb);
    cout << "s " << init_len << " 0 0\n";
    rep(i, 0, init_len) {
        B[i] = A[i];
        in_B[A[i]] = true;
    }

    int cur = 0; // 初期位置は 0[cite: 2]

    rep(i, 0, (int)targets.size() - 1) {
        int des = targets[i+1];
        while (cur != des) {
            vi path = get_bfs_path(N, g, cur, des);
            int next_v = path[1];

            if (!in_B[next_v]) {
                // next_v を持つ A 内のインデックスを探す（全都市が A に存在するため必ず見つかる）
                int target_pa = 0;
                rep(j, 0, La) {
                    if (A[j] == next_v) {
                        target_pa = j;
                        break;
                    }
                }

                int pa = min(target_pa, max(0, La - Lb));
                int len = min(Lb, La - pa);

                cout << "s " << len << " " << pa << " 0\n";

                fill(in_B.begin(), in_B.end(), false);
                rep(j, 0, len) {
                    B[j] = A[pa + j];
                    in_B[B[j]] = true;
                }
            }

            cout << "m " << next_v << "\n";
            cur = next_v;
        }
    }

    return 0;
}