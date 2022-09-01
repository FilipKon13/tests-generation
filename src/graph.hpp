#ifndef TESTGEN_GRAPH_HPP_
#define TESTGEN_GRAPH_HPP_

#include "rand.hpp"

#include <algorithm>
#include <type_traits>
#include <vector>

namespace test {

namespace detail {
    std::vector<int> get_permutation(int n, gen_type & gen) {
        std::vector<int> V(n + 1, -1);
        std::iota(std::begin(V) + 1, std::end(V), 1);
        std::shuffle(std::begin(V) + 1, std::end(V), gen);
        return V;
    }
} /* namespace detail */

class Graph {
    using container_t = std::vector<std::vector<int>>;
    container_t G;
public:
    explicit Graph(container_t::size_type n) : G{n} {}
    [[nodiscard]] std::vector<int> & operator[](int i) {return G[i-1];}
    [[nodiscard]] std::vector<int> const & operator[](int i) const {return G[i-1];}
    void addEdge(int a, int b) {
        (*this)[a].push_back(b);
        (*this)[b].push_back(a);
    }
    void permute(gen_type & gen) {
        const int n = static_cast<int>(G.size());
        const auto per = detail::get_permutation(n, gen);
        Graph new_G(G.size());
        for(int w = 1; w <= n; ++w) {
            for(auto v : (*this)[w]) {
                new_G[per[w]].push_back(per[v]);
            }
        }
        for(int w = 1; w <= n; ++w) {
            std::shuffle(std::begin(new_G[w]), std::end(new_G[w]), gen);
        }
        *this = std::move(new_G);
    }
    [[nodiscard]] auto begin() {return std::begin(G);}
    [[nodiscard]] auto end() {return std::end(G);}
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
    explicit constexpr Tree(int n, bool permute, int range) : n{n}, range{range}, permute{permute} {
        if(n < 1 || range < 1) {
            throw std::runtime_error("wrong parameters");
        }
    }
    explicit constexpr Tree(int n, bool permute = true) : Tree{n, permute, n} {}
    Graph generate(gen_type & gen) const override {
        Graph G{static_cast<size_t>(n)};
        for(int i = 2; i <= n; ++i) {
            const auto begin = std::max(1, i - range);
            const auto end = i - 1;
            G.addEdge(i, uniform_distribution<int>{begin, end}(gen));
        }
        if(permute) {
            G.permute(gen);
        }
        return G;
    }
};

class Path : public Generating<Graph> {
    int n;
    bool permute;
public:
    explicit constexpr Path(int n, bool permute = true) : n{n}, permute{permute} {
        if(n < 1) {
            throw std::runtime_error("wrong parameters");
        }
    }
    Graph generate(gen_type & gen) const override {
        Graph G{static_cast<size_t>(n)};
        if(permute) {
            const auto per = detail::get_permutation(n, gen);
            for(int w = 1; w < n; ++w) {
                G.addEdge(per[w], per[w + 1]);
            }
        } else {
            for(int w = 1; w < n; ++w) {
                G.addEdge(w, w + 1);
            }
        }
        return G;
    }
};

class Clique : public Generating<Graph> {
    int n;
    bool permute;
public:
    explicit constexpr Clique(int n, bool permute = true) : n{n}, permute{permute} {
        if(n < 1) {
            throw std::runtime_error("wrong parameters");
        }
    }
    Graph generate(gen_type & gen) const override {
        Graph G{static_cast<size_t>(n)};
        for(int i=1;i<=n;i++) {
            for(int j=i+1;j<=n;j++) {
                G.addEdge(i, j);
            }
        }
        if(permute) {
            for(int i=1;i<=n;i++) {
                std::shuffle(std::begin(G[i]), std::end(G[i]), gen);
            }
        }
        return G;
    }
};

} /* namespace test */

#endif /* TESTGEN_GRAPH_HPP_ */