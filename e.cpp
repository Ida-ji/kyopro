#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vvi = vector<vector<int>>;
using vb = vector<bool>;


#define rep(i, s, t) for (int i = s; i < t; i++)

//負の数に対応した安全なモジュロ演算
long long safeMod(long long a, long long m) {
    long long res = a % m;
    if (res < 0) res += m;
    return res;
}

int main() {
    int N; cin >> N;
    string S; cin >> S;
    ll A; cin >> A;

    ll initialValue = 0;
    for (int i = 0; i < N; ++i) {
        if (S[i] == '1') {
            initialValue += (1LL << (N-1-i));
        }
    }

    //既に良い文字列なら0を出力して終了
    if (initialValue > 0 && initialValue % A == 0) {
        cout << 0 << endl;
        return 0;
    }

    //N文字全て1(Sを操作して得られる最大の数)がA未満なら-1
    ll Max = 0;
    rep(i, 0, N) {
        Max += (1LL << (N-1-i));
    }
    if (Max < A) {
        cout << -1 << endl;
        return 0;
    }

    //前半と後半に分ける
    int n1 = N/2;
    int n2 = N-n1;

    //すべて'0'(X = 0)になる場合を特定する
    int bad_i = 0;
    int bad_j = 0;
    rep(i, 0, N) {
        if (S[i] == '1') {
            if (i < n1) {
                bad_i |= (1 << i);
            } else {
                bad_j |= (1 << (i - n1));
            }
        }
    }

    //i文字目を変えた時の変化量を記録しておく
    vector<ll> delta(N);
    rep(i, 0, N) {
        ll valChange = (1LL << (N-1-i)) % A;
        if (S[i] == '1') {
            delta[i] = (A - valChange) % A; //負方向の変化
        } else {
            delta[i] = valChange;           //正の変化
        }
    }

    ll targetMod = safeMod(-initialValue, A);
    map<ll, int> n1Result; //{変化量の余り, 操作回数}
    int ans = 1e9; //最小手数

    //前半
    for (int i=0; i<(1<<n1); i++) {
        ll n1CurrentMod = 0;
        int count = 0;
        rep(j, 0, n1) {
            if ((i >> j) & 1) {
                //iのjビット目が立っていたら、
                //jビット目を変化させる
                n1CurrentMod = (n1CurrentMod + delta[j]) % A;
                count++;
            }
        }
        //同じ余りを作る組み合わせがあれば、操作回数が少ない方を採用する
        if (n1Result.find(n1CurrentMod) == n1Result.end() || n1Result[n1CurrentMod] > count) {
            n1Result[n1CurrentMod] = count;
        }
    }

    //後半
    for (int i=0; i<(1<<n2); i++) {
        ll n2CurrentMod = 0;
        int count = 0;
        rep(j, 0, n2) {
            if ((i >> j) & 1) {
                n2CurrentMod = (n2CurrentMod + delta[j+n1]) % A;
                count++;
            }
        }
        //前半にあったら嬉しいなーっていう変化量
        ll requiredResult = safeMod(targetMod - n2CurrentMod, A);

        //前半の中にあるかな？
        auto it = n1Result.find(requiredResult);
        if (it != n1Result.end()) {
            int totalCount = it->second + count;
            if (totalCount == 0) continue;
            ans = min(ans, totalCount);
        }
    }

    //出力
    cout << ans << "\n";

    return 0;
}