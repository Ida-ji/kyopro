#include <bits/stdc++.h>
using ll = long long;
using namespace std;

//UnionFind
struct UnionFind {
    vector<int> par; //par[i] はiの親の番号

    UnionFind(int n) : par(n, -1) {} //最初はみんなリーダー（-1）

    //リーダー（根）を探す
    int find(int x) {
        if (par[x] < 0) return x;
        return par[x] = find(par[x]); //経路圧縮：親を直接リーダーに繋ぎ変える
    }

    //AとBを合体させる
    bool unite(int x, int y) {
        x = find(x); y = find(y);
        if (x == y) return false; //既に同じグループ
        //小さい方の木を、大きい方の木にくっつける
        if (par[x] > par[y]) swap(x, y);
        par[x] += par[y];
        par[y] = x;
        return true;
    }

    //AとBが同じグループか判定
    bool same(int x, int y) {
        return find(x) == find(y);
    }

    int size(int x) {
        return -par[find(x)]; // xが含まれるグループのサイズを返す
    }
};