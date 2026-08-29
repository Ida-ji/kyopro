#include <bits/stdc++.h>
using ll = long long;
using namespace std;

//焼きなまし
int main() {

    //乱数
    random_device rd;
    mt19937 gen(rd());
    
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
            //mt()の結果を最大値で割って [0.0, 1.0) の実数を作る（高速！）
            double rand_real = (double)rd() / 4294967295.0; 
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