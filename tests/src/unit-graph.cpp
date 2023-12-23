#include "doctest.h"
#include <graph.hpp>
using namespace test;
using namespace std;

#define assert(...) ;

int dfs(int w, int p, Graph const & G, vector<bool> & vis) {
    vis[w] = true;
    int res{1};
    for(auto v : G[w])  if(v != p) {
        assert(!vis[v]); /* no cycles in tree */
        res += dfs(v, w, G, vis);
    }
    return res;
}

TEST_CASE("test_tree_structure") {
    gen_type gen{13};
    const int n = 1000;
    Graph G = Tree(n, false).generate(gen);
    vector<bool> vis(n+1, false);

    CHECK(G.size() == n);
    CHECK(dfs(1, -1, G, vis) == n); /* connected */
}

void test_tree_path() {
    gen_type gen{13};
    const int n = 1000;
    Graph G = Tree(n, true, 1).generate(gen); /* range == 1 => path */
    vector<bool> vis(n+1, false);

    assert(dfs(1, -1, G, vis) == n); /* connected */
    int two{};
    for(int i=1;i<=n;i++) two += G[i].size() == 2;
    assert(two == n - 2);
}

void test_tree() {
    // test_tree_structure();
    test_tree_path();
}

void test_path_not_random() {
    const int n = 1000;
    gen_type gen{14};
    Graph G = Path(n, false).generate(gen);
    int cnt{0};
    for(int i=1;i<=n;++i) {
        if(G[i].size() == 1)    ++cnt;
        else if(G[i].size() == 2);
        else assert(false);
    }
    assert(cnt == 2);
    assert(G[1].size() == 1);
    assert(G[n].size() == 1);
}

void test_path_random() {
    const int n = 1000;
    gen_type gen{14};
    Graph G = Path(n, true).generate(gen);
    int cnt{0};
    for(int i=1;i<=n;++i) {
        if(G[i].size() == 1)    ++cnt;
        else if(G[i].size() == 2);
        else assert(false);
    }
    assert(cnt == 2);
}

void test_path() {
    test_path_not_random();
    test_path_random();
}

void test_clique_not_random() {
    const int n = 50;
    gen_type gen{15};
    Graph G = Clique(n, false).generate(gen);

    for(int i=1;i<=n;i++)   assert(G[i].size() == n-1);
}

void test_clique_random() {
    const int n = 50;
    gen_type gen{15};
    Graph G = Clique(n, true).generate(gen);

    for(int i=1;i<=n;i++)   assert(G[i].size() == n-1);
}

void test_clique() {
    test_clique_random();
    test_clique_not_random();
}