#include <bits/stdc++.h>
using ll = long long;
using namespace std;

int N;
vector<int> dx = {-1, 0, 1, 0};
vector<int> dy = {0, -1, 0, 1};
// 2次元グリッドワーシャルフロイド法の状態数 (基本的にはN*N)
int numstates;
// dist_table[s_state][t_state]: 状態 s から 状態 t への最小操作文字数（コスト）
vector<vector<int>> disttable;

// next_op[s_state][t_state]: 状態 s から 状態 t へ向かうための「最初の基本操作（F, R, L）」
// L/R等の基本操作がある時に使う、それ以外では使わない
//基本操作がある時にはそれに応じて下も全て変える
vector<vector<char>> nextop;

// マス座標と向きから状態IDへ変換
int getstateid(int x, int y) {
    return (x * N + y);
}

// 状態IDからマスの座標と向きを復元
void fromstateid(int id, int &x, int &y) {
    y = id % N;
    x = id / N;
}

//2次元グリッドワーシャルフロイド法
void init_warshall_floyd() {
    numstates = N*N;
    const int INF = 1e9;
    disttable.assign(numstates, vector<int>(numstates, INF));

    //自己ループ初期化＆LRFによる1手遷移の初期化
    for (int i=0; i<numstates; i++) {
        disttable[i][i] = 0; //自己ループ
        int cx, cy, cdir;
        fromstateid(i, cx, cy);

        //次の点に向けて距離1とかを作る
        //disttable[i][next] = 1;

    }

    //3重for文
    //kは経由地
    for (int k = 0; k < numstates; k++) {
        for (int i = 0; i < numstates; i++) {
            for (int j = 0; j < numstates; j++) {
                if (disttable[i][k] < INF && disttable[k][j] < INF) {
                    if (disttable[i][k] + disttable[k][j] < disttable[i][j]) {
                        disttable[i][j] = disttable[i][k] + disttable[k][j];
                        nextop[i][j] = nextop[i][k]; // 最初の1歩を更新
                    }
                }
            }
        }
    }
}