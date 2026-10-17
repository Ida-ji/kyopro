#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vb = vector<bool>;
using vvi = vector<vector<int>>;
using vvb = vector<vector<bool>>;


#define rep(i, s, t) for (int i = s; i < t; i++)

int main() {
    int N; cin >> N;
    //0.Nの桁数を調べる
    string sN = to_string(N);
    int d = (int)sN.size();
    //Nの各数字を格納
    vi num(0);
    rep(i, 0, d) {
        num.push_back(sN[i] - '0');
    }
    int ans = 0;
    //ビット全探索
    for (int i=0; i < (1 << d); i++) {
        vi a(0), b(0);
        //ビットが立っていたらaに加える
        for (int j=0; j<d; j++) {
            if (i >> j & 1) {
                a.push_back(num[j]);
            }
            else b.push_back(num[j]);
        }
            if ((int)a.size() == 0 || (int)b.size() == 0) continue;

        //ソートする
        sort(a.begin(), a.end());
        sort(b.begin(), b.end());
        if (a.back() == 0 || b.back() == 0) continue;

        //数字に直す
        vi dd(10);
        dd = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
        int aa = 0;
        int asz = (int)a.size();
        rep(i, 0, asz) {
            aa += a[i]*(dd[i]);
        }
        int bb = 0;
        int bsz = (int)b.size();
        rep(i, 0, bsz) {
            bb += b[i]*(dd[i]);
        }
        //積を取る
        if (ans < aa*bb) {
            ans = aa*bb;
        }

    }
    cout << ans << endl;
}