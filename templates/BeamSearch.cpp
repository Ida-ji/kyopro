#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vb = vector<bool>;
using vvi = vector<vector<int>>;
using vvb = vector<vector<bool>>;
using vpii = vector<pair<int, int>>;

#define rep(i, s, t) for (int i = s; i < t; i++)

// ビームの構造体
struct beam {
    int score; //ビームにおける得点
    //...他にも情報を加えたりなど
};

// スコアの降順（大きい順）でソートするための比較関数
//もし小さい順にソートするならreturn a.score < b.score;
bool compareBeams(const beam &a, const beam &b) {
    return a.score > b.score;
}

int main() {

    //...
    //ビームが入ってる配列
    vector<beam> beams;
    //初期値を格納
    //...
    
    // 初期状態のスコアを計算
    beam initbeam;
    beams.push_back(initbeam);

    //ビームサーチの幅
    int width = 30;

    //ビームサーチ
    auto start = chrono::steady_clock::now();
    while (true) {
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(now - start).count() > 1900) break;
        vector<beam> newbeams;

        for (const auto &cb : beams) {
            //近傍を生やす
            beam nb = cb;
            //スコア計算をする
            //...

            newbeams.push_back(nb);
        }
        // 元のビームと新しいビームを合わせてソートし、上位width個を残す
        for(auto &nb : newbeams) beams.push_back(nb);
        sort(beams.begin(), beams.end(), compareBeams);
        if ((int)beams.size() > width) beams.resize(width);
    }    
}