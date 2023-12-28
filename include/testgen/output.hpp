#ifndef TESTGEN_OUTPUT_HPP_
#define TESTGEN_OUTPUT_HPP_

#include <ostream>

#include "graph.hpp"

namespace test {

class Space {
    friend std::ostream & operator<<(std::ostream & s, [[maybe_unused]] Space const & ignored) {
        return s << ' ';
    }
} const SPACE;

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
    enum DumpState : int8_t { WAS_SPACE,
                              NON_SPACE };

public:
    // no forwarding references as output needs l-value references anyway
    template<typename... Args>
    void dumpOutput(Args const &... outs) {
        if constexpr(sizeof...(outs) != 0) {
            auto state = WAS_SPACE;
            ([&] {
                constexpr auto IS_SPACE = std::is_same_v<Args, Space>;
                if(state == NON_SPACE && !IS_SPACE) {
                    *this << '\n';
                }
                *this << outs;
                state = IS_SPACE ? WAS_SPACE : NON_SPACE;
            }(),
             ...);
            if(state == NON_SPACE) {
                *this << '\n';
            }
        }
    }
};

void printEdges(std::ostream & s, Graph const & g, int shift = 0) {
    for(auto [a, b] : g.getEdges()) {
        s << a + shift << ' ' << b + shift << '\n';
    }
}

void printEdgesAsTree(std::ostream & s, Graph const & g, int shift = 0) {
    std::vector<int> par(g.size());
    auto dfs = [&g, &par](uint w, uint p, auto && self) -> void {
        par[w] = p;
        for(auto v : g[w]) {
            if(v != p) {
                self(v, w, self);
            }
        }
    };
    dfs(0, -1, dfs);
    for(uint i = 1; i < g.size(); i++) {
        s << par[i] + shift << '\n';
    }
}

} /* namespace test */

#endif /* TESTGEN_OUTPUT_HPP_ */