#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vb = vector<bool>;
using pii = pair<int, int>;

#define rep(i, s, t) for (int i = s; i < t; i++)

struct ans {
    int x; int y; int x2; int y2;
};

int main () {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // 1. 入力受付
    int N; cin >> N;
    vi A(N), B(N);
    set<pair<int, int>> BA; // (y, x) の順で管理
    rep(i, 0, N) {
        cin >> A[i] >> B[i];
        BA.insert({B[i], A[i]});
    }    

    // 2. 各xとyの座標をソート・重複削除
    vi sorted_A = A, sorted_B = B;
    sort(sorted_A.begin(), sorted_A.end());
    sorted_A.erase(unique(sorted_A.begin(), sorted_A.end()), sorted_A.end());
    sort(sorted_B.begin(), sorted_B.end());
    sorted_B.erase(unique(sorted_B.begin(), sorted_B.end()), sorted_B.end());

    int asz = (int)sorted_A.size();
    int bsz = (int)sorted_B.size();
    
    // group も 2次元配列ではなく set で管理
    set<pair<int, int>> group; 
    rep(i, 0, bsz) {
        int cy = sorted_B[i];
        int max_ai = -1;
        
        // B[i] 行にある作りたい点のうち、最大の x を探す
        for (int x : sorted_A) {
            if (BA.count({cy, x})) {
                max_ai = max(max_ai, x);
            }
        }
        if (max_ai == -1) continue;

        // max_ai 以下の A[j] について (cy, A[j]) を group に追加
        rep(j, 0, asz) {
            if (sorted_A[j] > max_ai) break;
            group.insert({cy, sorted_A[j]});
        }
    }

    // 3. queueに (0, 0) を入れる
    queue<pair<int, int>> q;
    q.push({0, 0});
    set<pair<int, int>> made; // 作成済み座標 (y, x) を set で管理
    made.insert({0, 0});

    int cnt = 0;
    if (BA.count({0, 0})) cnt++;

    int cnt_all = 0; // 操作全体の回数
    vector<ans> ansarray;

    while (!q.empty()) {
        auto [cy, cx] = q.front();
        q.pop();

        // 5. x方向で次の nx を見る
        auto nx_it = upper_bound(sorted_A.begin(), sorted_A.end(), cx);
        if (nx_it != sorted_A.end()) { // イテレータの範囲外チェック
            int nx = *nx_it;
            // group に含まれており、かつ未作成の場合のみ処理
            if (group.count({cy, nx}) && !made.count({cy, nx})) {
                made.insert({cy, nx});
                q.push({cy, nx});
                cnt_all++;
                if (BA.count({cy, nx})) cnt++;
                ansarray.push_back({cx, cy, nx, cy});
                if (cnt == N) break;
            }
        }

        // 6. y方向で次の ny を見る
        auto ny_it = upper_bound(sorted_B.begin(), sorted_B.end(), cy);
        if (ny_it != sorted_B.end()) { // イテレータの範囲外チェック
            int ny = *ny_it;
            // group に含まれており、かつ未作成の場合のみ処理
            if (group.count({ny, cx}) && !made.count({ny, cx})) {
                made.insert({ny, cx});
                q.push({ny, cx});
                cnt_all++;
                if (BA.count({ny, cx})) cnt++;
                ansarray.push_back({cx, cy, cx, ny});
                if (cnt == N) break;
            }
        }
    }

    // 7. 出力
    cout << cnt_all << "\n";
    for (const auto& a : ansarray) {
        cout << a.x << " " << a.y << " " << a.x2 << " " << a.y2 << "\n";
    }

    return 0;
}