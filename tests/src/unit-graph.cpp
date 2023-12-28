#include "doctest.h"
#include <testgen/graph.hpp>
#include <set>
using namespace test;
using namespace std;

static_assert(is_move_constructible<Graph>::value);
static_assert(is_nothrow_move_constructible<Graph>::value);
static_assert(is_move_assignable<Graph>::value);

bool operator==(Graph A, Graph B) {
    for(auto & V : A) {
        sort(V.begin(), V.end());
    }
    for(auto & V : B) {
        sort(V.begin(), V.end());
    }
    return equal(A.begin(), A.end(), B.begin(), B.end());
}

uint dfs(uint w, uint p, Graph const & G, vector<bool> & vis, bool check_cycles) {
    vis[w] = true;
    uint res{1};
    for(auto v : G[w]) {
        if(!vis[v]) {
            res += dfs(v, w, G, vis, check_cycles);
        } else {
            REQUIRE_UNARY_FALSE(check_cycles && v != p);
        }
    }
    return res;
}

bool isConnected(Graph const & G, bool check_cycles = false) {
    auto n = G.size();
    vector<bool> vis(n, false);
    auto sz = dfs(0, -1, G, vis, check_cycles);
    return sz == n;
}

TEST_CASE("test_tree_structure") {
    int const n = 1000;
    gen_type gen{13};
    Graph const G = Tree(n).generate(gen);
    CHECK(G.size() == n);
    CHECK(isConnected(G, true));
}

TEST_CASE("test_tree_path") {
    int const n = 1000;
    gen_type gen{13};
    Graph const G = Tree(n, 1).generate(gen);    // range == 1 => path
    int two{};
    for(int i = 0; i < n; i++) {
        two += G[i].size() == 2;
    }

    CHECK(two == n - 2);
    CHECK(isConnected(G, true));
}

TEST_CASE("test_path_not_random") {
    int const n = 1000;
    Graph const G = Path(n).generate();
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
    CHECK(isConnected(G, true));
}

TEST_CASE("test_path_random") {
    int const n = 1000;
    gen_type gen{14};
    Graph G = Path(n).generate(gen);
    int cnt{0};
    for(int i = 0; i < n; ++i) {
        if(G[i].size() == 1) {
            ++cnt;
        } else if(G[i].size() != 2) {
            CHECK(false);
        }
    }
    CHECK(cnt == 2);
    CHECK(isConnected(G, true));
}

TEST_CASE("test_clique_not_random") {
    int const n = 50;
    Graph const G = Clique(n).generate();

    for(int i = 0; i < n; i++) {
        CHECK(G[i].size() == n - 1);
    }
    CHECK(isConnected(G));
}

TEST_CASE("test_clique_random") {
    int const n = 50;
    gen_type gen{0};
    Graph G = Clique(n).generate(gen);
    for(int i = 0; i < n; i++) {
        CHECK(G[i].size() == n - 1);
    }
    CHECK(isConnected(G));
}

TEST_CASE("test_cycle_not_random") {
    int const n = 50;
    Graph const G = Cycle(n).generate();
    for(int i = 0; i < n; i++) {
        CHECK(G[i].size() == 2);
    }
    CHECK(isConnected(G));
}

TEST_CASE("test_cycle_random") {
    int const n = 50;
    gen_type gen{0};
    Graph const G = Cycle(n).generate(gen);
    for(int i = 0; i < n; i++) {
        CHECK(G[i].size() == 2);
    }
    CHECK(isConnected(G));
}

TEST_CASE("test_merge") {
    int const n = 50;
    Graph const A = Clique(n).generate();
    Graph const B = Path(n).generate();
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
        Graph const C = merge(A, B, vector<pair<uint, uint>>{});
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
        Graph const C = merge(A, B, vector<pair<uint, uint>>{{0, 0}, {0, 1}});
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

TEST_CASE("test_remove_isolated") {
    Graph G(5);
    G.addEdge(0, 1);
    G.addEdge(3, 4);
    G.removeIsolated();
    CHECK(G.size() == 4);
    CHECK(G[0] == vector<uint>{1});
    CHECK(G[1] == vector<uint>{0});
    CHECK(G[2] == vector<uint>{3});
    CHECK(G[3] == vector<uint>{2});
}

TEST_CASE("test_make_simple") {
    Graph G(5);
    G.addEdge(0, 0);
    G.addEdge(1, 2);
    G.addEdge(1, 2);
    G.makeSimple();
    CHECK(G.size() == 5);
    CHECK(G[0].empty());
    CHECK(G[1] == vector<uint>{2});
    CHECK(G[2] == vector<uint>{1});
    CHECK(G[3].empty());
    CHECK(G[4].empty());
}

TEST_CASE("test_get_edges") {
    Graph const G = Clique(5).generate();
    Graph res(5);
    for(auto [a, b] : G.getEdges()) {
        res.addEdge(a, b);
    }
    CHECK(G == res);
}

TEST_CASE("test_contract") {
    Graph G = Clique(5).generate();
    Graph exp = merge(Clique(4).generate(), Graph(1));
    G.contract(0, 4);
    G.makeSimple();
    CHECK(G == exp);
}

TEST_CASE("test_identify") {
    Graph G = Clique(3).generate();
    Graph P = Path(3).generate();
    Graph exp = merge(Clique(3).generate(), Graph(1), {{0, 0}, {2, 0}});
    CHECK(identify(G, P, {{0, 0}, {2, 2}}) == exp);
}