#include <bits/stdc++.h>
using ll = long long;
using namespace std;


//正規分布を計算する関数
double normal_cdf(double mean, double std_dev, double x) {
    return 0.5 * (1.0 + erf((x - mean) / (std_dev * sqrt(2.0))));
}

//平均と標準偏差と整数xを引数として、
//正規分布においてサンプリングしてroundでまとめた時にxとなるような確率を、
//doubleで返す関数
double get_round_prob(double mu, double sigma, int x) {
    if (sigma <= 0.0) {
        return (round(mu) == x) ? 1.0 : 0.0;
    }
    
    // 標準正規分布の累積分布関数(CDF)を計算するラムダ関数
    auto cdf = [&](double val) {
        return 0.5 * erfc(-(val - mu) / (sigma * sqrt(2.0)));
    };
    
    // [x - 0.5, x + 0.5)の範囲に収まる確率を返す
    return cdf(x + 0.5) - cdf(x - 0.5);
}