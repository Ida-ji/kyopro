#include <bits/stdc++.h>
using ll = long long;
using namespace std;

// エラトステネスの篩
// 返り値: is_prime[i] が true ならば i は素数
// primes には N 以下の素数が昇順で格納される
vector<bool> sieve(int N, vector<ll>& primes) {
    vector<bool> is_prime(N + 1, true);
    is_prime[0] = is_prime[1] = false; // 0 と 1 は素数ではない
    
    primes.clear();
    
    for (int p = 2; p <= N; ++p) {
        if (!is_prime[p]) continue;
        
        primes.push_back(p);
        
        // p * p から始めることで、すでに消されている倍数の重複チェックを回避（オーバーフローに注意）
        if ((ll)p * p <= N) {
            for (int i = p * p; i <= N; i += p) {
                is_prime[i] = false;
            }
        }
    }
    return is_prime;
}


// factorize: int Mを与えて、Mの素因数分解のmapを返す関数
// 構造体としてまとめておくと使いやすい
struct LinearSieve {
    int n;
    vector<int> min_factor; // min_factor[i] は i の最小の素因数

    LinearSieve(int n) : n(n), min_factor(n + 1) {
        for (int i = 0; i <= n; ++i) min_factor[i] = i;
        for (int p = 2; p * p <= n; ++p) {
            if (min_factor[p] == p) { // p が素数のとき
                for (int i = p * p; i <= n; i += p) {
                    if (min_factor[i] == i) {
                        min_factor[i] = p;
                    }
                }
            }
        }
    }

    // 高速素因数分解: O(log M)
    // 返り値: {素因数, 指数} の map
    map<int, int> factorize(int m) {
        map<int, int> res;
        while (m > 1) {
            res[min_factor[m]]++;
            m /= min_factor[m];
        }
        return res;
    }
};
//使用例：int target; auto factors = sieve.factorize(target);
