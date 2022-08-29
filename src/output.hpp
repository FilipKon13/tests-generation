#ifndef TESTGEN_OUTPUT_HPP_
#define TESTGEN_OUTPUT_HPP_

#include <ostream>

namespace test {

class Space {
    friend std::ostream& operator<<(std::ostream & s, Space const & x) {
        return s << ' ';
    }
} space;

class Output : public std::ostream {
public:
    Output(std::ostream && stream) : std::ostream(std::move(stream)) {}
    Output(std::ostream & stream) {set(&stream);}
    Output() = default;
    Output(const Output&) = delete;
    Output(Output&&) = delete;
    Output& operator=(const Output&) = delete;
    Output& operator=(Output&&) = delete;
    ~Output() = default;

    void set(std::ostream * stream) {
        rdbuf(stream->rdbuf());
    }

    template<typename T>
    Output& operator<<(T const & out) {
        *static_cast<std::ostream*>(this) << out;
        return * this;
    }

private:
    enum DumpState : int8_t {
        FIRST, SPACE, NON_SPACE
    };

public: 
    // no forwarding references as output needs l-value references anyway
    template<typename... Args>
    void dump_output(Args const &... outs) {
        if constexpr (sizeof...(outs) != 0) {
            auto state = FIRST;
            ([&] {
                constexpr auto is_space = std::is_same_v<Args, Space>;
                if (state == NON_SPACE && !is_space)
                    *this << '\n';
                *this << outs;
                state = is_space ? SPACE : NON_SPACE;
            }(), ...);
            if(state == NON_SPACE)
                *this << '\n';
        }
    }
};

} /* namespace test */

#endif /* TESTGEN_OUTPUT_HPP_ */