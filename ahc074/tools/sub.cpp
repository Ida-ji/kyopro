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


//タワーとブロック番号を受け取って、そのディスプレイを返す関数
string see_subDisplay(const vector<string> &tower, int b) {
    int depth_start = 12*b; //そのブロックが始まる深さ
    int depth_end = 12*(b+1); //そのブロックが終わる深さ
    int block_start = 4*b; //そのブロックが始まる列番号
    string ret = "";
    rep(i, block_start, block_start+4) {
        rep(j, depth_start, depth_end) {
            if (tower[j][i] == '0' || tower[j][i] == '1') {
                ret += tower[j][i];
                break;
            }
        }
    }   
    return ret; 
}

//サブターゲットとサブディスプレイを受け取って、エラー箇所の個数を返す関数
int count_error(const string &sub_target, const string &sub_display) {
    int ret = 0;
    rep(i, 0, 4) {
        if (sub_display[i] != sub_target[i]) ret++;
    }
    return ret;
}

//タワーとブロックとサブターゲットとサブディスプレイとオーダーを受け取って、
//オーダーを更新する関数
void step(vector<string> &tower, int b, const string &sub_target, string &sub_display, 
        vpii &order, vi &tower_ID, int error_count) {
    int depth_start = 12*b; //そのブロックが始まる深さ
    int depth_end = 12*(b+1); //そのブロックが終わる深さ
    int block_start = 4*b; //そのブロックが始まる列番号

    //cerr << "target: " << sub_target << " " << "display: " << sub_display << endl;
    while (sub_target != sub_display) {
        //12枚の紙において、「抜き出す位置」と「挿入する位置」を試す
        int best_depth = -1; int best_pos = -1;

        //まず、「選べる位置」を特定する
        vi depth_canmove(0); //最大4つ
        rep(i, block_start, block_start+4) {
            rep(j, depth_start, depth_end) {
                if (tower[j][i] == '0' || tower[j][i] == '1') {
                    depth_canmove.push_back(j);
                    break;
                }
            }
        }
        //for (auto x : depth_canmove) cerr << x << endl;

        vpii move_candidate(0);
        //選べる位置から、挿入する位置を試す(最大64通り)
        rep(d, 0, depth_canmove.size()) {
            rep(j, depth_canmove[d]+1, depth_end) {
                int i = depth_canmove[d];
                if (i == j) continue;
                //深さiにある紙を、深さjの紙の直下に置く
                vector<string> new_tower = tower;
                string str_i = tower[i];
                new_tower.erase(new_tower.begin() + i);
                new_tower.insert(new_tower.begin() + j, str_i);

                vi new_tower_ID = tower_ID;
                int int_i = tower_ID[i];
                new_tower_ID.erase(new_tower_ID.begin() + i);
                new_tower_ID.insert(new_tower_ID.begin() + j, int_i);

                //エラーの個数を調べる
                string new_display = see_subDisplay(new_tower, b);
                int err = count_error(sub_target, new_display);

                //0(ターゲットと一致)なら
                if (err == 0) {
                    //orderに追加して、終了
                    order.push_back({tower_ID[i], j});
                    //cerr << "完了: " << tower_ID[i] << " " << j << endl;
                    //towerとtower_IDをnewに差し替える
                    tower = new_tower;
                    tower_ID = new_tower_ID;
                    return;
                }
                else if (err < error_count) {
                    move_candidate.push_back({i, j});
                }
                //cerr << "試した：" << i << " " << j << endl;
            }
        }

        int r_depth;
        int r_j;
        if (!move_candidate.empty() && rng.rand_double() > 0.96) {
            //集まったmove_candidateの中からランダムに選ぶ
            int r = rng.rand_int(move_candidate.size());
            //for (auto [x, y] : move_candidate) cerr << x << " " << y << endl;
            r_depth = move_candidate[r].first;
            r_j = move_candidate[r].second;
        }
        else {
            //全体から選ぶ
            int r1 = rng.rand_int(depth_canmove.size());
            int r2 = 11;
            r_depth = depth_canmove[r1];
            r_j = depth_start + r2;
        }


        order.push_back({tower_ID[r_depth], r_j});
        //cerr << "途中: " << tower_ID[r_depth] << " " << r_j << endl;

        //towerとtower_IDを更新
        string str_r = tower[r_depth];
        tower.erase(tower.begin() + r_depth);
        tower.insert(tower.begin() + r_j, str_r);

        int int_r = tower_ID[r_depth];
        tower_ID.erase(tower_ID.begin() + r_depth);
        tower_ID.insert(tower_ID.begin() + r_j, int_r);

        //sub_displayを更新
        sub_display = see_subDisplay(tower, b);
    }
}


int main() {
    // 高速入出力
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    //1.入力受付
    int N, K, P, R; cin >> N >> K >> P >> R;
    vector<string> A(P); rep(i, 0, P) cin >> A[i];

    vector<string> component(12);
    component[0] = "...1";
    component[1] = "..1.";
    component[2] = ".1..";
    component[3] = "1...";
    component[4] = "...0";
    component[5] = "..0.";
    component[6] = ".0..";
    component[7] = "0...";
    component[8] = "....";
    component[9] = "....";
    component[10] = "....";
    component[11] = "....";

    //1.タワー構築
    vector<string> tower(K);
    vi tower_ID(K);
    iota(tower_ID.begin(), tower_ID.end(), 0); //0～K-1を生成
    rep(b, 0, 16) {
        //12個で1ブロック
        string s(4*b, '.');
        string e(4*(16-b-1), '.');
        rep(i, 0, 12) {
            tower[12*b+i] = s + component[i] + e;
        }
    }

    //2.タワーを出力
    rep(i, 0, K) {
        cout << tower[i] << endl;
    }

    //-----------------------------------------------------

    rep(r, 0, R) {
        //商品番号を受け取り
        int a; cin >> a;
        string target = A[a];

        //3.各ブロックごとに揃えていく
        vpii order(0); //操作列を格納する
        rep(b, 0, 16) {
            string sub_target = target.substr(4*b, 4);
            string sub_display = see_subDisplay(tower, b);
            if (sub_target == sub_display) continue; //揃っている場合はスキップ
            else {
                //揃っていない場合は、揃える
                //4.サブターゲットとサブディスプレイでのエラー箇所の個数を調べる
                int error_count = count_error(sub_target, sub_display);

                //5.ステップ関数を実行する
                step(tower, b, sub_target, sub_display, order, tower_ID, error_count);
                //cerr << "order_size: " << order.size() << endl;
            }
            //cerr << "step" << b << "を終了します"  << endl;
        }

        //6.操作列を出力
        cout << order.size() << endl;
        rep(i, 0, order.size()) {
            cout << order[i].first << " " << order[i].second << endl;
        }
        cerr << "turn" << r << "を終了します"  << endl;
    }
}