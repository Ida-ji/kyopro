#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vb = vector<bool>;
using vvi = vector<vector<int>>;
using vvb = vector<vector<bool>>;
using vpii = vector<pair<int, int>>;

#define rep(i, s, t) for (int i = s; i < t; i++)

int n;
int m;

vvi initstacks; //initstacks[i][j]: 山iの下からj番目
vpii initboxidx; //i番目の箱の位置　{山の番号, 下から何番目か}

struct beam {
    //スタックの様子、出力配列、コスト
    vvi stacks; vpii output; vpii boxidx; int cost;
};

// スコアをソートするための比較関数
bool compareBeams(const beam &a, const beam &b) {
    return a.cost < b.cost;
}

int main() {
    //1.入力受取
    cin >> n >> m;
    initstacks.resize(m, vi(0));
    initboxidx.resize(n);
    rep(i, 0, m) {
        rep(j, 0, n/m) {
            int b; cin >> b;
            b--;
            initstacks[i].push_back(b);
            initboxidx[b] = {i, j};
        }    
    }

    //2.入力受取のやつを初期ビームに入れる
    vector<beam> beams;
    beams.push_back({initstacks, {}, initboxidx, 0});

    //3.ビームサーチ前の下準備
    int width = 80; //ビーム幅

    //4.tターン繰り返す
    rep(t, 0, n) {
        vector<beam> nextbeams;
        //5.各ビームについて
        for (auto cb : beams) {
            //6.t番目の箱の位置を見る
            int idx = cb.boxidx[t].first;
            int height = cb.boxidx[t].second;

            //ex.t+1番目の箱の山の位置を見る
            int idx1 = -1;
            if (t != n-1) {
                idx1 = cb.boxidx[t+1].first;
            }
            //ex2.t+2番目の箱の山の位置を見る
            int idx2 = -1;
            if (t < n-2) {
                idx2 = cb.boxidx[t+2].first;
            }

            //7.出力する{v, i}を求める
            //7-1.箱が一番上に無い場合
            if (height != (int)cb.stacks[idx].size()-1) {
                //もし(山idxのサイズ-1)に一致しないなら
                int outbox = cb.stacks[idx][height+1];
                //8.別の山に置くような近傍ビームを生やす
                rep(nextidx, 0, m) {
                    if (nextidx == idx) continue;
                    if (t != n-1 && nextidx == idx1) continue;
                    if (t < n-2 && nextidx == idx2) continue;
                    beam nb = cb;
                    //9.運ぶやつをまず出力
                    nb.output.push_back({outbox+1, nextidx+1}); 

                    int nextidxsize = (int)nb.stacks[nextidx].size();
                    //for文の範囲はheight+1から(int)nb.stacks[idx].size()
                    rep(j, height+1, (int)nb.stacks[idx].size()) {
                        //10.移動する箱の番号を取得
                        int movebox = nb.stacks[idx][j];
                        nb.stacks[nextidx].push_back(movebox);
                        //11.boxidxを更新
                        nb.boxidx[movebox] = {nextidx, (nextidxsize+(j-height-1))};
                        
                    }
                    //nextstacks[idx]のサイズを調整する
                    nb.stacks[idx].resize(height+1);

                    //12.t番目の箱を取り出す
                    nb.output.push_back({t+1, 0});
                    nb.stacks[idx].pop_back();

                    //13.コストを加算
                    nb.cost += (int)cb.stacks[idx].size() - height;

                    //14.近傍ビームを新たなビーム群に追加
                    nextbeams.push_back(nb);
                }
            } else {
                //7-2.箱が一番上な場合
                //12.そのままt番目の箱を取り出す
                beam nb = cb;
                nb.output.push_back({t+1, 0});
                nb.stacks[idx].pop_back();
                //13.コストを加算
                nb.cost++;
                //14.近傍ビームを新たなビーム群に追加
                nextbeams.push_back(nb);
            }
        }

        //15.ビーム幅で整える
        sort(nextbeams.begin(), nextbeams.end(), compareBeams);
        if ((int)nextbeams.size() > width) nextbeams.resize(width);
        beams = nextbeams; //更新
    }

    //16.最もいいビームを求める
    beam bestbeam = beams[0];

    //17.出力
    for (auto p : bestbeam.output) {
        cout << p.first << " " << p.second << endl;
    }

}