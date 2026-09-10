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
    ll N; cin >> N;
    vector<ll> A(N); rep(i, 0, N) cin >> A[i];
    vector<ll> B(N); rep(i, 0, N) cin >> B[i];

    vector<ll> W(N, 1);
    bool ischange = false;
    rep(i, 0, N) {
        if (A[i] > B[i]) {
            W[i] = 1000000000000000000;
            ischange = true;
        }
    }
    if (ischange) {
        cout << "Yes" << endl;
        rep(i, 0, N) cout << W[i] << " ";
    }
    else cout << "No" << endl;
}