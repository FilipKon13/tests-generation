#include "doctest.h"
#include <set>

#include <testgen/graph.hpp>
using namespace test;
using namespace std;

static_assert(is_nothrow_move_constructible_v<Graph>);
static_assert(is_nothrow_move_assignable_v<Graph>);

static_assert(is_nothrow_copy_assignable_v<Tree>);
static_assert(is_nothrow_copy_constructible_v<Tree>);
static_assert(is_nothrow_move_assignable_v<Tree>);
static_assert(is_nothrow_move_constructible_v<Tree>);

static_assert(is_nothrow_copy_assignable_v<Clique>);
static_assert(is_nothrow_copy_constructible_v<Clique>);
static_assert(is_nothrow_move_assignable_v<Clique>);
static_assert(is_nothrow_move_constructible_v<Clique>);

static_assert(is_nothrow_copy_assignable_v<Path>);
static_assert(is_nothrow_copy_constructible_v<Path>);
static_assert(is_nothrow_move_assignable_v<Path>);
static_assert(is_nothrow_move_constructible_v<Path>);

static_assert(is_nothrow_copy_assignable_v<Cycle>);
static_assert(is_nothrow_copy_constructible_v<Cycle>);
static_assert(is_nothrow_move_assignable_v<Cycle>);
static_assert(is_nothrow_move_constructible_v<Cycle>);

bool operator==(Graph a, Graph b) {
    for(auto & v : a) {
        sort(v.begin(), v.end());
    }
    for(auto & v : b) {
        sort(v.begin(), v.end());
    }
    return equal(a.begin(), a.end(), b.begin(), b.end());
}

uint dfs(uint w, Graph const & g, vector<bool> & vis, uint p, bool check_cycles) {
    vis[w] = true;
    uint res{1};
    for(auto v : g[w]) {
        if(!vis[v]) {
            res += dfs(v, g, vis, w, check_cycles);
        } else {
            REQUIRE_UNARY_FALSE(check_cycles && v != p);
        }
    }
    return res;
}

bool isConnected(Graph const & g, bool check_cycles = false) {
    auto n = g.size();
    vector<bool> vis(n, false);
    auto sz = dfs(0, g, vis, -1, check_cycles);
    return sz == n;
}

TEST_CASE("test_tree_structure") {
    int const n = 1000;
    gen_type gen{13};
    Graph const g = Tree(n).generate(gen);
    CHECK(g.size() == n);
    CHECK(isConnected(g, true));
}

TEST_CASE("test_tree_path") {
    int const n = 1000;
    gen_type gen{13};
    Graph const g = Tree(n, 1).generate(gen);    // range == 1 => path
    int two{};
    for(int i = 0; i < n; i++) {
        two += g[i].size() == 2 ? 1 : 0;
    }

    CHECK(two == n - 2);
    CHECK(isConnected(g, true));
}

TEST_CASE("test_path_not_random") {
    int const n = 1000;
    Graph const g = Path(n).generate();
    int cnt{0};
    for(int i = 0; i < n; ++i) {
        if(g[i].size() == 1) {
            ++cnt;
        } else if(g[i].size() != 2) {
            CHECK(false);
        }
    }
    CHECK(cnt == 2);
    CHECK(g[0].size() == 1);
    CHECK(g[n - 1].size() == 1);
    CHECK(isConnected(g, true));
}

TEST_CASE("test_path_random") {
    int const n = 1000;
    gen_type gen{14};
    Graph g = Path(n).generate(gen);
    int cnt{0};
    for(int i = 0; i < n; ++i) {
        if(g[i].size() == 1) {
            ++cnt;
        } else if(g[i].size() != 2) {
            CHECK(false);
        }
    }
    CHECK(cnt == 2);
    CHECK(isConnected(g, true));
}

TEST_CASE("test_clique_not_random") {
    int const n = 50;
    Graph const g = Clique(n).generate();

    for(int i = 0; i < n; i++) {
        CHECK(g[i].size() == n - 1);
    }
    CHECK(isConnected(g));
}

TEST_CASE("test_clique_random") {
    int const n = 50;
    gen_type gen{0};
    Graph g = Clique(n).generate(gen);
    for(int i = 0; i < n; i++) {
        CHECK(g[i].size() == n - 1);
    }
    CHECK(isConnected(g));
}

TEST_CASE("test_cycle_not_random") {
    int const n = 50;
    Graph const g = Cycle(n).generate();
    for(int i = 0; i < n; i++) {
        CHECK(g[i].size() == 2);
    }
    CHECK(isConnected(g));
}

TEST_CASE("test_cycle_random") {
    int const n = 50;
    gen_type gen{0};
    Graph const g = Cycle(n).generate(gen);
    for(int i = 0; i < n; i++) {
        CHECK(g[i].size() == 2);
    }
    CHECK(isConnected(g));
}

TEST_CASE("test_merge") {
    int const n = 50;
    Graph const a = Clique(n).generate();
    Graph const b = Path(n).generate();
    SUBCASE("initializer list empty") {
        Graph const c = merge(a, b, {});
        for(int i = 0; i < n; ++i) {
            CHECK(c[i].size() == n - 1);
        }
        CHECK(c[n].size() == 1);
        for(int i = n + 1; i < 2 * n - 1; i++) {
            CHECK(c[i].size() == 2);
        }
        CHECK(c[2 * n - 1].size() == 1);
    }
    SUBCASE("initializer list nonempty") {
        Graph const c = merge(a, b, {{0, 0}, {0, 1}});
        for(int i = 1; i < n; ++i) {
            CHECK(c[i].size() == n - 1);
        }
        for(int i = n + 2; i < 2 * n - 1; ++i) {
            CHECK(c[i].size() == 2);
        }
        CHECK(c[0].size() == n + 1);
        CHECK(c[n].size() == 2);
        CHECK(c[n + 1].size() == 3);
    }
    SUBCASE("vector of pair empty") {
        Graph const c = merge(a, b, vector<pair<uint, uint>>{});
        for(int i = 0; i < n; ++i) {
            CHECK(c[i].size() == n - 1);
        }
        CHECK(c[n].size() == 1);
        for(int i = n + 1; i < 2 * n - 1; i++) {
            CHECK(c[i].size() == 2);
        }
        CHECK(c[2 * n - 1].size() == 1);
    }
    SUBCASE("vector of pair nonempty") {
        Graph const c = merge(a, b, vector<pair<uint, uint>>{{0, 0}, {0, 1}});
        for(int i = 1; i < n; ++i) {
            CHECK(c[i].size() == n - 1);
        }
        for(int i = n + 2; i < 2 * n - 1; ++i) {
            CHECK(c[i].size() == 2);
        }
        CHECK(c[0].size() == n + 1);
        CHECK(c[n].size() == 2);
        CHECK(c[n + 1].size() == 3);
    }
}

TEST_CASE("test_remove_isolated") {
    Graph g(5);
    g.addEdge(0, 1);
    g.addEdge(3, 4);
    g.removeIsolated();
    CHECK(g.size() == 4);
    CHECK(g[0] == vector<uint>{1});
    CHECK(g[1] == vector<uint>{0});
    CHECK(g[2] == vector<uint>{3});
    CHECK(g[3] == vector<uint>{2});
}

TEST_CASE("test_make_simple") {
    Graph g(5);
    g.addEdge(0, 0);
    g.addEdge(1, 2);
    g.addEdge(1, 2);
    g.makeSimple();
    CHECK(g.size() == 5);
    CHECK(g[0].empty());
    CHECK(g[1] == vector<uint>{2});
    CHECK(g[2] == vector<uint>{1});
    CHECK(g[3].empty());
    CHECK(g[4].empty());
}

TEST_CASE("test_get_edges") {
    Graph const g = Clique(5).generate();
    Graph res(5);
    for(auto [a, b] : g.getEdges()) {
        res.addEdge(a, b);
    }
    CHECK(g == res);
}

TEST_CASE("test_contract") {
    Graph g = Clique(5).generate();
    Graph const exp = merge(Clique(4).generate(), Graph(1));
    g.contract(0, 4);
    g.makeSimple();
    CHECK(g == exp);
}

TEST_CASE("test_identify") {
    Graph const g = Clique(3).generate();
    Graph const P = Path(3).generate();
    Graph const exp = merge(Clique(3).generate(), Graph(1), {{0, 0}, {2, 0}});
    CHECK(identify(g, P, {{0, 0}, {2, 2}}) == exp);
}