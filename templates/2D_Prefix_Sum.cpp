//This code uses templates generated with AI before the contest.
//Repository URL : https://github.com/Ida-ji/kyopro/tree/main/templates

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
    int gridsz;
    vvi grid;
    // sumgridを計算 (2次元累積和)
    vvi sumgrid(gridsz + 1, vi(gridsz + 1, 0));
    rep(i, 0, gridsz) {
        rep(j, 0, gridsz) {
            sumgrid[i + 1][j + 1] = grid[i][j] + sumgrid[i][j + 1] + sumgrid[i + 1][j] - sumgrid[i][j];
        }
    }
}