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





int main() {
    // 高速入出力
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    //1.入力受付
    int N, K, P, R; cin >> N >> K >> P >> R;
    vector<string> A(P); rep(i, 0, P) cin >> A[i];

    //1.白穴紙を生成
    vi idx_shiroana(N); //[i]:i番目が白の紙が上から何番目か(0-indexed)
    rep(i, 0, N) {
        string s1(i, '.');
        string s2(N-i-1, '.');
        string s = s1 + '0' + s2;
        cout << s << endl;
        idx_shiroana[i] = i;
    }

    //2.黒穴紙を生成
    vi idx_kuroana(N); //黒穴紙が上から何番目か(0-indexed)
    rep(i, 0, N) {
        string s1(i, '.');
        string s2(N-i-1, '.');
        string s = s1 + '1' + s2;
        cout << s << endl;
        idx_kuroana[i] = N+i;
    }
    //後は注文の序盤を再現
    int idx_A0;
    rep(i, 0, N) {
        if (i == 0) {
            cout << A[245] << endl;
            idx_A0 = 2*N;
        }    
        else {
            string ss(N, '.');
            cout << ss << endl;
        }
    }

    //ディスプレイの状態を表す文字列
    string display(N, '0'); //初期では全て白
    //----------------------------------------------------

    bool use_shortcut = false; //テンプレを使うとtrue
    //クエリ処理を繰り返す
    rep(r, 0, R) {

        //cerr << "r: " << r << endl;
        //1.商品番号受け取る
        int a; cin >> a;
        string target = A[a];

        //cerr << target << endl;
        vpii order(0); //操作列
        if (a == 245) {
            //テンプレを使いまわすパターン
            //1.まずa番の位置を特定する
            //これはidx_A0;
            //2.idx_shiroana[N-1]を2*N番目に移動
            if (idx_shiroana[N-1] < idx_kuroana[N-1]) {
                order.push_back({N-1, 2*N});
                //idxを更新
                int depth_i = idx_shiroana[N-1];
                rep(j, 0, N) {
                    //白
                    int depth_j = idx_shiroana[j];
                    if (depth_i >= depth_j) continue;
                    //jの方が深い場合、一つ浅くする必要がある
                    idx_shiroana[j]--;
                }
                rep(j, 0, N) {
                    //黒
                    int depth_j = idx_kuroana[j];
                    if (depth_i >= depth_j) continue;
                    //jの方が深い場合、一つ浅くする必要がある
                    idx_kuroana[j]--;
                }
                idx_shiroana[N-1] = 2*N;

                //3.idx_kuroana[N-1]を2*N番目に移動
                order.push_back({2*N-1, 2*N});
                //idxを更新
                depth_i = idx_kuroana[N-1];
                rep(j, 0, N) {
                    //白
                    int depth_j = idx_shiroana[j];
                    if (depth_i >= depth_j) continue;
                    //jの方が深い場合、一つ浅くする必要がある
                    idx_shiroana[j]--;
                }
                rep(j, 0, N) {
                    //黒
                    int depth_j = idx_kuroana[j];
                    if (depth_i >= depth_j) continue;
                    //jの方が深い場合、一つ浅くする必要がある
                    idx_kuroana[j]--;
                }
                idx_kuroana[N-1] = 2*N;
            }
            else {

                //2.idx_kuroana[N-1]を2*N番目に移動
                order.push_back({2*N-1, 2*N});
                //idxを更新
                int depth_i = idx_kuroana[N-1];
                rep(j, 0, N) {
                    //白
                    int depth_j = idx_shiroana[j];
                    if (depth_i >= depth_j) continue;
                    //jの方が深い場合、一つ浅くする必要がある
                    idx_shiroana[j]--;
                }
                rep(j, 0, N) {
                    //黒
                    int depth_j = idx_kuroana[j];
                    if (depth_i >= depth_j) continue;
                    //jの方が深い場合、一つ浅くする必要がある
                    idx_kuroana[j]--;
                }
                idx_kuroana[N-1] = 2*N;
                //3.idx_shiroana[N-1]を2*N番目に移動
                order.push_back({N-1, 2*N});
                //idxを更新
                depth_i = idx_shiroana[N-1];
                rep(j, 0, N) {
                    //白
                    int depth_j = idx_shiroana[j];
                    if (depth_i >= depth_j) continue;
                    //jの方が深い場合、一つ浅くする必要がある
                    idx_shiroana[j]--;
                }
                rep(j, 0, N) {
                    //黒
                    int depth_j = idx_kuroana[j];
                    if (depth_i >= depth_j) continue;
                    //jの方が深い場合、一つ浅くする必要がある
                    idx_kuroana[j]--;
                }
                idx_shiroana[N-1] = 2*N;
            }

            //4.0番の紙を最上位へ移動
            order.push_back({2*N, 0});

            int sz_order = order.size();
            cout << sz_order << endl;
            rep(i, 0, sz_order) {
                cout << order[i].first << " " << order[i].second << endl;
            }
            use_shortcut = true;

        }
        else {
            if (use_shortcut) {
                //紙を再び2*Nへ移動
                order.push_back({2*N, 2*N});
                idx_shiroana[N-1]--;
                idx_kuroana[N-1]--;
                //cerr << "shiroana: " << idx_shiroana[N-1] << endl;
                //cerr << "kuroana: " << idx_kuroana[N-1] << endl;
                use_shortcut = false;
            }
            //2.現在のディスプレイとターゲットを見比べていく
            vi shiro_kara_kuro(0); //要素は左から何番目かの数値
            vi kuro_kara_shiro(0); //要素は左から何番目かの数値
            rep(i, 0, N) {
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
                    //idxを更新
                    rep(j, 0, N) {
                        //白
                        int depth_j = idx_shiroana[j];
                        if (depth_i >= depth_j) continue;
                        //jの方が深い場合、一つ浅くする必要がある
                        idx_shiroana[j]--;
                    }
                    rep(j, 0, N) {
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

                //idxを更新
                rep(j, 0, N) {
                    //白
                    int depth_j = idx_shiroana[j];
                    if (depth_i >= depth_j) continue;
                    //jの方が深い場合、一つ浅くする必要がある
                    idx_shiroana[j]--;
                }
                rep(j, 0, N) {
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
                    //idxを更新
                    rep(j, 0, N) {
                        //白
                        int depth_j = idx_shiroana[j];
                        if (depth_i >= depth_j) continue;
                        //jの方が深い場合、一つ浅くする必要がある
                        idx_shiroana[j]--;
                    }
                    rep(j, 0, N) {
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
                //idxを更新
                rep(j, 0, N) {
                    //白
                    int depth_j = idx_shiroana[j];
                    if (depth_i >= depth_j) continue;
                    //jの方が深い場合、一つ浅くする必要がある
                    idx_shiroana[j]--;
                }
                rep(j, 0, N) {
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

            //3.操作列を出力
            int sz_order = order.size();
            cout << sz_order << endl;
            rep(i, 0, sz_order) {
                cout << order[i].first << " " << order[i].second << endl;
            }
        }
    }

}