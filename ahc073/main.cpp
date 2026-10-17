//This code uses templates generated with AI before the contest.
//Repository URL : https://github.com/Ida-ji/kyopro/tree/main/templates
#pragma GCC optimize("O3,unroll-loops")
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vb = vector<bool>;
using vvi = vector<vector<int>>;
using vvb = vector<vector<bool>>;
using vpii = vector<pair<int, int>>;
using pii = pair<int, int>;


#define rep(i, s, t) for (int i = s; i < t; i++)



#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using vi = vector<int>;
using vb = vector<bool>;
using vvi = vector<vector<int>>;
using vvb = vector<vector<bool>>;
using vpii = vector<pair<int, int>>;

#define rep(i, s, t) for (int i = s; i < t; i++)

int dy[] = {-1, 1, 0, 0};
int dx[] = {0, 0, -1, 1};

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

// マス(sy, sx)から(ty, tx)への最短経路文字列を取得
string getpath(int sy, int sx, int ty, int tx, int N) {
    if (sy == ty && sx == tx) return "";

    //考え方：マンハッタン距離で、上に行って右に行く、みたいにシンプルにする
    string ret = "";
    if (sy > ty) {
        ret += string(sy - ty, 'U');
    } else {
        ret += string(ty - sy, 'D');
    }
    if (sx > tx) {
        ret += string(sx - tx, 'L');
    } else {
        ret += string(tx - sx, 'R');
    }

    //retをシャッフルして多様性を持たせたい
    shuffle(ret.begin(), ret.end(), rng);
    return ret;
}


int main() {
    // 高速入出力
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    //1.入力受付
    int N;cin >> N;
    int M, K, T; cin >> M >> K >> T;
    int init_ci, init_cj; cin >> init_ci >> init_cj; //現在の位置
    vpii g(M);
    rep(i, 0, M) {
        int a, b; cin >> a >> b;
        g[i] = {a, b};
    }

    int finalscore = 1e9;
    vector<string> bestans(0);
    vpii best_mahoujin(3);
    //山登り

    auto start = chrono::steady_clock::now();
    while (true) {
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(now - start).count() > 1950) break;
        //魔方陣の初期位置をランダムに設定
        vpii init_mahoujin(3); //初期の魔方陣の位置
        rep(i, 0, 3) {
            int r1 = rng.rand_int(100);
            while (r1 < 30) r1 = rng.rand_int(100);
            int r2 = rng.rand_int(100);
            while (r2 < 30) r2 = rng.rand_int(100);
            init_mahoujin[i] = {r1, r2};
        }
        int ci = init_ci;
        int cj = init_cj;

        vpii mahoujin = init_mahoujin; //魔方陣の位置

        int m = 0; //到達した目的地の個数
        int cnt = 0; //移動回数
        vector<string> ans(0);
        while (m < M) {
            //3.自分の位置から目的地までの距離を調べる
            vector<string> paths(4, "");
            paths[0] = getpath(ci, cj, g[m].first, g[m].second, N);

            //4.魔方陣を使って仮想的に反転移動する
            int ni0 = mahoujin[0].first*2 - ci;
            int nj0 = mahoujin[0].second*2 - cj;
            if (ni0 >= 0 && N > ni0 && nj0 >= 0 && N > nj0) {
                //5.反転移動した後の位置から目的地までの距離を調べる
                if (15 < ci && ci < 105 && 15 < cj && cj < 105) {
                    paths[1] = getpath(ni0, nj0, g[m].first, g[m].second, N);
                }
                
            }

            int ni1 = mahoujin[1].first*2 - ci;
            int nj1 = mahoujin[1].second*2 - cj;
            if (ni1 >= 0 && N > ni1 && nj1 >= 0 && N > nj1) {
                if (15 < ci && ci < 105 && 15 < cj && cj < 105) {
                    //5.反転移動した後の位置から目的地までの距離を調べる
                    paths[2] = getpath(ni1, nj1, g[m].first, g[m].second, N);
                }    
            }

            int ni2 = mahoujin[2].first*2 - ci;
            int nj2 = mahoujin[2].second*2 - cj;
            if (ni2 >= 0 && N > ni2 && nj2 >= 0 && N > nj2) {
                if (15 < ci && ci < 105 && 15 < cj && cj < 105) {
                    //5.反転移動した後の位置から目的地までの距離を調べる
                    paths[3] = getpath(ni2, nj2, g[m].first, g[m].second, N);
                }    
            }

            //6.距離最小のものを調べる
            int bestdist = 1e9;
            int bestidx = -1;
            char startdir;
            rep(i, 0, 4) {
                if ((int)paths[i].size() > 0 && bestdist > (int)paths[i].size()) {
                    bestdist = (int)paths[i].size();
                    bestidx = i;
                    startdir = paths[i][0];
                }
            }


            //7.反転移動か否か
            if (bestidx == 0) {
                //通常移動
                if (startdir == 'U') {
                    if (cnt > T) {
                        continue;
                    }
                    ans.push_back("U");
                    //位置更新
                    ci--;
                    cnt++;
                }
                else if (startdir == 'D') {
                    if (cnt > T) {
                        continue;
                    }
                    ans.push_back("D");
                    //位置更新
                    ci++;  
                    cnt++;
                }
                else if (startdir == 'L') {
                    if (cnt > T) {
                        continue;
                    }
                    ans.push_back("L");
                    //位置更新
                    cj--;  
                    cnt++;
                }
                else if (startdir == 'R') {
                    if (cnt > T) {
                        continue;
                    }
                    ans.push_back("R");
                    //位置更新
                    cj++;  
                    cnt++;
                }
                if (ci == g[m].first && cj == g[m].second) {
                    m++;
                }
            }
            else {
                //魔方陣移動：どの魔方陣か特定
                //使う魔方陣：mahoujin[bestidx]
                if (cnt > T) {
                    continue;
                }
                ans.push_back("J " + to_string(mahoujin[bestidx-1].first) + " " + to_string(mahoujin[bestidx-1].second));
                //魔方陣の位置更新
                auto [mahoujin_pi, mahoujin_pj] = mahoujin[bestidx-1]; //かつての魔方陣の位置
                mahoujin[bestidx-1] = {ci, cj};
                //位置更新
                ci = mahoujin_pi*2 - ci;
                cj = mahoujin_pj*2 - cj;
                if (ci == g[m].first && cj == g[m].second) {
                    m++;
                }
                cnt++;
            }
        }


        int current_score = (int)ans.size();
        if (current_score < finalscore) {
            finalscore = current_score;
            bestans = ans;
            best_mahoujin = init_mahoujin;
        }
    }

    //最適初期魔方陣位置を出力
    rep(i, 0, K) {
        cout << best_mahoujin[i].first << " " << best_mahoujin[i].second << endl;;
    }

    //3.最適解の移動を出力
    rep(i, 0, (int)bestans.size()) {
        cout << bestans[i] << endl;
    }

    return 0;
}