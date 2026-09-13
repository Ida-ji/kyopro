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
    // URBG要件を満たすための定義
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
    //0.最初の入力受付
    int N, M, T; cin >> N >> M >> T;
    //Tターン繰り返す
    rep(t, 0, T) {
        //1.入力受付
        vvi x(2*N*(N-1), vi(M));
        vpii x_sum(2*N*(N-1)); //{評価値の総和、種ID}
        rep(i, 0, 2*N*(N-1)) {
            x_sum[i].second = i;
            rep(j, 0, M) {
                cin >> x[i][j];
                x_sum[i].first += x[i][j];
            }    
        }

        //前半：N*N個の種を選ぶ
        vi chosen(0);
        //2.総和でソートする
        sort(x_sum.rbegin(), x_sum.rend());
        //3.前半N*N-M個を選び、残りを別の配列に転記
        vi unchosen(0);
        vb used(2*N*(N-1), false);
        rep(i, 0, N*N-M) {
            chosen.emplace_back(x_sum[i].second);
            used[x_sum[i].second] = true;
        }
        rep(i, 0, 2*N*(N-1)-N*N+M) {
            unchosen.emplace_back(x_sum[i+N*N-M].second);
        }
        //4.残りの配列の中から、各評価値ごとにmaxを求める
        rep(i, 0, M) {
            int max_value = -1;
            int max_value_idx = -1;

            rep(j, 0, 2*N*(N-1)-N*N+M) {
                if (used[unchosen[j]]) continue;
                if (max_value < x[unchosen[j]][i]) {
                    max_value = x[unchosen[j]][i];
                    max_value_idx = unchosen[j];
                }
            }

            //chosenに追加
            chosen.emplace_back(max_value_idx);
            used[max_value_idx] = true;
        }

        if ((int)chosen.size() != N*N) {
            //デバッグ用
            cout << "chosenのサイズがN*Nではありません。  :" << (int)chosen.size() << endl;
            return 0;
        }

        //後半：種の植え方を決める
        int finalscore = 0;
        vi finalpos; //植える配置の一次元配列
        auto start = chrono::steady_clock::now();
        while (true) {
            auto now = chrono::steady_clock::now();
            if (chrono::duration_cast<chrono::milliseconds>(now - start).count() > 190) break;
            //5.植える位置をランダムに決める
            vi current_pos = chosen;
            shuffle(current_pos.begin(), current_pos.end(), rng); //シャッフル

            //6.下方向と右方向において、仮想的に新たな種を作り、
            //評価値の仮想総和を求める
            int current_score = 0;
            //下
            rep(i, 0, N*(N-1)) {
                rep(j, 0, M) {
                    //i番目と(i+N)番目の種を交配
                    if (rng.rand_int(2) == 0) {
                        current_score += x[current_pos[i]][j];
                    }
                    else {
                        current_score += x[current_pos[i+N]][j];
                    }
                }
            }
            //上
            rep(i, 0, N*N) {
                if ((i+1)%N == 0) continue;
                rep(j, 0, M) {
                    //i番目と(i+1)番目の種を交配
                    if (rng.rand_int(2) == 0) {
                        current_score += x[current_pos[i]][j];
                    }
                    else {
                        current_score += x[current_pos[i+1]][j];
                    }
                }
            }

            //7.比較
            if (current_score > finalscore) {
                finalscore = current_score;
                finalpos = current_pos;
            }
        }
        //8.出力
        rep(i, 0, N) {
            rep(j, 0, N) {
                cout << finalpos[i*N+j] << " ";
            }
            cout << endl;
        }
    }
}