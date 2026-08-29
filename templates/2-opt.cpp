#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using vi = vector<int>;
using vvi = vector<vector<int>>;

int N;
vi A; 

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> dist_n(0, N-1);
uniform_real_distribution<double> dist_real(0.0, 1.0);

//1次元配列におけるswapとinsert
//確率半半
int main() {
    //...
    int i1 = dist_n(gen), i2 = dist_n(gen);
        
    //遷移処理
    if (dist_real(gen) < 0.5) swap(A[i1], A[i2]);
    else {
        int val = A[i1];
        A.erase(A.begin() + i1);
        A.insert(A.begin() + i2, val);
    }
}        