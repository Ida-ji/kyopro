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

    bool operator>(const beam &other) const {
        return score > other.score;
    }

    bool operator<(const beam &other) const {
        return score < other.score;
    }
};

// スコアの降順（大きい順）でソートするための比較関数
//もし小さい順にソートするならreturn a.score < b.score;
bool compareBeams(const beam &a, const beam &b) {
    return a.score > b.score;
}


// Chokudaiサーチ
int main() {
    // 高速化設定
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Chokudaiサーチ パラメータ設定
    const int MAX_DEPTH = 200;      // 探索の最大深さ（最終ゴールとなる深さ）
    const int CHOKUDAI_WIDTH = 1;   // 1周あたり各深さから取り出す状態数（基本は 1）
    const size_t MAX_BEAM_SIZE = 30; // 各深さに保持する最大要素数（MLE防止）
    int width = 5; //生やす近傍の個数

    // 各深さの状態を multiset（優先度付き集合）で管理
    vector<multiset<beam>> beams(MAX_DEPTH + 1);

    // 初期状態のスコアを計算
    beam initbeam;

    //...
    beams[0].insert(initbeam);


    int loop_count = 0; // 周回数カウント

    auto start = chrono::steady_clock::now();
    while (true) {
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(now - start).count() > 1900) break;

        loop_count++;

        // 浅い層 (0) から 順番に 深い層 (MAX_DEPTH) へ1段階ずつ展開
        rep(depth, 0, MAX_DEPTH) {
            if (beams[depth].empty()) continue;

            // 各深さから CHOKUDAI_WIDTH 個の状態を取り出して展開
            rep(w, 0, CHOKUDAI_WIDTH) {
                if (beams[depth].empty()) break;
                // ★最高スコアの要素を O(log B) で取り出して削除
                auto it = prev(beams[depth].end());
                beam cb = *it;
                beams[depth].erase(it);

                rep(wid, 0, width) {
                    //近傍を生やしてdepth+1の深さに入れる


                    // 実装例：
                    beam nb = cb;

                    //...
                    if (depth + 1 <= MAX_DEPTH) {
                        // ★状態を追加
                        beams[depth + 1].insert(nb);

                        // ★上限を超えたら「最もスコアが低い要素」を O(log B) で捨てる（MLE防止）
                        if (beams[depth + 1].size() > MAX_BEAM_SIZE) {
                            beams[depth + 1].erase(beams[depth + 1].begin());
                        }
                    }
                }
            }
        }
    }

    // 最終深さ MAX_DEPTH に到達した中で最も優秀な状態を取り出す
    if (!beams[MAX_DEPTH].empty()) {
        beam bestbeam = *prev(beams[MAX_DEPTH].end());

        // 出力処理
        //...
    }

    /*
    // 全層の中で最もスコアが高い状態を出力したい場合
    ll best_score = -1;
    beam bestbeam;

    rep(d, 0, MAX_DEPTH + 1) {
        if (!beams[d].empty()) {
            beam top_beam = *prev(beams[d].end());
            if (top_beam.score > best_score) {
                best_score = top_beam.score;
                bestbeam = top_beam;
            }
        }
    }
    */

    return 0;
}