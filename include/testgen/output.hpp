#ifndef TESTGEN_OUTPUT_HPP_
#define TESTGEN_OUTPUT_HPP_

#include <ostream>

#include "graph.hpp"

namespace test {

class Space {
    friend std::ostream & operator<<(std::ostream & s, [[maybe_unused]] Space const & ignored) {
        return s << ' ';
    }
} space;

class Output : public std::ostream {
public:
    explicit Output(std::ostream && stream) :
      std::ostream(std::move(stream)) {}
    explicit Output(std::ostream const & stream) {
        set(stream);
    }
    Output() = default;
    Output(Output const &) = delete;
    Output(Output &&) = delete;
    Output & operator=(Output const &) = delete;
    Output & operator=(Output &&) = delete;
    ~Output() override = default;

    void set(std::ostream const & stream) {
        rdbuf(stream.rdbuf());
    }

private:
    enum DumpState : int8_t { SPACE,
                              NON_SPACE };

public:
    // no forwarding references as output needs l-value references anyway
    template<typename... Args>
    void dump_output(Args const &... outs) {
        if constexpr(sizeof...(outs) != 0) {
            auto state = SPACE;
            ([&] {
                constexpr auto is_space = std::is_same_v<Args, Space>;
                if(state == NON_SPACE && !is_space) {
                    *this << '\n';
                }
                *this << outs;
                state = is_space ? SPACE : NON_SPACE;
            }(),
             ...);
            if(state == NON_SPACE) {
                *this << '\n';
            }
        }
    }
};


void printEdges(std::ostream & s, Graph const & G, int shift = 0) {
    for(auto [a,b] : G.get_edges()) {
        s << a + shift << ' ' << b + shift << '\n';
    }
}

void printEdgesAsTree(std::ostream & s, Graph const & G, int shift = 0) {
    std::vector<int> par(G.size());
    auto dfs = [&G, &par](int w, int p, auto && self) -> void {
        par[w] = p;
        for(auto v : G[w]) {
            if(v != p) {
                self(v,w,self);
            }
        }
    };
    dfs(0,-1,dfs);
    for(uint i=1;i<G.size();i++) {
        s << par[i] + shift << '\n';
    }
}

} /* namespace test */

#endif /* TESTGEN_OUTPUT_HPP_ */