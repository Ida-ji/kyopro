#include <bits/stdc++.h>
using ll = long long;
using namespace std;

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

//焼きなまし
int main() {
    
    //時間管理の準備
    auto start = chrono::steady_clock::now();
    const double TIME_LIMIT = 1900.0; // 制限時間 (ms)
    
    //温度パラメータの設定
    const double START_TEMP = 500.0;
    const double END_TEMP = 1.0;
    double temp = START_TEMP;
    
    long long current_score = 0; // 初期解のスコスコア
    long long best_score = current_score;

    int loop_count = 0;
    
    while (true) {
        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration_cast<chrono::milliseconds>(now - start).count();
        if (elapsed >= TIME_LIMIT) break;
            
        // 温度の更新（線形冷却）
        double progress = elapsed / TIME_LIMIT;
        temp = START_TEMP + (END_TEMP - START_TEMP) * progress;
        
        loop_count++;

        //...
        //状態の変形とスコア計算

        //...
        //遷移を受け入れるかどうかの判定
        int new_score;
        long long score_diff = new_score - current_score; 
        
        bool accept = false;
        if (score_diff >= 0) {
            accept = true;
        } else {
            //悪くなっても確率的に許容
            double rand_real = rng.rand_double(); 
            double prob = exp((double)score_diff / temp);
            
            if (rand_real < prob) {
                accept = true;
            }
        }

        //状態の確定or差し戻し
        if (accept) {
            best_score = current_score;
            //他にもベスト状態の保存
            //...
        } else {
            //変更差し戻し
            //...
        }
    }
}