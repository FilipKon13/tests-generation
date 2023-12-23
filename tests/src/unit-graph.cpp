#include "doctest.h"
#include <graph.hpp>
using namespace test;
using namespace std;


int dfs(int w, int p, Graph const & G, vector<bool> & vis) {
    vis[w] = true;
    int res{1};
    for(auto v : G[w])  if(v != p) {
        REQUIRE_FALSE(vis[v]); /* no cycles in tree */
        res += dfs(v, w, G, vis);
    }
    return res;
}

TEST_CASE("test_tree_structure") {
    int const n = 1000;
    gen_type gen{13};
    Graph const G = Tree(n, false).generate(gen);
    vector<bool> vis(n+1, false);

    CHECK(G.size() == n);
    CHECK(dfs(1, -1, G, vis) == n); /* connected */
}

TEST_CASE("test_tree_path") {
    int const n = 1000;
    gen_type gen{13};
    Graph const G = Tree(n, true, 1).generate(gen); /* range == 1 => path */
    vector<bool> vis(n+1, false);

    CHECK(dfs(1, -1, G, vis) == n); /* connected */
    int two{};
    for(int i=1;i<=n;i++) two += G[i].size() == 2;
    CHECK(two == n - 2);
}

TEST_CASE("test_path_not_random") {
    int const n = 1000;
    gen_type gen{14};
    Graph const G = Path(n, false).generate(gen);
    int cnt{0};
    for(int i=1;i<=n;++i) {
        if(G[i].size() == 1)    ++cnt;
        else if(G[i].size() == 2);
        else CHECK(false);
    }
    CHECK(cnt == 2);
    CHECK(G[1].size() == 1);
    CHECK(G[n].size() == 1);
}

TEST_CASE("test_path_random") {
    int const n = 1000;
    gen_type gen{14};
    Graph const G = Path(n, true).generate(gen);
    int cnt{0};
    for(int i=1;i<=n;++i) {
        if(G[i].size() == 1)    ++cnt;
        else if(G[i].size() == 2);
        else CHECK(false);
    }
    CHECK(cnt == 2);
}

TEST_CASE("test_clique_not_random") {
    int const n = 50;
    gen_type gen{15};
    Graph G = Clique(n, false).generate(gen);

    for(int i=1;i<=n;i++)   CHECK(G[i].size() == n-1);
}

TEST_CASE("test_clique_random") {
    int const n = 50;
    gen_type gen{15};
    Graph const G = Clique(n, true).generate(gen);

    for(int i=1;i<=n;i++)   CHECK(G[i].size() == n-1);
}