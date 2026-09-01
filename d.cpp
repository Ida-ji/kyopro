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
        if (chrono::duration_cast<chrono::milliseconds>(now - start).count() > 1950) break;
        //2.移動手段をランダムに3つ選ぶ
        vpii ij(3);
        rep(i, 0, 3) {
            int ry = rng.rand_int(N);
            int rx = rng.rand_int(N);
            ij[i] = {ry, rx};
        } 

        bool visited[N][N];
        memset(visited, false, sizeof(visited)); // 高速ゼロクリア

        bool ghost_visited[N][N];
        memset(ghost_visited, false, sizeof(visited)); // 高速ゼロクリア

        int barrier[N][N];
        memset(barrier, 0, sizeof(visited));

        int posy = 0; int posx = 0;
        vi m(10000);
        vpii paper(10000, {-1, -1}); //お札を置いた場所
        int total_dist = 0;
        rep(t, 0, 10000) {
            //3.怪異が来る場所を読み取る
            auto [at, bt] = ab[t];


            //4.現在位置から次の行先の候補を3つ出す
            vpii candidates(3);
            rep(idx, 0, 3) {
                candidates[idx] = {(posy + ij[idx].first)%N, (posx + ij[idx].second)%N};
            }

            //もし(at, bt)がvisitedなら、次の行先は「まだ行ったことが無い場所」にする
            if (visited[at][bt]) {
                vi not_visited_candidates(0);
                rep(i, 0, 3) {
                    if (!visited[candidates[i].first][candidates[i].second] 
                            && !ghost_visited[candidates[i].first][candidates[i].second]) {
                        //行ったことが無いのでnot_visited_candidatesに追加
                        not_visited_candidates.push_back(i);
                    }
                }
                int r = -1;
                if (!not_visited_candidates.empty()) {
                    r = rng.rand_int((int)not_visited_candidates.size());
                    m[t] = not_visited_candidates[r];
                }
                else {
                    r = rng.rand_int(3);
                    m[t] = r;
                }
                //total_distは何もしない
                //posyとposxを更新
                posy = (posy + ij[r].first)%N;
                posx = (posx + ij[r].second)%N;
                paper[t] = {posy, posx};
                //visitedをtrueに
                visited[posy][posx] = true;
                //barrierを更新
                rep(i, max(0, posy-2), min(N-1, posy+2)) {
                    rep(j, max(0, posx-2), min(N-1, posx+2)) {
                        barrier[i][j]++;
                    }
                }
            }

            else {
                //t < 2000の時、
                if (t < 2000) {
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

                    //本当にmindが最小なんですか？
                    bool israndom = false;
                    rep(i, 0, t) {
                        int simu_d = calcdist(at, bt, paper[i].first, paper[i].second);
                        if (simu_d <= mind) {
                            //mindは最小ではないので、やはり次の行先は「まだ行ったことが無い場所」にする
                            vi not_visited_candidates(0);
                            rep(i, 0, 3) {
                                if (!visited[candidates[i].first][candidates[i].second] 
                                    && !ghost_visited[candidates[i].first][candidates[i].second]) {
                                    //行ったことが無いのでnot_visited_candidatesに追加
                                    not_visited_candidates.push_back(i);
                                }
                            }
                            int r = -1;
                            if (!not_visited_candidates.empty()) {
                                r = rng.rand_int((int)not_visited_candidates.size());
                                m[t] = not_visited_candidates[r];
                            }
                            else {
                                r = rng.rand_int(3);
                                m[t] = r;
                            }
                            total_dist += 2*(int)sqrt(t+1);
                            //posyとposxを更新
                            posy = (posy + ij[r].first)%N;
                            posx = (posx + ij[r].second)%N;
                            paper[t] = {posy, posx};
                            //visitedをtrueに
                            visited[posy][posx] = true;
                            //barrierを更新
                            rep(i, max(0, posy-2), min(N-1, posy+2)) {
                                rep(j, max(0, posx-2), min(N-1, posx+2)) {
                                    barrier[i][j]++;
                                }
                            }
                            israndom = true; //行先をランダムに変えましたのtrue
                            break;
                        }
                    }

                    if (!israndom) {
                        //6.mtを保存＆total_distに加算
                        m[t] = mt;
                        total_dist += mind;

                        //7.posyとposxを更新
                        posy = (posy + ij[mt].first)%N;
                        posx = (posx + ij[mt].second)%N;
                        visited[posy][posx] = true;
                        paper[t] = {posy, posx};
                        //barrierを更新
                        rep(i, max(0, posy-2), min(N-1, posy+2)) {
                            rep(j, max(0, posx-2), min(N-1, posx+2)) {
                                barrier[i][j]++;
                            }
                        }
                    }
                }
                else {
                    //t >= 2000の時
                    //5.各候補に対して、最も脆弱性の高いマスに行く
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
                    total_dist += (int)sqrt(t+1);

                    //posyとposxを更新
                    posy = (posy + ij[most_vulner_idx].first)%N;
                    posx = (posx + ij[most_vulner_idx].second)%N;
                    //visitedをtrueに
                    visited[posy][posx] = true;
                    //barrierを更新
                    rep(i, max(0, posy-2), min(N-1, posy+2)) {
                        rep(j, max(0, posx-2), min(N-1, posx+2)) {
                            barrier[i][j]++;
                        }
                    }
                }
            }
            //怪異が来た場所をメモ
            ghost_visited[at][bt] = true;
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