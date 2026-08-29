#include <bits/stdc++.h>
using ll = long long;
using namespace std;

//乱数生成
//一般的なやつ
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> distrib(1, 6);


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

int main() {
  rng.rand_int(6); //0, 1, 2, 3, 4, 5
  rng.rand_double();
  cout << distrib(gen) << endl;
}  