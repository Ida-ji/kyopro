#include <bits/stdc++.h>
using ll = long long;
using namespace std;

//UnionFind (準備)
struct UnionFind {
    vector<int> par; //par[i] はiの親の番号

    UnionFind(int n) : par(n, -1) {} //最初はみんなリーダー（-1）

    //リーダー（根）を探す
    int find(int x) {
        if (par[x] < 0) return x;
        return par[x] = find(par[x]); //経路圧縮：親を直接リーダーに繋ぎ変える
    }

    //AとBを合体させる
    bool unite(int x, int y) {
        x = find(x); y = find(y);
        if (x == y) return false; //既に同じグループ
        //小さい方の木を、大きい方の木にくっつける
        if (par[x] > par[y]) swap(x, y);
        par[x] += par[y];
        par[y] = x;
        return true;
    }

    //AとBが同じグループか判定
    bool same(int x, int y) {
        return find(x) == find(y);
    }

    int size(int x) {
        return -par[find(x)]; // xが含まれるグループのサイズを返す
    }
};

// 辺の情報を表す構造体（ソート用）
struct Edge {
    int u, v, w, id;
    // 重み w の昇順でソートできるようにする
    bool operator<(const Edge& max_e) const {
        return w < max_e.w;
    }
};

int M; //辺の本数
int N; //頂点の個数
vector<bool> ison; //1ならON、0ならOFF
vector<Edge> edges; // 辺のリスト（クラスカル法用）

int main() {
    //cin >> M;
    edges.resize(M);

    for (int i = 0; i < M; i++) {
        int u, v, w; cin >> u >> v >> w;
        u--; v--; // 1-indexed から 0-indexed に変換
        edges[i] = {u, v, w, i};
    }

    //クラスカル法による最小全域木(MST)の構成
    ison.assign(M, false); // サイズMで初期化
    
    // 辺を重みの軽い順にソート
    //(重みを考慮しない場合はsortをしないだけでよい)
    sort(edges.begin(), edges.end());
    
    UnionFind uf(N);
    int edge_count = 0;
    for (int i = 0; i < M; i++) {
        // 重みが軽い辺から順に、閉路を作らないなら採用する
        if (uf.unite(edges[i].u, edges[i].v)) { //もしuniteが正常に出来るなら元々連結でない
            ison[edges[i].id] = true; // 元の辺番号をONにする
            edge_count++;
            if (edge_count == N - 1) break; // 木が完成したら終了
        }
    }
}