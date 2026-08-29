#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vb = vector<bool>;
using vvi = vector<vector<int>>;
using vvb = vector<vector<bool>>;

#define rep(i, s, t) for (int i = s; i < t; i++)

int N;
int main() {
    std::vector<int> vec = {10, 20, 30, 40};
    
    // 2番目の要素（20）を削除
    vec.erase(vec.begin() + 1); 
    
    // vec は {10, 30, 40} になる

    vec.pop_back(); //配列の末尾を削除する

    set<pair<int, int>> cell_set;
    if (!cell_set.count({1, 3})) {
        //のように、setに対して存在判定はcount
    }

    // 'a' を 5回繰り返した文字列sを作る
    string s(5, 'a');

    vi idx(N*N);
    random_device rd;
    iota(idx.begin(), idx.end(), 0); //0~N*Nを生成
    shuffle(idx.begin(), idx.end(), rd); //シャッフル

    vector<int> vec = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    vector<int> result;
    unordered_set<int> seen;

    for (int n : vec) {
        if (seen.insert(n).second) { // すでに存在していなければ挿入成功
            result.push_back(n);
        }
    }
}
