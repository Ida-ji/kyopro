#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vvi = vector<vector<int>>;
using vb = vector<bool>;
using pii = pair<int, int>;

#define rep(i, s, t) for (int i = s; i < t; i++)

int judge(char a, char b) {
    if (a == 'G' && b == 'P') return -1;
    if (a == 'P' && b == 'C') return -1;
    if (a == 'C' && b == 'G') return -1;
    if (a == b) return 0;

    return 1;
}

int main() {
    int N, M; cin >> N >> M;
    //rank[i] = [x, y]
    //人yはx勝でi位
    vvi rank(2*N, vi(2)); 
    rep(i, 0, 2*N) {
        rank[i] = {0, i};
    }
    vector<vector<char>> janken(2*N, vector<char>(M));
    rep(i, 0, 2*N) {
        rep(j, 0, M) {
            cin >> janken[i][j];
        }
    }


    rep(i, 0, M) {
        rep(j, 0, N) {
            int player1 = rank[2*j][1];
            int player2 = rank[2*j+1][1];
            //勝負
            int result = judge(janken[player1][i], janken[player2][i]);
            //勝つほど数値は下がっていく
            if (result == 1) rank[2*j][0]--;
            else if (result == -1) rank[2*j+1][0]--;
        }
        sort(rank.begin(), rank.end());
    }
    rep(i, 0, 2*N) cout << rank[i][1]+1 << endl;
}