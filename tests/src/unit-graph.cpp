#include "doctest.h"
#include <testgen/graph.hpp>
using namespace test;
using namespace std;

static_assert(std::is_move_constructible<Graph>::value);
static_assert(std::is_nothrow_move_constructible<Graph>::value);
static_assert(std::is_move_assignable<Graph>::value);

int dfs(int w, int p, Graph const & G, vector<bool> & vis) {
    vis[w] = true;
    int res{1};
    for(auto v : G[w]) {
        if(v != p) {
            REQUIRE_FALSE(vis[v]); /* no cycles in tree */
            res += dfs(v, w, G, vis);
        }
    }
    return res;
}

TEST_CASE("test_tree_structure") {
    int const n = 1000;
    gen_type gen{13};
    Graph const G = Tree(n).generate(gen);
    vector<bool> vis(n, false);

    int sz = dfs(0, -1, G, vis);

    CHECK(G.size() == n);
    CHECK(sz == n);    // connected
}

TEST_CASE("test_tree_path") {
    int const n = 1000;
    gen_type gen{13};
    Graph const G = Tree(n, 1).generate(gen);    // range == 1 => path
    vector<bool> vis(n, false);

    int sz = dfs(0, -1, G, vis);
    int two{};
    for(int i = 0; i < n; i++) {
        two += G[i].size() == 2;
    }

    CHECK(sz == n);    // connected
    CHECK(two == n - 2);
}

TEST_CASE("test_path_not_random") {
    int const n = 1000;
    gen_type gen{14};
    Graph const G = Path(n).generate(gen);
    int cnt{0};
    for(int i = 0; i < n; ++i) {
        if(G[i].size() == 1) {
            ++cnt;
        } else if(G[i].size() != 2) {
            CHECK(false);
        }
    }
    CHECK(cnt == 2);
    CHECK(G[0].size() == 1);
    CHECK(G[n - 1].size() == 1);
}

TEST_CASE("test_path_random") {
    int const n = 1000;
    gen_type gen{14};
    Graph G = Path(n).generate(gen);
    G.permute(gen);
    int cnt{0};
    for(int i = 0; i < n; ++i) {
        if(G[i].size() == 1) {
            ++cnt;
        } else if(G[i].size() != 2) {
            CHECK(false);
        }
    }
    CHECK(cnt == 2);
}

TEST_CASE("test_clique_not_random") {
    int const n = 50;
    gen_type gen{15};
    Graph G = Clique(n).generate(gen);

    for(int i = 0; i < n; i++) {
        CHECK(G[i].size() == n - 1);
    }
}

TEST_CASE("test_clique_random") {
    int const n = 50;
    gen_type gen{15};
    Graph const G = Clique(n).generate(gen);

    for(int i = 0; i < n; i++) {
        CHECK(G[i].size() == n - 1);
    }
}

TEST_CASE("test_merge") {
    int const n = 50;
    gen_type gen{0};
    Graph const A = Clique(n).generate(gen);
    Graph const B = Path(n).generate(gen);
    SUBCASE("initializer list empty") {
        Graph const C = merge(A, B, {});
        for(int i = 0; i < n; ++i) {
            CHECK(C[i].size() == n - 1);
        }
        CHECK(C[n].size() == 1);
        for(int i = n + 1; i < 2 * n - 1; i++) {
            CHECK(C[i].size() == 2);
        }
        CHECK(C[2 * n - 1].size() == 1);
    }
    SUBCASE("initializer list nonempty") {
        Graph const C = merge(A, B, {{0, 0}, {0, 1}});
        for(int i = 1; i < n; ++i) {
            CHECK(C[i].size() == n - 1);
        }
        for(int i = n + 2; i < 2 * n - 1; ++i) {
            CHECK(C[i].size() == 2);
        }
        CHECK(C[0].size() == n + 1);
        CHECK(C[n].size() == 2);
        CHECK(C[n + 1].size() == 3);
    }
    SUBCASE("vector of pair empty") {
        Graph const C = merge(A, B, std::vector<std::pair<int, int>>{});
        for(int i = 0; i < n; ++i) {
            CHECK(C[i].size() == n - 1);
        }
        CHECK(C[n].size() == 1);
        for(int i = n + 1; i < 2 * n - 1; i++) {
            CHECK(C[i].size() == 2);
        }
        CHECK(C[2 * n - 1].size() == 1);
    }
    SUBCASE("vector of pair nonempty") {
        Graph const C = merge(A, B, std::vector<std::pair<int, int>>{{0, 0}, {0, 1}});
        for(int i = 1; i < n; ++i) {
            CHECK(C[i].size() == n - 1);
        }
        for(int i = n + 2; i < 2 * n - 1; ++i) {
            CHECK(C[i].size() == 2);
        }
        CHECK(C[0].size() == n + 1);
        CHECK(C[n].size() == 2);
        CHECK(C[n + 1].size() == 3);
    }
}