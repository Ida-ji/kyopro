#include <bits/stdc++.h>
using namespace std;
using ll = long long;

int N;
vector<vector<int>> g; // 隣接リスト: 各頂点に接続する隣接頂点
vector<vector<int>> uv_w; // 頂点u,vを結ぶ辺の重み

//上下左右
int dy[] = {-1, 1, 0, 0};
int dx[] = {0, 0, -1, 1};


//点の親頂点 (pathを復元するときなどには必要)
vector<vector<int>> parent(N, vector<int>(N));
//二次元グリッドでは：
//vector<vector<pair<int, int>>> parent(N, vector<pair<int, int>>(N, {-1, -1}));

//ダイクストラ法
//頂点start_vから全頂点への最短距離を計算するダイクストラ法
vector<int> makedist(int start_v) {
    int inf = 1e9;
    vector<int> dist(N, inf);
    // {コスト, 頂点} の最小ヒープ
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

    dist[start_v] = 0;
    pq.push({0, start_v});

    while (!pq.empty()) {
        auto [cs, u] = pq.top();
        pq.pop();
        
        if (dist[u] < cs) continue;
        //if (ny < 0 || ny >= N || nx < 0 || nx >= N) continue;
        //↓壁などがある場合
        /*
            if (dir == 0 && !hwall[cy - 1][cx]) continue; // 上
            if (dir == 1 && !hwall[cy][cx]) continue;     // 下
            if (dir == 2 && !vwall[cy][cx - 1]) continue; // 左
            if (dir == 3 && !vwall[cy][cx]) continue;     // 右
        */

        for (int v : g[u]) {
            int next_w = uv_w[u][v];
            if (dist[v] > dist[u] + next_w) {
                dist[v] = dist[u] + next_w;
                //parent[v] = u;
                //二次元グリッドでは：parent[ny][nx] = {y, x};など
                pq.push({dist[v], v});
            }
        }
    }
    //返すのは、頂点ごとにおける「頂点start_vからの最短距離」の配列
    return dist;
}

