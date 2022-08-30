#ifndef TESTGEN_GRAPH_HPP_
#define TESTGEN_GRAPH_HPP_

#include "rand.hpp"

#include <vector>
#include <type_traits>
#include <cassert>
#include <algorithm>

namespace test {

class Graph {
    using container_t = std::vector<std::vector<int>>;
    container_t G;
public:
    Graph(container_t::size_type n) : G{n} {}
    [[nodiscard]] std::vector<int> & operator[](int i) noexcept {return G[i-1];}
    [[nodiscard]] std::vector<int> const & operator[](int i) const noexcept {return G[i-1];}
    void addEdge(int a, int b) {
        (*this)[a].push_back(b);
        (*this)[b].push_back(a);
    }
    void permute(gen_type & gen) { // TODO compare splitting permutation into transpositions vs what is now
        const int n = static_cast<int>(G.size());
        std::vector<int> V(n + 1, -1);
        std::iota(begin(V) + 1, end(V), 1);
        std::shuffle(begin(V) + 1, end(V), gen);
        Graph new_G(G.size());
        for(int w = 1; w <= n; ++w) for(auto v : (*this)[w]) new_G[V[w]].push_back(V[v]);
        *this = std::move(new_G);
    }
    [[nodiscard]] container_t::size_type size() const {return G.size();}
};
static_assert(std::is_move_constructible<Graph>::value);
static_assert(std::is_nothrow_move_constructible<Graph>::value);
static_assert(std::is_move_assignable<Graph>::value);

class Tree : public Generating<Graph> {
    int n;
    int range;
    bool permute;
public:
    constexpr Tree(int n, bool permute, int range) : n{n}, range{range}, permute{permute} {
        assert(n >= 1 && range >= 1);
    }
    constexpr Tree(int n, bool permute = true) : Tree{n, permute, n} {}
    Graph generate(gen_type & gen) const override {
        Graph G{static_cast<unsigned long>(n)};
        for(int i = 2; i <= n; ++i) {
            const auto begin = std::max(1, i - range);
            const auto end = i - 1;
            G.addEdge(i, uniform_distribution<int>{begin, end}(gen));
        }
        if(permute) G.permute(gen);
        return G;
    }
};


} /* namespace test */

#endif /* TESTGEN_GRAPH_HPP_ */