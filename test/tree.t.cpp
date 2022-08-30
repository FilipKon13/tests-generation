#include "test.hpp"
#include <graph.hpp>
using namespace test;
using namespace std;

int dfs(int w, int p, Graph const & G, vector<bool> & vis) {
    vis[w] = true;
    int res{1};
    for(auto v : G[w])  if(v!=p) {
        assert(!vis[v]); /* no cycles in tree */
        res += dfs(v, w, G, vis);
    }
    return res;
}

void test_tree_structure() {
    gen_type gen{13};
    const int n = 1000;
    Graph G = Tree(n, false).generate(gen);
    vector<bool> vis(n+1, false);

    assert(G.size() == n);
    assert(dfs(1, -1, G, vis) == n); /* connected */
}

void test_path() {
    gen_type gen{13};
    const int n = 1000;
    Graph G = Tree(n, true, 1).generate(gen); /* range == 1 => path */
    vector<bool> vis(n+1, false);

    assert(dfs(1, -1, G, vis) == n); /* connected */
    assert(G[1].size() == 2); /* 1 is not a endpoint, 0.1% chance of fail */
}

int main() {
    test_tree_structure();
    test_path();

    TEST_OK();
    return 0;
}