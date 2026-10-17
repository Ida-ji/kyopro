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
//壁などが無い場合 && 途中をあまり気にしなくて良い場合はO(1)で取得可能
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

    //retをシャッフルして多様性を持たせたい時にはこちら
    //shuffle(ret.begin(), ret.end(), rng);
    return ret;
}