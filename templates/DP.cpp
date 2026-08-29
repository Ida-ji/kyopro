#include <bits/stdc++.h>
using ll = long long;
using namespace std;

int main() {
    //①ナップサック問題
    //重さ w_i価値 v_iのアイテムがN個あります。重さの合計がW以下になるように選ぶとき、価値の合計の最大値を求めてください。
    int N = 3, W = 5;
    vector<int> w = {2, 3, 4}; // 重さ
    vector<int> v = {3, 4, 5}; // 価値
    
    // dp[i][j]: i個目までで容量jの時の最大価値
    vector<vector<int>> dp(N + 1, vector<int>(W + 1, 0));
    
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j <= W; ++j) {
            if (j >= w[i]) {
                // 選ぶ場合と選ばない場合の最大値
                dp[i + 1][j] = max(dp[i][j], dp[i][j - w[i]] + v[i]);
            } else {
                // 選べない（容量オーバー）
                dp[i + 1][j] = dp[i][j];
            }
        }
    }
    
    cout << "最大価値: " << dp[N][W] << endl;

    //②部分和問題
    //N個の整数a_iがあります。これらの中からいくつか選んで、合計をKにすることは可能でしょうか？
    vector<int> a = {3, 5, 8};
    int N = a.size();
    int K = 11;
    
    // dp[i][j]: i個目までで合計jが作れるか
    vector<vector<bool>> dp(N + 1, vector<bool>(K + 1, false));
    
    dp[0][0] = true; // 合計0は何も選ばなくても作れる
    
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j <= K; ++j) {
            // 使わない場合
            if (dp[i][j]) dp[i + 1][j] = true;
            // 使う場合
            if (j >= a[i] && dp[i][j - a[i]]) dp[i + 1][j] = true;
        }
    }
    
    if (dp[N][K]) cout << "合計 " << K << " は作れます" << endl;
    else cout << "合計 " << K << " は作れません" << endl;
    
    return 0;
}