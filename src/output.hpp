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
    Output(std::ostream & stream) {
        set(&stream);
    }
    Output() = default;
    Output(const Output&) = delete;
    Output(Output&&) = default;
    Output& operator=(const Output&) = delete;
    Output& operator=(Output&&) = default;
    ~Output() = default;
    void set(std::ostream * stream) {
        rdbuf(stream->rdbuf());
    }
    template<typename T>
    friend Output& operator<<(Output & s, T const & out) {
        *static_cast<std::ostream*>(&s) << out;
        return s;
    }

    // TODO : remove recursive template

    // no forwarding references as output needs l-value references anyway
    template<typename Arg1, typename Arg2, typename... Args>
    void dump_output(Arg1 const & first, Arg2 const & second, Args const &... outs) {
        *this << first;
        if constexpr (!std::is_same<Arg1, Space>::value && !std::is_same<Arg2, Space>::value)
            *this << '\n';
        dump_output(second, outs...);
    }

    template<typename Arg> // last thing with newline, unless space
    void dump_output(Arg const & outs) {
        *this << outs;
        if constexpr (!std::is_same<Arg, Space>::value)
            *this << '\n';
    }

    void dump_output() {} // empty
};

} /* namespace test */

#endif /* TESTGEN_OUTPUT_HPP_ */