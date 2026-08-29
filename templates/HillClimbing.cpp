#include <bits/stdc++.h>
using ll = long long;
using namespace std;

int main() {
    //...

    //まずは初期値を設定
    //...
    int finalscore = 0;
    //山登り
    auto start = chrono::steady_clock::now();
    while (true) {
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(now - start).count() > 1900) break;
        //まずは初期化(grid = initgridとか)
        //...

        int current_score = 0;
        if (current_score > finalscore) {
        finalscore = current_score;
        //finalans = ans;
        }
    }

}