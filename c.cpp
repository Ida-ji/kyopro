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

int main() {   
    //1.入力受付
    int N, M; cin >> N >> M;
    vi X(M); rep(i, 0, M) cin >> X[i];
    vector<ll> A(M); rep(i, 0, M) cin >> A[i];
    
    //2.idxを求める
    vector<ll> idx(N); //元々どこにあった石なのか
    int right = N-1;
    for (int i=M-1; i>=0; i--) {
        //rightが9でA[2]=4でX[2]=6なら、
        //ギリギリ大丈夫
        if (right-A[i]+1 < X[i]-1) {
            cout << -1 << endl;
            return 0;
        }
        //例えば、right=6, A[i]=2なら、
        //6, 5を見させたい
        //rightは4に更新
        for (int j=right; j>right-A[i]; j--) {
            idx[j] = i;
        }
        right -= A[i];
    }

    ll ans = 0;
    //3.手数を計算
    rep(i, 0, N) {
        ans += i-X[idx[i]]+1;
    }
    cout << ans << endl;
}