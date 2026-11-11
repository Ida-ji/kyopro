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
using vpii = vector<pair<int, int>>;
using pii = pair<int, int>;


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

int checkerror(string s1, string s2) {
    int cnt = 0;
    rep(i, 0, s1.size()) {
        if (s1[i] != s2[i]) cnt++;
    }
    return cnt;
}

//編集箇所の多いところを優先的に選ぶ
vi checkAorder(vector<string> A, int A_num) {
    vpii Aerror(A.size());
    rep(i, 0, A.size()) {
        Aerror[i] = {0, i};
    }
    rep(i, 0, A.size()-1) {
        rep(j, i+1, A.size()) {
            //編集後にエラー(手数)を加算
            Aerror[i].first += checkerror(A[i], A[j]);
        }
        //ノイズも加算
        Aerror[i].first += rng.rand_int(10);
    }
    //Aerrorをソートして、編集箇所の多い順に並べる
    sort(Aerror.begin(), Aerror.end(), greater<pair<int, int>>());
    vi A_order(A_num);
    rep(i, 0, A_num) {
        A_order[i] = Aerror[i].second;
    }
    return A_order;
}

//文字列同士の不一致数を返す関数
int count_error(const string a, const string b) {
    int ret = 0;
    rep(i, 0, a.size()) {
        if (a[i] != b[i]) ret++;
    }
    return ret;
}

//シミュレート関数
//isSimulation : シミュレーションかどうか
pair<vpii, int> simulate(int R, vector<string> A, int A_num, vi A_order, 
                string &display, int N, vi idx_shiroana, vi idx_kuroana, bool isSimulation, vi goods) {
    int simulateCount = 0; //操作回数
    //何回回転したか(一番上にある紙は2*N+rotatenumで表され、その中身(商品番号)はA_order[rotatenum])
    int rotatenum = 0; 
    int old_rotatenum = 0; //前回のクエリ(テンプレ使用)が終了した段階で一番上にあったものの商品番号
    bool use_shortcut = false; //テンプレを使うとtrue

    vpii finalOrder(0); //操作列の合計
    int orderCount = 0; //各操作ごとの回数
rep(r, 0, R) {

        //cerr << "r: " << r << endl;
        //1.商品番号受け取る
        int a;
        if (isSimulation == false) {
            cin >> a;
        }
        else {
            a = goods[r];
        }
        string target = A[a];

        vpii order(0); //各操作列
        orderCount = 0;
        //cerr << target << endl;
        bool canuse_shortcut = false; //テンプレを使えるかどうか
        //1 : そもそもテンプレが紙にあるか
        int a_idx = -1; //aは上から何番目にありますか？
        rep(i, 0, A_num) {
            if (A_order[(rotatenum+i)%A_num] == a) {
                canuse_shortcut = true;
                a_idx = i;
                break;
            }
        }

        //2: エラー箇所を逐一直すより早いかどうかチェック
        
        if (canuse_shortcut) {
            int num1 = count_error(target, display+'0'); //0は適当
            if (num1+3 <= a_idx) canuse_shortcut = false;
            a_idx = -1;
        }
        
        

        if (canuse_shortcut) {
            canuse_shortcut = false;
            //テンプレを使いまわすパターン
            //1.まずa番の位置を特定する
            //これはidx_A[a];
            if (use_shortcut) {
                //紙を再び下へ移動
                order.push_back({2*N+old_rotatenum, 2*N+A_num-1});
                simulateCount++;
                orderCount++;
                use_shortcut = false;
            }

            //2.2*N番目に番号aの紙がくるまで、紙を回し続ける
            while (A_order[rotatenum] != a) {
                //2*N番目にある紙を下に移動
                order.push_back({2*N+rotatenum, 2*N+A_num-1});
                simulateCount++;
                orderCount++;

                rotatenum = (rotatenum+1)%A_num; //次に上に来る紙の番号を更新
            }

            //4.a番の紙を最上位へ移動
            order.push_back({2*N+rotatenum, 0});
            simulateCount++;
            orderCount++;
            old_rotatenum = rotatenum; //次回のクエリに備えて、oldA_iを更新
            rotatenum = (rotatenum+1)%A_num; //次に上に来る紙の番号を更新

            //finalOrderに操作列の回数と操作列を格納
            finalOrder.push_back({orderCount, -1});
            rep(i, 0, orderCount) {
                finalOrder.push_back(order[i]);
            }
            if (isSimulation == false) {
                cout << orderCount << endl;
                rep(i, 0, orderCount) {
                    cout << order[i].first << " " << order[i].second << endl;
                }
            }
            orderCount = 0;
            use_shortcut = true;

        }
        else {
            if (use_shortcut) {
                //紙を再び下へ移動
                order.push_back({2*N+old_rotatenum, 2*N+A_num-1});
                simulateCount++;
                orderCount++;
                use_shortcut = false;
            }
            //まず、N-1番目の所を揃える
            //while (2*N番目にある紙のN-1番目の色がtargetのN-1番目の色と違う)
            while (A[A_order[rotatenum]][N-1] != target[N-1]) {
                //if (r <= 10) cerr << "oldA_i: " << rotatenum << endl;
                //2*N番目にある紙を下に移動
                order.push_back({2*N+rotatenum, 2*N+A_num-1});
                simulateCount++;
                orderCount++;
                rotatenum = (rotatenum+1)%A_num; //次に上に来る紙の番号を更新
                //if (r <= 10) cerr << "newA_i: " << rotatenum << endl;
            }

            //2.現在のディスプレイとターゲットを見比べていく
            vi shiro_kara_kuro(0); //要素は左から何番目かの数値
            vi kuro_kara_shiro(0); //要素は左から何番目かの数値
            rep(i, 0, N-1) {
                //3.現在白で、黒にしたいマス一覧を出す
                if (display[i] == '0' && target[i] == '1') {
                    shiro_kara_kuro.push_back(i);
                    //cerr << "shiro: " << i << endl;
                }
                //4.現在黒で、白にしたいマス一覧を出す
                if (display[i] == '1' && target[i] == '0') {
                    kuro_kara_shiro.push_back(i);
                    //cerr << "kuro: " << i << endl;
                }
            }

            /*
            rep(i, 0, shiro_kara_kuro.size()) {
            cerr << shiro_kara_kuro[i] << endl;
            }
            cerr << "----" << endl;
            rep(i, 0, kuro_kara_shiro.size()) {
            cerr << kuro_kara_shiro[i] << endl;
            }
            */
            



            rep(i, 0, shiro_kara_kuro.size()) {
                //まず場所を特定
                int depth_i = idx_shiroana[shiro_kara_kuro[i]];
                if (idx_kuroana[shiro_kara_kuro[i]] < idx_shiroana[shiro_kara_kuro[i]]) {
                    //黒穴の方が上にあるので、黒穴紙の方を操作する必要がある
                    depth_i = idx_kuroana[shiro_kara_kuro[i]];
                    //orderに追加
                    order.push_back({N+kuro_kara_shiro[i], 2*N-1});
                    simulateCount++;
                    orderCount++;
                    //idxを更新
                    rep(j, 0, N-1) {
                        //白
                        int depth_j = idx_shiroana[j];
                        if (depth_i >= depth_j) continue;
                        //jの方が深い場合、一つ浅くする必要がある
                        idx_shiroana[j]--;
                    }
                    rep(j, 0, N-1) {
                        //黒
                        int depth_j = idx_kuroana[j];
                        if (depth_i >= depth_j) continue;
                        //jの方が深い場合、一つ浅くする必要がある
                        idx_kuroana[j]--;
                    }
                    idx_kuroana[kuro_kara_shiro[i]] = 2*N-1;
                    continue;
                }

                //白穴の方を操作する
                //cerr << "shiro " << shiro_kara_kuro[i] << " depth: " << depth_i << endl;
                //orderに追加
                order.push_back({shiro_kara_kuro[i], 2*N-1});
                simulateCount++;
                orderCount++;

                //idxを更新
                rep(j, 0, N-1) {
                    //白
                    int depth_j = idx_shiroana[j];
                    if (depth_i >= depth_j) continue;
                    //jの方が深い場合、一つ浅くする必要がある
                    idx_shiroana[j]--;
                }
                rep(j, 0, N-1) {
                    //黒
                    int depth_j = idx_kuroana[j];
                    if (depth_i >= depth_j) continue;
                    //jの方が深い場合、一つ浅くする必要がある
                    idx_kuroana[j]--;
                }
                idx_shiroana[shiro_kara_kuro[i]] = 2*N-1;
            }


            rep(i, 0, kuro_kara_shiro.size()) {
                //まず場所を特定
                int depth_i = idx_kuroana[kuro_kara_shiro[i]];

                /*
                    if (r==1 && i<10) {
                    cerr << "shirodepth: " << idx_shiroana[shiro_kara_kuro[i]] << endl;
                    cerr << "kurodepth: " << idx_kuroana[shiro_kara_kuro[i]] << endl;
                }
                */

                if (idx_shiroana[kuro_kara_shiro[i]] < idx_kuroana[kuro_kara_shiro[i]]) {
                    //白穴の方が上にあるので、白穴紙の方を操作する必要がある
                    //cerr << "shiro " << shiro_kara_kuro[i] << " depth: " << depth_i << endl;
                    int depth_i = idx_shiroana[kuro_kara_shiro[i]];
                    //orderに追加
                    order.push_back({shiro_kara_kuro[i], 2*N-1});
                    simulateCount++;
                    orderCount++;
                    //idxを更新
                    rep(j, 0, N-1) {
                        //白
                        int depth_j = idx_shiroana[j];
                        if (depth_i >= depth_j) continue;
                        //jの方が深い場合、一つ浅くする必要がある
                        idx_shiroana[j]--;
                    }
                    rep(j, 0, N-1) {
                        //黒
                        int depth_j = idx_kuroana[j];
                        if (depth_i >= depth_j) continue;
                        //jの方が深い場合、一つ浅くする必要がある
                        idx_kuroana[j]--;
                    }
                    idx_shiroana[shiro_kara_kuro[i]] = 2*N-1;
                    continue;
                }

                //黒穴の方を操作する
                //cerr << "kuro " << kuro_kara_shiro[i] << " depth: " << depth_i << endl;
                //orderに追加
                order.push_back({N+kuro_kara_shiro[i], 2*N-1});
                simulateCount++;
                orderCount++;
                //idxを更新
                rep(j, 0, N-1) {
                    //白
                    int depth_j = idx_shiroana[j];
                    if (depth_i >= depth_j) continue;
                    //jの方が深い場合、一つ浅くする必要がある
                    idx_shiroana[j]--;
                }
                rep(j, 0, N-1) {
                    //黒
                    int depth_j = idx_kuroana[j];
                    if (depth_i >= depth_j) continue;
                    //jの方が深い場合、一つ浅くする必要がある
                    idx_kuroana[j]--;
                }
                idx_kuroana[kuro_kara_shiro[i]] = 2*N-1;
            }
            //ディスプレイを更新
            display = target;

            //finalOrderに操作列の回数と操作列を格納
            finalOrder.push_back({orderCount, -1});
            rep(i, 0, orderCount) {
                finalOrder.push_back(order[i]);
            }
            if (isSimulation == false) {
                cout << orderCount << endl;
                rep(i, 0, orderCount) {
                    cout << order[i].first << " " << order[i].second << endl;
                }
            }
            orderCount = 0;
        }
    }
    return {finalOrder, simulateCount};
}


int main() {
    // 高速入出力
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    //1.入力受付
    int N, K, P, R; cin >> N >> K >> P >> R;
    vector<string> A(P); rep(i, 0, P) cin >> A[i];

    //1.白穴紙を生成
    vi idx_shiroana(N-1); //[i]:i番目が白の紙が上から何番目か(0-indexed)
    rep(i, 0, N-1) {
        string s1(i, '.');
        string s2(N-i-1, '.');
        string s = s1 + '0' + s2;
        cout << s << endl;
        idx_shiroana[i] = i;
    }
    cout << string(N, '.') << endl; //最後の紙は白穴紙ではないので、空白を出力

    //2.黒穴紙を生成
    vi idx_kuroana(N-1); //黒穴紙が上から何番目か(0-indexed)
    rep(i, 0, N-1) {
        string s1(i, '.');
        string s2(N-i-1, '.');
        string s = s1 + '1' + s2;
        cout << s << endl;
        idx_kuroana[i] = N+i;
    }
    cout << string(N, '.') << endl; //最後の紙は黒穴紙ではないので、空白を出力

    //後は注文の序盤を再現
    int A_num = 64;
    int candidates_num = 30;
    int mont_num = 5; //モンテカルロやる回数
    //モンテカルロシミュレーションでやる注文の個数
    int mont_goods_num = 512; 

    vvi A_order_candidates(candidates_num, vi(A_num)); //Aの紙の上からの商品番号

    //0.A_orderに入れる商品列の候補を出す
    rep(i, 0, candidates_num) {
        A_order_candidates[i] = checkAorder(A, A_num);
    }


    vi best_A_order(0);

    vvi a_orders(mont_num, vi(mont_goods_num));
    //1.ランダムに注文列を作っていく
    rep(i, 0, mont_num) {
        rep(j, 0, mont_goods_num) {
            int a_r = rng.rand_int(A.size());
            a_orders[i][j] = a_r;
        }
    }


    int best_simulateCount = 1e9;
    int best_A_order_idx = -1;
    //2.注文列を元に、各候補に対してmont_num回シミュレーションする
    rep(i, 0, A_order_candidates.size()) {
        int simulateCount_i = 0;
        rep(j, 0, mont_num) {
            //ディスプレイの状態を表す文字列
            string display(N-1, '0'); //初期ではN-1個は確定で白
            display += string(1, A[A_order_candidates[i][0]][N-1]);
            auto [finalOrder, simulateCount] = 
                simulate(mont_goods_num, A, A_num, A_order_candidates[i], 
                            display, N, idx_shiroana, idx_kuroana, true, a_orders[j]);
            simulateCount_i += simulateCount;
        }
        //操作回数を更新
        if (simulateCount_i < best_simulateCount) {
            best_simulateCount = simulateCount_i;
            best_A_order_idx = i;
        }
    }


    //最良の配置
    best_A_order = A_order_candidates[best_A_order_idx];


    rep(i, 0, A_num) { //64個
        cout << A[best_A_order[i]] << endl;   
    }

    rep(i, 0, N-A_num) {
        cout << string(N, '.') << endl; //最後の紙はAの紙ではないので、空白を出力
    }


    //ディスプレイの状態を表す文字列
    string display(N-1, '0'); //初期ではN-1個は確定で白
    display += string(1, A[best_A_order[0]][N-1]);
    
    //シミュレートする
    //isSimulation=falseの時は、a_ordersは使わないので、適当に0とかで
    auto [finalOrder, simulateCount] = 
        simulate(R, A, A_num, best_A_order, display, 
                N, idx_shiroana, idx_kuroana, false, a_orders[0]);
}