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
    int T; cin >> T;
    while (T>0) {
        T--;

        int N; cin >> N;
        vi A(N);
        vi B(N);
        vi diff(N);

        rep(i, 0, N) {
            int a, b; cin >> a >> b;
            A[i] = a; B[i] = b;
            diff[i] = a-b;
        }    

        //Nの偶奇で場合分け
        if (N%2 == 1) {
            //奇数
            
        }

    }
}