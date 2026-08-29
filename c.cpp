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
    cin >> N;
    vvi h(N, vi(N));

    rep(i, 0, N) {
        rep(j, 0, N) {
            cin >> h[i][j];
        }    
    } 

    pii trackpos = {0, 0};
    int trackd = 0;

    // 候補マスの中から移動コスト（距離）が最小のマスを1つ選ぶ関数
    auto select_best_candidate = [&](bool is_positive_target) -> pii {
        vpii candidates;
        rep(i, 0, N) {
            rep(j, 0, N) {
                if (is_positive_target && h[i][j] > 0) {
                    candidates.push_back({i, j});
                } else if (!is_positive_target && h[i][j] < 0) {
                    candidates.push_back({i, j});
                }
            }
        }

        if (candidates.empty()) return {-1, -1};

        // 候補数が20個を超える場合はランダムに10個だけ抽出
        if (candidates.size() > 20) {
            rep(i, 0, 20) {
                int swap_idx = i + rng.rand_int((int)candidates.size() - i);
                swap(candidates[i], candidates[swap_idx]);
            }
            candidates.resize(10);
        }

        // 抽出された候補（最大10個）の中でマンハッタン距離が最も近いマスを選択
        pii best_pos = candidates[0];
        int min_dist = abs(trackpos.first - candidates[0].first) + abs(trackpos.second - candidates[0].second);

        for (size_t i = 1; i < candidates.size(); i++) {
            int dist = abs(trackpos.first - candidates[i].first) + abs(trackpos.second - candidates[i].second);
            if (dist < min_dist) {
                min_dist = dist;
                best_pos = candidates[i];
            }
        }
        return best_pos;
    };

    rep(t, 0, 500) {
        // 高いマスの候補選定
        pii target_high = select_best_candidate(true);

        if (target_high.first == -1) {
            if (trackd == 0) goto ex;
        } else {
            int r1y = target_high.first;
            int r1x = target_high.second;

            string path1 = getpath(trackpos.first, trackpos.second, r1y, r1x);
            for (auto c : path1) cout << c << endl;
            trackpos = {r1y, r1x};

            trackd += h[r1y][r1x];
            cout << "+" << h[r1y][r1x] << endl;
            h[r1y][r1x] = 0;
        }

        // 低いマスの候補選定
        pii target_low = select_best_candidate(false);
        if (target_low.first == -1) continue;

        int r2y = target_low.first;
        int r2x = target_low.second;

        string path2 = getpath(trackpos.first, trackpos.second, r2y, r2x);
        for (auto c : path2) cout << c << endl;
        trackpos = {r2y, r2x};

        int amount = min(-h[r2y][r2x], trackd);
        trackd -= amount;
        h[r2y][r2x] += amount;
        cout << "-" << amount << endl;
    }
    
ex:
    return 0;
}