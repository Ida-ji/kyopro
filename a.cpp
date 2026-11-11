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
    //高々2回だけ2点swapする
    int swap_num = rng.rand_int(3);
    rep(i, 0, swap_num) {
        int r1 = rng.rand_int(A_num);
        int r2 = rng.rand_int(A_num);
        swap(A_order[r1], A_order[r2]);
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

//Aを受け取り、最適なpaper127を返す関数
string get_best_127(vector<string> A, int N, vi A_order_candidate) {
    vpii num_one(N);
    rep(i, 0, N) {
        num_one[i] = {0, i};
    }
    rep(i, 0, A.size()) {
        //もしA_order_candidatesに含まれていたら、無視する
        bool is_in_candidate = false;
        rep(j, 0, A_order_candidate.size()) {
            string s = A[A_order_candidate[j]];
            if (s == A[i]) {
                is_in_candidate = true;
                break;
            }
        }
        if (is_in_candidate) continue;
        rep(j, 1, N-1) {
            if (A[i][j] == '1') num_one[j].first++;
        }
    }
    //適度にノイズを足す
    rep(i, 0, 5) {
        int r = rng.rand_int(N-2)+1;
        num_one[r].first += rng.rand_int(5);
    }
    //num_oneをソートして、1の数が多い順に並べる
    sort(num_one.rbegin(), num_one.rend());
    string paper127(N, '.');
    paper127[0] = '1';
    rep(i, 0, 5) {
        paper127[num_one[i].second] = '1';
    }
    return paper127;
}
//idxを更新する関数
void update_idx(int depth_i, vi &idx_shiroana, vi &idx_kuroana, int N) {
    rep(j, 0, N-1) {
        //白
        int depth_j = idx_shiroana[j];
        if (depth_i > depth_j) continue;
        //jの方が深い場合、一つ浅くする必要がある
        idx_shiroana[j]--;
    }
    rep(j, 0, N-1) {
        //黒
        int depth_j = idx_kuroana[j];
        if (depth_i > depth_j) continue;
        //jの方が深い場合、一つ浅くする必要がある
        idx_kuroana[j]--;
    }
}

//シミュレート関数
//isSimulation : シミュレーションかどうか
pair<vpii, int> simulate(int R, vector<string> A, int A_num, vi A_order, 
                string &display, int N, vi idx_shiroana, vi idx_kuroana, bool isSimulation, vi goods, string paper127) {
    int simulateCount = 0; //操作回数
    //何回回転したか(一番上にある紙は2*N+rotatenumで表され、その中身(商品番号)はA_order[rotatenum])
    int rotatenum = 0; 
    int old_rotatenum = 0; //前回のクエリ(テンプレ使用)が終了した段階で一番上にあったものの商品番号
    bool can_use_paper127 = false; //paper127を使えるかどうか
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
        //0. paper127を使っていたら、紙を再び下へ移動
        if (can_use_paper127) {
            order.push_back({N+63, N+63});
            orderCount++;
            simulateCount++;
            can_use_paper127 = false;

            //それに伴い、idx_shiroanaとidx_kuroanaを更新する
            rep(j, 0, N-1) {
                idx_shiroana[j]--;
                idx_kuroana[j]--;
            }
            //ここで、最上位にあったpaper127を下に移動させたので、displayが変化する
            //paper127の4か所は黒くなっているので、そこを反映させる
            rep(i, 0, N-1) {
                if (paper127[i] == '1') {
                    /*
                    if (r == 697 && isSimulation == false) {
                        cerr << "r : " << r << endl;
                        cerr << "display : " << display << endl;
                        cerr << "shiro : " << idx_shiroana[i] << endl;
                        cerr << "kuro : " << idx_kuroana[i] << endl;
                    }
                    */
                    //この番目のところで、一番最初に見える色が黒か白か特定する
                    if (idx_shiroana[i] > idx_kuroana[i]) {
                        //黒穴の方が上にあるので、黒にする
                        display[i] = '1';
                    }
                    else {
                        //白穴の方が上にあるので、白にする
                        display[i] = '0';
                    }
                    
                }
            }
        } 
        /*
        if (r == 694 && isSimulation == false) {
            cerr << "r : "  << r << endl;
            cerr << "target  : " << target << endl;
            cerr << "display : " << display << endl;
            for (int i = 0; i < N-1; i++) {
                cerr << "shiro : " << idx_shiroana[i] << endl;
                cerr << "kuro : " << idx_kuroana[i] << endl;
            }
        }
        */

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
            if (num1+2 <= a_idx) canuse_shortcut = false;
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

            //ex. paper127は「4か所だけ黒く、後は全て穴」の紙にする
            //まずはハードコード

            //現在のaの4か所が黒いか？(papre 127を使えるか？)
            can_use_paper127 = true;
            rep(i, 0, N-1) {
                if (paper127[i] == '1' && target[i] == '0') {
                    can_use_paper127 = false;
                    break;
                }
            }

            //もし使えるなら、
            if (can_use_paper127) {
                //paper127を使うために、0番目のidx_shiroanaとidx_kuroanaを下に持ってくる
                if (idx_shiroana[0] < idx_kuroana[0]) {
                    //白穴の方が上にあるので、白穴紙の方を先に操作する必要がある
                    order.push_back({0, 2*N-1});
                    update_idx(idx_shiroana[0], idx_shiroana, idx_kuroana, N);
                    idx_shiroana[0] = 2*N-1;
                    order.push_back({N, 2*N-1});
                    update_idx(idx_kuroana[0], idx_shiroana, idx_kuroana, N);
                    idx_kuroana[0] = 2*N-1;
                    simulateCount += 2;
                    orderCount += 2;
                }
                else {
                    //黒穴の方が上にあるので、黒穴紙の方を操作する必要がある
                    order.push_back({N, 2*N-1});
                    update_idx(idx_kuroana[0], idx_shiroana, idx_kuroana, N);
                    idx_kuroana[0] = 2*N-1;
                    order.push_back({0, 2*N-1});
                    update_idx(idx_shiroana[0], idx_shiroana, idx_kuroana, N);
                    idx_shiroana[0] = 2*N-1;
                    simulateCount += 2;
                    orderCount += 2;
                }
                //まず、paper127を最上位に持ってくる
                order.push_back({N+63, 0});
                simulateCount++;
                orderCount++;
                //そして、idx_shiroanaとidx_kuroanaを更新する
                rep(j, 1, N-1) {
                    //白
                    idx_shiroana[j]++;
                    //黒
                    idx_kuroana[j]++;
                }
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
                //もしpaper127を使っているのなら、そこの4か所は黒くなっているので、スキップする
                if (display[i] == '0' && target[i] == '1') {
                    if (can_use_paper127 && paper127[i] == '1') continue;
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
                    if (can_use_paper127) {
                        order.push_back({N+shiro_kara_kuro[i], 2*N-1});    
                    }
                    else order.push_back({N+shiro_kara_kuro[i], 2*N-2});
                    simulateCount++;
                    orderCount++;
                    //idxを更新
                    update_idx(depth_i, idx_shiroana, idx_kuroana, N);
                    if (can_use_paper127) {
                        idx_kuroana[shiro_kara_kuro[i]] = 2*N-1;
                    }
                    else idx_kuroana[shiro_kara_kuro[i]] = 2*N-2;
                    continue;
                }

                //白穴の方を操作する
                //cerr << "shiro " << shiro_kara_kuro[i] << " depth: " << depth_i << endl;
                //orderに追加
                if (can_use_paper127) {
                    order.push_back({shiro_kara_kuro[i], 2*N-1});    
                }
                else order.push_back({shiro_kara_kuro[i], 2*N-2});
                simulateCount++;
                orderCount++;

                //idxを更新
                update_idx(depth_i, idx_shiroana, idx_kuroana, N);
                if (can_use_paper127) {
                    idx_shiroana[shiro_kara_kuro[i]] = 2*N-1;
                }
                else idx_shiroana[shiro_kara_kuro[i]] = 2*N-2;
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
                    if (can_use_paper127) {
                        order.push_back({kuro_kara_shiro[i], 2*N-1});    
                    }
                    else order.push_back({kuro_kara_shiro[i], 2*N-2});
                    simulateCount++;
                    orderCount++;
                    //idxを更新
                    update_idx(depth_i, idx_shiroana, idx_kuroana, N);
                    if (can_use_paper127) {
                        idx_shiroana[kuro_kara_shiro[i]] = 2*N-1;
                    }
                    else idx_shiroana[kuro_kara_shiro[i]] = 2*N-2;
                    continue;
                }

                //黒穴の方を操作する
                //cerr << "kuro " << kuro_kara_shiro[i] << " depth: " << depth_i << endl;
                //orderに追加
                if (can_use_paper127) {
                    order.push_back({N+kuro_kara_shiro[i], 2*N-1});    
                }
                else order.push_back({N+kuro_kara_shiro[i], 2*N-2});
                simulateCount++;
                orderCount++;
                //idxを更新
                update_idx(depth_i, idx_shiroana, idx_kuroana, N);
                if (can_use_paper127) {
                    idx_kuroana[kuro_kara_shiro[i]] = 2*N-1;
                }
                else idx_kuroana[kuro_kara_shiro[i]] = 2*N-2;
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

    
    /*
    int paper127_candidates_num = 15;
    vector<string> paper127_candidates(paper127_candidates_num);
    rep(i, 0, paper127_candidates_num) {
    
        int r1 = rng.rand_int(9);
        int r2 = rng.rand_int(10);
        int r3 = rng.rand_int(10);
        int r4 = rng.rand_int(10);
        int r5 = rng.rand_int(10);

        string paper127 = string(N, '.');
        paper127[0] = '1';
        paper127[1+r1] = '1';
        paper127[1+r1+r2] = '1';
        paper127[1+r1+r2+r3] = '1';
        paper127[1+r1+r2+r3+r4] = '1';
        paper127[1+r1+r2+r3+r4+r5] = '1';
        paper127_candidates[i] = paper127;
    
        //paper127_candidates[i] = get_best_127(A, N);
    }
    */

    //ex. paper127は「4か所だけ黒く、後は全て穴」の紙にする
    //string paper127 = string(4, '1') + string(N-4, '.');
    //cout << paper127 << endl; //例

    //後は注文の序盤を再現
    int A_num = 64;
    int candidates_num = 15;
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

    int paper127_candidates_num = 15;
    pair<string, int> best_paper127_and_idx = {"", -1};
    int best_simulateCount = 1e9;
    //2.注文列を元に、各候補に対してmont_num回シミュレーションする

        rep(i, 0, A_order_candidates.size()) {
            int simulateCount_i = 0;
            rep(j, 0, paper127_candidates_num) {
                string paper127 = get_best_127(A, N, A_order_candidates[i]);
                rep(k, 0, mont_num) {
                //ディスプレイの状態を表す文字列
                    string display(N-1, '0'); //初期ではN-1個は確定で白
                    display += string(1, A[A_order_candidates[i][0]][N-1]);
                    auto [finalOrder, simulateCount] = 
                        simulate(mont_goods_num, A, A_num, A_order_candidates[i], 
                                    display, N, idx_shiroana, idx_kuroana, true, a_orders[j], paper127);
                    simulateCount_i += simulateCount;
                }
                //操作回数を更新
                if (simulateCount_i < best_simulateCount) {
                    best_simulateCount = simulateCount_i;
                    best_paper127_and_idx = {paper127, i};
                }
            }    
        }
    

    //最良のpaper127
    string best_paper127 = best_paper127_and_idx.first;
    cout << best_paper127 << endl;

    //最良の配置
    best_A_order = A_order_candidates[best_paper127_and_idx.second];


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
                N, idx_shiroana, idx_kuroana, false, a_orders[0], best_paper127);
}