#ifndef TESTGEN_GRAPH_HPP_
#define TESTGEN_GRAPH_HPP_

#include "rand.hpp"

#include <algorithm>
#include <numeric>
#include <type_traits>
#include <vector>

namespace test {

class Graph {
    using container_t = std::vector<std::vector<uint>>;
    using edges_container_t = std::vector<std::pair<uint, uint>>;
    container_t g;

public:
    Graph() :
      Graph(0) {}
    explicit Graph(container_t::size_type n) :
      g{n} {}

    [[nodiscard]] std::vector<uint> & operator[](uint i) {
        return g[i];
    }

    [[nodiscard]] std::vector<uint> const & operator[](uint i) const {
        return g[i];
    }

    void addEdge(uint a, uint b) {
        g[a].push_back(b);
        if(a != b) {
            g[b].push_back(a);
        }
    }

    void permute(gen_type & gen) {
        auto const n = g.size();
        auto const per = get_permutation(n, gen);
        Graph new_G(g.size());
        for(uint w = 0; w < n; ++w) {
            for(auto v : (*this)[w]) {
                new_G[per[w]].push_back(per[v]);
            }
        }
        for(uint w = 0; w < n; ++w) {
            shuffle_sequence(std::begin(new_G[w]), std::end(new_G[w]), gen);
        }
        *this = std::move(new_G);
    }

    int contract(int a, int b) {
        if(a == b) { return a; }
        for(auto v : g[b]) {
            addEdge(a, v);
        }
        g[b].clear();
        for(auto & V : g) {
            V.resize(remove(V.begin(), V.end(), b) - V.begin());
        }
        return a;
    }

    void makeSimple() {
        auto const n = size();
        for(auto i = 0U; i < n; i++) {
            // remove loops
            g[i].resize(std::remove(g[i].begin(), g[i].end(), i) - g[i].begin());
            // remove multi-edges
            std::sort(g[i].begin(), g[i].end());
            g[i].resize(std::unique(g[i].begin(), g[i].end()) - g[i].begin());
        }
    }

    void removeIsolated() {
        auto const n = size();
        std::vector<int> translate(n, -1);
        container_t new_G;
        for(auto i = 0U; i < n; i++) {
            if(!g[i].empty()) {
                translate[i] = new_G.size();
                new_G.emplace_back(std::move(g[i]));
            }
        }
        for(auto & V : new_G) {
            for(auto & v : V) {
                v = translate[v];
            }
        }
        std::swap(g, new_G);
    }

    [[nodiscard]] auto begin() {
        return std::begin(g);
    }

    [[nodiscard]] auto end() {
        return std::end(g);
    }

    [[nodiscard]] auto begin() const {
        return std::cbegin(g);
    }

    [[nodiscard]] auto end() const {
        return std::cend(g);
    }

    [[nodiscard]] container_t::size_type size() const {
        return g.size();
    }

    [[nodiscard]] edges_container_t getEdges() const {
        edges_container_t res;
        auto const n = size();
        for(auto a = 0U; a < n; a++) {
            for(auto b : g[a]) {
                if(a <= b) {
                    res.emplace_back(a, b);
                }
            }
        }
        return res;
    }
};

template<typename List>
Graph merge(Graph const & ag, Graph const & bg, List const & new_edges) {
    auto const As = ag.size();
    auto const Bs = bg.size();
    Graph R(As + Bs);
    for(auto [a, b] : ag.getEdges()) {
        R.addEdge(a, b);
    }
    for(auto [a, b] : bg.getEdges()) {
        R.addEdge(As + a, As + b);
    }
    for(auto [a, b] : new_edges) {
        R.addEdge(a, As + b);
    }
    return R;
}

Graph merge(Graph const & ag, Graph const & bg, std::initializer_list<std::pair<int, int>> const & new_edges = {}) {
    return merge<std::initializer_list<std::pair<int, int>>>(ag, bg, new_edges);
}

template<typename List>
Graph identify(Graph const & ag, Graph const & bg, List const & vertices) {
    auto As = ag.size();
    Graph R = merge(ag, bg, {});
    for(auto [a, b] : vertices) {
        R.contract(a, As + b);
    }
    R.removeIsolated();
    R.makeSimple();
    return R;
}

Graph identify(Graph const & ag, Graph const & bg, std::initializer_list<std::pair<int, int>> const & vertices) {
    return identify<std::initializer_list<std::pair<int, int>>>(ag, bg, vertices);
}
class Tree : public Generating<Graph> {
    uint n;
    uint range;

public:
    //NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    explicit constexpr Tree(uint n, uint range) :
      n{n}, range{range} {
        assume(n >= 1U);
        assume(range >= 1U);
    }

    explicit constexpr Tree(uint n) :
      Tree{n, n} {}

    [[nodiscard]] Graph generate(gen_type & gen) const override {
        Graph G(n);
        for(auto i = 1U; i < n; ++i) {
            const auto begin = static_cast<uint>(std::max(0, static_cast<int>(i) - static_cast<int>(range)));
            const auto end = i - 1;
            G.addEdge(i, uni_dist<uint>::gen(begin, end, gen));
        }
        return G;
    }
};

template<typename Derived>
class StaticGraphBase : public Generating<Graph> {
public:
    [[nodiscard]] Graph generate(gen_type & gen) const override {
        Graph G = static_cast<const Derived *>(this)->generate();
        G.permute(gen);
        return G;
    }
};

class Path : public StaticGraphBase<Path> {
    uint n;

public:
    explicit constexpr Path(uint n) :
      n{n} {
        assume(n >= 1U);
    }
    using StaticGraphBase<Path>::generate;
    [[nodiscard]] Graph generate() const {
        Graph G(n);
        for(auto w = 0U; w < n - 1; ++w) {
            G.addEdge(w, w + 1);
        }
        return G;
    }
};

class Clique : public StaticGraphBase<Clique> {
    uint n;

public:
    explicit constexpr Clique(uint n) :
      n{n} {
        assume(n >= 1U);
    }
    using StaticGraphBase<Clique>::generate;
    [[nodiscard]] Graph generate() const {
        Graph G(n);
        for(uint i = 0; i < n; i++) {
            for(uint j = i + 1; j < n; j++) {
                G.addEdge(i, j);
            }
        }
        return G;
    }
};

class Cycle : public StaticGraphBase<Cycle> {
    uint n;

public:
    explicit constexpr Cycle(uint n) :
      n{n} {
        assume(n >= 3U);
    }
    using StaticGraphBase<Cycle>::generate;
    [[nodiscard]] Graph generate() const {
        Graph G(n);
        for(auto i = 0U; i < n - 1; i++) {
            G.addEdge(i, i + 1);
        }
        G.addEdge(n - 1, 0);
        return G;
    }
};

class Star : public StaticGraphBase<Star> {
    uint n;

public:
    explicit constexpr Star(uint n) :
      n{n} {
        assume(n >= 1U);
    }
    using StaticGraphBase<Star>::generate;
    [[nodiscard]] Graph generate() const {
        Graph G(n);
        for(auto i = 1U; i < n; i++) {
            G.addEdge(0, i + 1);
        }
        return G;
    }
};

} /* namespace test */

#endif /* TESTGEN_GRAPH_HPP_ */