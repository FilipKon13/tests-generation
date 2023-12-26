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
    edges_container_t edges;

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
        (*this)[a].push_back(b);
        (*this)[b].push_back(a);
        edges.emplace_back(a, b);
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
    [[nodiscard]] edges_container_t const & get_edges() const {
        return edges;
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

Graph merge(Graph const & A, Graph const & B, std::initializer_list<std::pair<int, int>> const & new_edges) {
    return merge<std::initializer_list<std::pair<int, int>>>(A, B, new_edges);
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

    Graph generate(gen_type & gen) const override {
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
    Graph generate([[maybe_unused]] gen_type & gen) const override {
        Graph G(n);
        for(int w = 0; w < n - 1; ++w) {
            G.addEdge(w, w + 1);
        }
        return G;
    }
};

class Clique : public Generating<Graph> {
    int n;

public:
    explicit constexpr Clique(int n) :
      n{n} {
        assume(n >= 1);
    }
    Graph generate([[maybe_unused]] gen_type & gen) const override {
        Graph G(n);
        for(int i = 0; i < n; i++) {
            for(int j = i + 1; j < n; j++) {
                G.addEdge(i, j);
            }
        }
        return G;
    }
};

} /* namespace test */

#endif /* TESTGEN_GRAPH_HPP_ */