#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vb = vector<bool>;
using vvi = vector<vector<int>>;
using vvb = vector<vector<bool>>;

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
    iota(idx.begin(), idx.end(), 0); //0~N*Nを生成
    shuffle(idx.begin(), idx.end(), rng); //シャッフル

    vector<int> vec = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    vector<int> result;
    unordered_set<int> seen;

    for (int n : vec) {
        if (seen.insert(n).second) { // すでに存在していなければ挿入成功
            result.push_back(n);
        }
    }
}
