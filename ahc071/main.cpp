//This code uses templates generated with AI before the contest.
//Repository URL : https://github.com/Ida-ji/kyopro/tree/main/templates
//#pragma GCC optimize("O3,unroll-loops")

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

struct FastRNG {
    using result_type = uint64_t;
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return UINT64_MAX; }

    uint64_t state = 88172645463325252ULL;

    inline uint64_t operator()() {
        state += 0xa0761d6478bd642fULL;
        unsigned __int128 mum = (unsigned __int128)state * (state ^ 0xe7037ed1a0b428dbULL);
        return (uint64_t)mum ^ (uint64_t)(mum >> 64);
    }

    inline uint64_t rand() {
        return (*this)();
    }

    inline int rand_int(int mod) {
        if (mod <= 1) return 0;
        return (int)(((unsigned __int128)rand() * (uint64_t)mod) >> 64);
    }

    inline double rand_double() { // [0.0, 1.0)
        return (rand() >> 11) * (1.0 / 9007199254740992.0);
    }
} rng;



int main() {
    //1.入力受付
    int W, H, K; cin >> W >> H >> K;
    vi c(5); rep(i, 0, 5) cin >> c[i];
    vvb hole(W, vb(H, false)); //右から、下から
    //x座標がi, i+1, i+2に位置する穴のうち、
    //最もy座標が大きいものを更新していく
    vi max_height(20, 0); 

    rep(i, 0, K) {
        int a, b; cin >> a >> b;
        hole[a][b] = true;
        int groupnum = a/3; //例えばaが7ならgroupnumは2
        if (b > max_height[groupnum]) {
            max_height[groupnum] = b;
        }
    }

    //2.max_heightに従い入れていく
    vvi covernum(W, vi(H, 0)); //どこのマスをどの幅のレンガがカバーしているか
    vvb mustcout(W, vb(H, false)); //出力する必要があるか？
    rep(i, 0, 20) {
        //iごとに高さmax_height[i]までレンガを積む
        rep(j, 0, max_height[i]+1) {
            //もしgroupにおいて一か所のみの穴(しかも中央)である場合
            //もしくは穴が0個の場合
            bool useone = false;  
            if (!hole[3*i][j] && !hole[3*i+2][j]) {
                useone = true;
            }    

            if (useone) {
                //幅1のレンガで穴を埋める
                covernum[3*i+1][j] = 1;
                mustcout[3*i+1][j] = true;
                //cout << 3*i+1 << " " << j << " " << 1 << endl;
            }
            else {
                //レンガを(3*i , j)が左下になるように積んでいく(幅3)
                covernum[3*i+1][j] = 3;
                mustcout[3*i+1][j] = true;
                //cout << 3*i << " " << j << " " << 3 << endl;
            }
        }
    }


    int it = 0;
    auto start = chrono::steady_clock::now();
    vpii center7(0); //幅7のレンガの中心{rx, ry}
    while (it < 80) {
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(now - start).count() > 1900) break;

        //後半：幅7のマスで置き換えていく
        //3.まず置き換えるマスを選ぶ
        int rx = rng.rand_int(19);
        if (rx == 0) continue; //左端は置き換え不可能
        int ry = rng.rand_int(H);
        //この時、x座標はrx*3+1でgroupはrx
        //もしmax_height[rx]がryより小さかったらcontinue;
        if (max_height[rx] < ry) continue;

        //既に置き換えされていたらcontinue;
        if (covernum[rx*3+1][ry] == 7) continue;
        //何もレンガが無かったらcontinue;
        if (mustcout[rx*3+1][ry] == false) continue;

        //[(rx-1)*3][ry]に穴が開いていたらcontinue;
        if (hole[(rx-1)*3][ry]) continue;
        //右端も同様
        if (hole[(rx+1)*3+2][ry]) continue;
        //既に幅7レンガでカバーされていたらcontinue;
        if (covernum[(rx-1)*3+1][ry] == 7) continue;
        //右端も同様
        if (covernum[(rx+1)*3+1][ry] == 7) continue;

        //一つ上/下のレンガが既に幅7ならcontinue(変える必要が無い)
        if (ry < 39 && covernum[rx*3+1][ry+1] == 7 && mustcout[rx*3+1][ry+1]) continue;
        if (ry > 0 && covernum[rx*3+1][ry-1] == 7 && mustcout[rx*3+1][ry-1]) continue;

        //両隣がmustcout=falseならcontinue(変える必要が無い)
        if (!mustcout[(rx-1)*3+1][ry] && !mustcout[(rx+1)*3+1][ry]) continue;

        it++;
        //4.付近を置き換える
        //真ん中
        covernum[rx*3+1][ry] = 7;
        center7.push_back({rx, ry});
        //その両隣
        covernum[(rx-1)*3+1][ry] = 7;
        mustcout[(rx-1)*3+1][ry] = false;
        covernum[(rx+1)*3+1][ry] = 7;
        mustcout[(rx+1)*3+1][ry] = false;


        //5.両隣の下を削る
        if (ry > 0) {
            for (int i=ry-1; i>=0; i--) {
                //そのマスが幅3か7ならbreak(削れない)
                if (covernum[(rx-1)*3+1][i] == 3 || covernum[(rx-1)*3+1][i] == 7) break;
                //そのマスに穴が開いているならbreak(削れない)
                if (hole[(rx-1)*3+1][i]) break;
                //そのマスが既に空ならbreak
                if (mustcout[(rx-1)*3+1][i] == false) break;

                //削る
                covernum[(rx-1)*3+1][i] = 0;
                mustcout[(rx-1)*3+1][i] = false;
            }
            for (int i=ry-1; i>=0; i--) {
                //そのマスが幅3か7ならbreak(削れない)
                if (covernum[(rx+1)*3+1][i] == 3 || covernum[(rx+1)*3+1][i] == 7) break;
                //そのマスに穴が開いているならbreak(削れない)
                if (hole[(rx+1)*3+1][i]) break;
                //そのマスが既に空ならbreak
                if (mustcout[(rx+1)*3+1][i] == false) break;

                //削る
                covernum[(rx+1)*3+1][i] = 0;
                mustcout[(rx+1)*3+1][i] = false;
            }
        }
    }

    //ex. 幅7じゃなくても良いやつは幅3に戻す
    for (auto [x, y] : center7) {
        //レンガの両隣の頭上がいないかチェック
        if (y < 39 && !mustcout[(x-1)*3+1][y+1] && !mustcout[(x+1)*3+1][y+1]) {
            //カバーしなければならない穴がある場合、continue;
            if (hole[x*3-2][y] || hole[x*3+4][y]) continue;
            //幅5のとき
            if (hole[x*3-1][y] || hole[x*3+3][y]) {
                covernum[x*3+1][y] = 5;
                covernum[(x-1)*3+1][y] = 0;
                covernum[(x+1)*3+1][y] = 0;                
            }
            else {
                //幅3
                covernum[x*3+1][y] = 3;
                //両隣にも穴が無い場合、幅は1に
                if (!hole[x*3][y] && !hole[x*3+2][y]) {
                    covernum[x*3+1][y] = 1;
                }
            }
        }
    }

    
    //6.出力
    int M = 0;
    rep(i, 0, 20) {
        rep(j, 0, max_height[i]+1) {
            //まず出力する必要があるかどうか見る
            if (mustcout[3*i+1][j]) {
                M++;
            }
        }
    }

    cout << M << endl;
    rep(i, 0, 20) {
        rep(j, 0, max_height[i]+1) {
            //まず出力する必要があるかどうか見る
            if (mustcout[3*i+1][j]) {
                //出力する必要がある場合、次にcovernum(レンガ幅)を見る
                if (covernum[3*i+1][j] == 1) {
                    cout << 3*i+1 << " " << j << " " << 1 << endl;
                }
                else if (covernum[3*i+1][j] == 3) {
                    cout << 3*i << " " << j << " " << 3 << endl;
                }
                else if (covernum[3*i+1][j] == 5) {
                    cout << 3*i-1 << " " << j << " " << 5 << endl;
                }
                else {
                    //幅7の場合、(3*i+1-3)がx座標
                    cout << 3*i-2 << " " << j << " " << 7 << endl;
                }
            }
        }
    }
    
}







