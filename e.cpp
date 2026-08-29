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

int N;
vector<string> steps;

int dy[] = {-1, 1, 0, 0};
int dx[] = {0, 0, -1, 1};

// 盤面内で一番小さい数の座標を求める
pii searchmin(vvi grid) {
    int m = 1e9;
    pii midx = {-1, -1};
    rep(i, 0, 5) {
        rep(j, 4, 8) { // 4~7
            if (grid[i][j] != -1 && grid[i][j] < m) {
                m = grid[i][j];
                midx = {i, j};
            }
        }
    }
    return midx;
}

// 盤面内で二点の移動時を再現する経路文字列を作る
string makepath(vvi grid, int sy, int sx, int ty, int tx) {
    if (sy == ty && sx == tx) return "";

    vector<vector<pair<int, int>>> parent(5, vpii(9));
    vvb seen(5, vb(9, false));
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

            if (ny < 0 || 5 <= ny || nx < 4 || 9 <= nx || seen[ny][nx]) continue;

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
    // 1.入力受付
    cin >> N;
    vvi grid;
    grid.resize(5, vi(9, -1));
    steps.resize(N);
    rep(i, 0, N) {
        // 工程2の結果を予めgridに反映させる
        rep(j, 0, N) cin >> grid[i][7-j];
    }

    // 2.全てのクレーンを動かす
    string initsteps = "PRRRQLLLPRRQLLPRQL";
    steps[0] = initsteps;
    rep(i, 1, N) {
        steps[i] = initsteps + 'B'; // 小クレーンはここで爆破
    }

    pii crane0idx = {0, 4}; // クレーン0の座標

    // 以下を25回繰り返す
    rep(t, 0, 25) {
        // 3. 搬入口が最初から空いている行があれば詰める
        rep(i, 0, N) {
            if (grid[i][4] == -1 && grid[i][3] != -1) {
                grid[i][4] = grid[i][3];
                grid[i][3] = -1;
            }
        }

        // 3.5. 搬入口(x=4)が塞がっており、x=3に待ちコンテナがある場合、右側の最も近い空きマス(x=5~7)へ退避させる
        rep(i, 0, N) {
            if (grid[i][4] != -1 && grid[i][3] != -1) {
                int empty_x = -1;
                rep(j, 5, 8) { // 右隣から順に空きマス(x=5, 6, 7)を検索
                    if (grid[i][j] == -1) {
                        empty_x = j;
                        break;
                    }
                }

                // 空きマスがあれば退避を実行して x=3 から即座に繰り出す
                if (empty_x != -1) {
                    string Pstep = makepath(grid, crane0idx.first, crane0idx.second, i, 4);
                    steps[0] += Pstep + 'P';

                    string Qstep = makepath(grid, i, 4, i, empty_x);
                    steps[0] += Qstep + 'Q';

                    grid[i][empty_x] = grid[i][4];
                    grid[i][4] = grid[i][3]; // x=3 から補填
                    grid[i][3] = -1;

                    crane0idx = {i, empty_x};
                    break; // 1つ退避させたらループを抜けて、盤面が更新された状態で searchmin へ進む
                }
            }
        }

        // 6. 盤面内で一番小さい数の座標を求める
        pii curminidx = searchmin(grid);
        if (curminidx.first == -1) break;

        // 7. 0番目のクレーンをそこに向かわせる
        string Pstep = makepath(grid, crane0idx.first, crane0idx.second, 
            curminidx.first, curminidx.second);

        steps[0] += Pstep + 'P'; // コンテナを掴む
        crane0idx = curminidx;

        // 8. 目的の搬出口に向かう
        int curtarget = grid[curminidx.first][curminidx.second];
        int lane = curtarget / N;

        string Qstep = makepath(grid, curminidx.first, curminidx.second, 
            lane, 8);

        steps[0] += Qstep + 'Q'; // コンテナを離す
        crane0idx = {lane, 8};
        grid[curminidx.first][curminidx.second] = -1;
    }

    // 9. 出力
    rep(i, 0, N) {
        cout << steps[i] << endl;
    }
}