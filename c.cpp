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

struct FastRNG {
    uint64_t state = 88172645463325252ULL;

    inline uint64_t rand() {
        state += 0xa0761d6478bd642fULL;
        unsigned __int128 mum = (unsigned __int128)state * (state ^ 0xe7037ed1a0b428dbULL);
        return (uint64_t)mum ^ (uint64_t)(mum >> 64);
    }

    inline int rand_int(int mod) {
        if (mod <= 1) return 0;
        return (int)(((unsigned __int128)rand() * (uint64_t)mod) >> 64);
    }

    inline double rand_double() { // [0.0, 1.0)
        return (rand() >> 11) * (1.0 / 9007199254740992.0);
    }
} rng;

int calcdist(int a, int b, int c, int d) {
    return (int)abs(a-c) + (int)abs(b-d);
}


int dy[] = {-1, 1, 0, 0};
int dx[] = {0, 0, -1, 1};


int main() {
    //1.入力受付
    int N, M; cin >> N >> M;
    vpii ab(N*N);
    rep(i, 0, N*N) {
        int a, b; cin >> a >> b;
        ab[i] = {a, b};
    }


    int finalscore = 1e9;
    vpii final_ij = {};
    vi final_m(10000);
    //山登り
    auto start = chrono::steady_clock::now();
    while (true) {
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(now - start).count() > 1900) break;
        //2.移動手段をランダムに3つ選ぶ
        vpii ij(3);
        rep(i, 0, 3) {
            int ry = rng.rand_int(N);
            int rx = rng.rand_int(N);
            ij[i] = {ry, rx};
        } 

        bool visited[N][N];
        memset(visited, false, sizeof(visited)); // 高速ゼロクリア

        int barrier[N][N];
        memset(visited, 0, sizeof(visited));

        int posy = 0; int posx = 0;
        vi m(10000);
        int total_dist = 0;
        rep(t, 0, 10000) {
            //3.怪異が来る場所を読み取る
            auto [at, bt] = ab[t];


            //4.現在位置から次の行先の候補を3つ出す
            vpii candidates(3);
            rep(idx, 0, 3) {
                candidates[idx] = {(posy + ij[idx].first)%N, (posx + ij[idx].second)%N};
            }

            //もし(at, bt)の周辺がvisitedなら、次の行先は「一番脆弱な場所」にする
            bool isvisited = false;
            rep(dir, 0, 4) {
                int at_around = at + dy[dir];
                int bt_around = bt + dx[dir];
                //盤面外ならcontinue;
                if (at_around < 0 || N <= at_around || bt_around < 0 || N <= bt_around) continue;
                if (visited[at_around][bt_around]) isvisited = true;
            }
            if (isvisited) {
                int most_vulner_idx = -1;
                int most_vulner_num = 1e9;
                rep(i, 0, 3) {
                    if (barrier[candidates[i].first][candidates[i].second] < 
                        most_vulner_num) {
                            most_vulner_idx = i;
                            most_vulner_num = barrier[candidates[i].first][candidates[i].second];
                        }
                }
                m[t] = most_vulner_idx;
                //total_distは何もしない
                //posyとposxを更新
                posy = (posy + ij[most_vulner_idx].first)%N;
                posx = (posx + ij[most_vulner_idx].second)%N;
                //visitedをtrueに
                visited[posy][posx] = true;
                //barrierを更新
                rep(i, max(0, posy-5), min(N-1, posy+5)) {
                    rep(j, max(0, posx-5), min(N-1, posx+5)) {
                        barrier[i][j]++;
                    }
                }
            }

            else {

                //5.各候補に対して、一番怪異との距離が小さいものを求める
                int mt = -1;
                int mind = 1e9;
                rep(idx2, 0, 3) {
                    int d = calcdist(at, bt, candidates[idx2].first, candidates[idx2].second);
                    if (d < mind) {
                        mt = idx2;
                        mind = d;
                    }
                }

                //6.mtを保存＆total_distに加算
                m[t] = mt;
                total_dist += mind;

                //7.posyとposxを更新
                posy = (posy + ij[mt].first)%N;
                posx = (posx + ij[mt].second)%N;
                //visitedをtrueに
                visited[posy][posx] = true;

                //barrierを更新
                rep(i, max(0, posy-5), min(N-1, posy+5)) {
                    rep(j, max(0, posx-5), min(N-1, posx+5)) {
                        barrier[i][j]++;
                    }
                }
            }
        }

        int current_score = total_dist;
        if (current_score < finalscore) {
          finalscore = current_score;
          final_ij = ij;
          final_m = m;
        }
    }

    //7.出力
    rep(i, 0, 3) cout << final_ij[i].first << " " << final_ij[i].second << endl;
    rep(i, 0, 10000) cout << final_m[i] << endl;

}