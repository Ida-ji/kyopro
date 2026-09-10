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

int main () {
    int N; cin >> N;
    vi p(N);
    rep(i, 0, N) cin >> p[i];
    int groupnum;
    if (N%10 == 0) groupnum = N/10;
    else groupnum = N%10+1;
    int count = 0;
    string ans = "Yes";
    rep(i, 0, groupnum) {
        rep(j, 0, 10) {
            if (count == N) break;
            if (p[count] <= i*10 || (i+1)*10+1 <= p[count]) ans = "No";
            count++;
        }
    }
    cout << ans << endl;
}