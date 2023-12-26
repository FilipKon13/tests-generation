#ifndef TESTGEN_GRAPH_HPP_
#define TESTGEN_GRAPH_HPP_

#include "rand.hpp"
#include "util.hpp"

#include <algorithm>
#include <numeric>
#include <type_traits>
#include <vector>

namespace test {

namespace detail {
std::vector<int> get_permutation(int n, gen_type & gen) {
    std::vector<int> V(n);
    std::iota(std::begin(V), std::end(V), 0);
    shuffle_sequence(std::begin(V), std::end(V), gen);
    return V;
}
} /* namespace detail */

class Graph {
    using container_t = std::vector<std::vector<int>>;
    using edges_container_t = std::vector<std::pair<int, int>>;
    container_t G;

public:
    explicit Graph(container_t::size_type n) :
      G{n} {}

    [[nodiscard]] std::vector<int> & operator[](int i) {
        return G[i];
    }

    [[nodiscard]] std::vector<int> const & operator[](int i) const {
        return G[i];
    }

    void addEdge(int a, int b) {
        G[a].push_back(b);
        if(a != b) {
            G[b].push_back(a);
        }
    }

    void permute(gen_type & gen) {
        const int n = G.size();
        const auto per = detail::get_permutation(n, gen);
        Graph new_G(G.size());
        for(int w = 0; w < n; ++w) {
            for(auto v : (*this)[w]) {
                new_G[per[w]].push_back(per[v]);
            }
        }
        for(int w = 0; w < n; ++w) {
            shuffle_sequence(std::begin(new_G[w]), std::end(new_G[w]), gen);
        }
        *this = std::move(new_G);
    }

    int contract(int a, int b) {
        if(a == b) { return a; }
        for(auto v : G[b]) {
            addEdge(a, v);
        }
        G[b].clear();
        for(auto & V : G) {
            V.resize(remove(V.begin(), V.end(), b) - V.begin());
        }
        return a;
    }

    void make_simple() {
        int const n = size();
        for(int i = 0; i < n; i++) {
            // remove loops
            G[i].resize(std::remove(G[i].begin(), G[i].end(), i) - G[i].begin());
            // remove multi-edges
            std::sort(G[i].begin(), G[i].end());
            G[i].resize(std::unique(G[i].begin(), G[i].end()) - G[i].begin());
        }
    }

    void remove_isolated() {
        int const n = size();
        std::vector<int> translate(n, -1);
        container_t new_G;
        for(int i = 0; i < n; i++) {
            if(!G[i].empty()) {
                translate[i] = new_G.size();
                new_G.emplace_back(std::move(G[i]));
            }
        }
        for(auto & V : new_G) {
            for(auto & v : V) {
                v = translate[v];
            }
        }
        swap(G, new_G);
    }

    [[nodiscard]] auto begin() {
        return std::begin(G);
    }

    [[nodiscard]] auto end() {
        return std::end(G);
    }

    [[nodiscard]] auto begin() const {
        return std::cbegin(G);
    }

    [[nodiscard]] auto end() const {
        return std::cend(G);
    }

    [[nodiscard]] container_t::size_type size() const {
        return G.size();
    }

    [[nodiscard]] edges_container_t get_edges() const {
        edges_container_t res;
        int const n = size();
        for(int a = 0; a < n; a++) {
            for(auto b : G[a]) {
                if(a <= b) {
                    res.emplace_back(a, b);
                }
            }
        }
        return res;
    }
};

template<typename List>
Graph merge(Graph const & A, Graph const & B, List const & new_edges) {
    int As = A.size();
    int Bs = B.size();
    Graph R(As + Bs);
    for(auto [a, b] : A.get_edges()) {
        R.addEdge(a, b);
    }
    for(auto [a, b] : B.get_edges()) {
        R.addEdge(As + a, As + b);
    }
    for(auto [a, b] : new_edges) {
        R.addEdge(a, As + b);
    }
    return R;
}

Graph merge(Graph const & A, Graph const & B, std::initializer_list<std::pair<int, int>> const & new_edges = {}) {
    return merge<std::initializer_list<std::pair<int, int>>>(A, B, new_edges);
}

template<typename List>
Graph identify(Graph const & A, Graph const & B, List const & vertices) {
    int As = A.size();
    Graph R = merge(A, B, {});
    for(auto [a, b] : vertices) {
        R.contract(a, As + b);
    }
    R.remove_isolated();
    R.make_simple();
    return R;
}

Graph identify(Graph const & A, Graph const & B, std::initializer_list<std::pair<int, int>> const & vertices) {
    return identify<std::initializer_list<std::pair<int, int>>>(A, B, vertices);
}
class Tree : public Generating<Graph> {
    int n;
    int range;

public:
    explicit constexpr Tree(int n, int range) :
      n{n}, range{range} {
        assume(n >= 1);
        assume(range >= 1);
    }

    explicit constexpr Tree(int n) :
      Tree{n, n} {}

    [[nodiscard]] Graph generate(gen_type & gen) const override {
        Graph G(n);
        for(int i = 1; i < n; ++i) {
            const auto begin = std::max(0, i - range);
            const auto end = i - 1;
            G.addEdge(i, UniDist<int>::gen(begin, end, gen));
        }
        return G;
    }
};

class Path : public Generating<Graph> {
    int n;

public:
    explicit constexpr Path(int n) :
      n{n} {
        assume(n >= 1);
    }
    [[nodiscard]] Graph generate() const {
        Graph G(n);
        for(int w = 0; w < n - 1; ++w) {
            G.addEdge(w, w + 1);
        }
        return G;
    }
    [[nodiscard]] Graph generate([[maybe_unused]] gen_type & gen) const override {
        return generate();
    }
};

class Clique : public Generating<Graph> {
    int n;

public:
    explicit constexpr Clique(int n) :
      n{n} {
        assume(n >= 1);
    }
    [[nodiscard]] Graph generate() const {
        Graph G(n);
        for(int i = 0; i < n; i++) {
            for(int j = i + 1; j < n; j++) {
                G.addEdge(i, j);
            }
        }
        return G;
    }
    [[nodiscard]] Graph generate([[maybe_unused]] gen_type & gen) const override {
        return generate();
    }
};

} /* namespace test */

#endif /* TESTGEN_GRAPH_HPP_ */