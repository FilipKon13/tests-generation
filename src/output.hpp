#ifndef TESTGEN_OUTPUT_HPP_
#define TESTGEN_OUTPUT_HPP_

#include <ostream>

namespace test {

class Space {
    friend std::ostream& operator<<(std::ostream & s, [[maybe_unused]] Space const & ignored) {
        return s << ' ';
    }
} space;

class Output : public std::ostream {
public:
    explicit Output(std::ostream && stream) : std::ostream(std::move(stream)) {}
    explicit Output(std::ostream const & stream) {set(stream);}
    Output() = default;
    Output(Output const &) = delete;
    Output(Output&&) = delete;
    Output& operator=(Output const &) = delete;
    Output& operator=(Output&&) = delete;
    ~Output() override = default;

    void set(std::ostream const & stream) {rdbuf(stream.rdbuf());}

private:
    enum DumpState : int8_t {SPACE, NON_SPACE};

public: 
    // no forwarding references as output needs l-value references anyway
    template<typename... Args>
    void dump_output(Args const &... outs) {
        if constexpr (sizeof...(outs) != 0) {
            auto state = SPACE;
            ([&] {
                constexpr auto is_space = std::is_same_v<Args, Space>;
                if (state == NON_SPACE && !is_space) {
                    *this << '\n';
                }
                *this << outs;
                state = is_space ? SPACE : NON_SPACE;
            }(), ...);
            if(state == NON_SPACE) {
                *this << '\n';
            }
        }
    }
};

} /* namespace test */

#endif /* TESTGEN_OUTPUT_HPP_ */