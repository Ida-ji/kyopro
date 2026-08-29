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

int dy[] = {-1, 1, 0, 0};
int dx[] = {0, 0, -1, 1};

random_device rd;

// マス(sy, sx)から(ty, tx)への最短経路文字列を取得
string getpath(int sy, int sx, int ty, int tx) {
    if (sy == ty && sx == tx) return "";

    vector<vector<pair<int, int>>> parent(N, vpii(N));
    vvb seen(N, vb(N, false));
    queue<pair<int, int>> q;

    q.push({sy, sx});
    seen[sy][sx] = true;

    while (!q.empty()) {
        auto [cy, cx] = q.front();
        q.pop();

        if (cy == ty && cx == tx) break;

        rep(dir, 0, 4) {
            int ny = cy + dy[dir];
            int nx = cx + dx[dir];

            if (ny < 0 || N <= ny || nx < 0 || N <= nx || seen[ny][nx]) continue;

            parent[ny][nx] = {cy, cx};
            seen[ny][nx] = true;
            q.push({ny, nx});
        }
    }

    // 経路復元
    string ret = "";
    int cy = ty, cx = tx;
    while (cy != sy || cx != sx) { 
        auto [py, px] = parent[cy][cx];
        if (cy - py == -1 && cx == px) ret += 'U';
        else if (cy - py == 1 && cx == px) ret += 'D';
        else if (cy == py && cx - px == -1) ret += 'L';
        else if (cy == py && cx - px == 1) ret += 'R';

        cy = py;
        cx = px;
    }

    reverse(ret.begin(), ret.end());
    return ret;
}

int main() {
    //1.入力受取
    cin >> N;
    vvi h(N, vi(N));

    rep(i, 0, N) {
        rep(j, 0, N) {
            cin >> h[i][j];
        }    
    } 

    pii trackpos = {0, 0};
    int trackd = 0;
    vector<string> order(0);
    rep(t, 0, 500) {
        //2.高いマスを見る
        int r1y = rng.rand_int(N);
        int r1x = rng.rand_int(N);
        int it = 0;
        while (h[r1y][r1x] <= 0) {
            r1y = rng.rand_int(N);
            r1x = rng.rand_int(N);
            it++;
            if (it > 500) {
                //500回やっても見つからなかったら、
                //もう盤面に高いマスは残ってないと言っていい
                break;
            }
        }

        //イテレータ上限に達してなお高いマスでない場合
        if (h[r1y][r1x] <= 0) {
            if (trackd == 0) goto ex;
            else {
                //盤面に高いマスは無いが、トラックに積んである土がある場合
                //トラックは何もしない
            }
        }
        //高いマスである場合（正常）
        else {
            //3.高いマスへ移動する
            string path1 = getpath(trackpos.first, trackpos.second, r1y, r1x);
            for (auto c : path1) cout << c << endl;
            trackpos = {r1y, r1x};
            //4.土を積む
            trackd += h[r1y][r1x];
            cout << "+" << h[r1y][r1x] << endl;
            h[r1y][r1x] = 0;
        }


        //5.低いマスを見る
        int r2y = rng.rand_int(N);
        int r2x = rng.rand_int(N);
        int it2 = 0;
        while (h[r2y][r2x] >= 0) {
            r2y = rng.rand_int(N);
            r2x = rng.rand_int(N);
            it2++;
        }

        //6.低いマスへ移動する
        string path2 = getpath(trackpos.first, trackpos.second, r2y, r2x);
        for (auto c : path2) cout << c << endl;
        trackpos = {r2y, r2x};
        //4.土を降ろす
        //min(-h[...][...], trackd)
        int amount = min(-h[r2y][r2x], trackd);
        trackd -= amount;
        h[r2y][r2x] += amount;
        cout << "-" << amount << endl;
    }ex:

    rep(i, 0, N) {
        rep(j, 0, N) {
            if (h[i][j] != 0) {
                //デバッグ用
                cout << "まだ盤面が綺麗になっていません" << endl;
            }
        }
    }
}