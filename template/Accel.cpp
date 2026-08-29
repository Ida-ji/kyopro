#include <bits/stdc++.h>
using ll = long long;
using namespace std;
//重要：しても早くならないケースもある

//コンパイル最適化
#pragma GCC optimize("O3,unroll-loops")

int main() {
    // 高速入出力
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    //...

    //ビームサーチにおいて、コピーは避けよう
    //for (auto &cb : beams) {

    //いつどこで更新がされるのかを見極め、
    //更新される可能性が低くなってきたら枝刈りしよう
    //(例：配列をソートした後全探索する必要は、ないかもしれない)

    //グリッドの壁のところを全て障害物判定にして、[N+2][N+2]の新たな盤面にすると、
    //盤面の内部判定が要らなくなるかもしれない

    //キューは動的メモリ確保が重いので、静的配列キューにする

    bool visited[50][50];
    memset(visited, false, sizeof(visited)); // 高速ゼロクリア
    //intの時は memset(visited, 0, sizeof(visited)); 

}