#ifndef TESTGEN_HPP_
#define TESTGEN_HPP_

#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <numeric>
#include <ostream>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

namespace test {
/* ==================== util.hpp ====================*/

inline void assume(bool value) {
    if(!value) {
        std::cerr << "Assumption failed!\n";
        exit(EXIT_FAILURE);
    }
}

/* ==================== rand.hpp ====================*/

constexpr uint64_t TESTGEN_SEED = 0;

class xoshiro256pp {
    // Suppress magic number errors (a lot of that in here and that is normal for a RNG)
public:
    typedef uint64_t result_type;

private:
    static inline result_type rotl(result_type x, unsigned k) {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
        return (x << k) | (x >> (64U - k));
    }

    std::array<result_type, 4> s{};

    void advance() {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
        auto const t = s[1] << 17U;
        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];
        s[2] ^= t;
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
        s[3] = rotl(s[3], 45U);
    }

    void jump() {
        static constexpr std::array JUMP = {0x180ec6d33cfd0abaUL, 0xd5a61266f0c9392cUL, 0xa9582618e03fc9aaUL, 0x39abdc4529b1661cUL};
        std::array<result_type, 4> t{};
        for(auto jump : JUMP) {
            for(unsigned b = 0; b < UINT64_WIDTH; b++) {
                if((jump & UINT64_C(1) << b) != 0) {
                    t[0] ^= s[0];
                    t[1] ^= s[1];
                    t[2] ^= s[2];
                    t[3] ^= s[3];
                }
                advance();
            }
        }
        std::copy(t.begin(), t.end(), s.begin());
    }

public:
    explicit xoshiro256pp(result_type seed) noexcept {
        auto next_seed = [x = seed]() mutable {
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
            auto z = (x += 0x9e3779b97f4a7c15UL);
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
            z = (z ^ (z >> 30U)) * 0xbf58476d1ce4e5b9UL;
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
            z = (z ^ (z >> 27U)) * 0x94d049bb133111ebUL;
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
            return z ^ (z >> 31U);
        };
        for(auto & v : s) {
            v = next_seed();
        }
    }

    [[nodiscard]] result_type operator()() noexcept {
        auto const result = rotl(s[0] + s[3], 23) + s[0];
        advance();
        return result;
    }

    [[nodiscard]] xoshiro256pp fork() noexcept {
        auto const result = *this;
        jump();
        return result;
    }

    static constexpr result_type max() {
        return UINT64_MAX;
    }

    static constexpr result_type min() {
        return 0;
    }
};

using gen_type = xoshiro256pp;

template<typename T>
struct Generating {
    virtual T generate(gen_type & gen) const = 0;
};

template<typename T>
struct is_generating {
private:
    template<typename V>
    static decltype(static_cast<const Generating<V> &>(std::declval<T>()), std::true_type{})
    helper(const Generating<V> &);

    static std::false_type helper(...); /* fallback */
public:
    static constexpr bool value = decltype(helper(std::declval<T>()))::value;
};

template<typename T>
class UniDist {
    static_assert(std::is_integral_v<T>);

    T _begin, _end;

public:
    UniDist(T begin, T end) :
      _begin(begin), _end(end) {
        assume(begin <= end);
    }
    template<typename Gen>
    T operator()(Gen && gen) const {
        return UniDist::gen(_begin, _end, std::forward<Gen>(gen));
    }
    template<typename Gen>
    static T gen(T begin, T end, Gen && gen) {
        if constexpr(std::is_integral_v<T>) {    // make this better
            auto const range = end - begin + 1;
            auto res = gen() % range;
            return res + begin;
        } else {
            using gen_t = std::remove_reference_t<Gen>;
            auto const urange = gen_t::max() - gen_t::min();
            auto const range = end - begin;
            return (static_cast<T>(gen()) / urange) * range + begin;
        }
    }
};
template<typename U, typename V>
UniDist(U, V) -> UniDist<std::common_type_t<U, V>>;

namespace detail {
template<typename T, typename Gen>
std::pair<T, T> generate_two(T x, T y, Gen && gen) {
    T v = UniDist(0, x * y - 1)(gen);
    return {v / y, v % y};
}

template<typename Iter>
void iter_swap(Iter a, Iter b) {
    std::swap(*a, *b);
}
} /* namespace detail */

template<typename Iter, typename Gen>
void shuffle_sequence(Iter begin, Iter end, Gen && gen) {
    if(begin == end) {
        return;
    }
    using gen_t = std::remove_reference_t<Gen>;
    auto const len = static_cast<typename gen_t::result_type>(std::distance(begin, end));
    auto const range = gen_t::max() - gen_t::min();
    if(range / len >= len) {    // faster variant
        auto it = begin + 1;
        if(len % 2 == 0) {
            detail::iter_swap(it++, begin + UniDist(0, 1)(gen));
        }
        while(it != end) {
            auto cnt = it - begin;
            auto p = detail::generate_two(cnt, cnt + 1, gen);
            detail::iter_swap(it++, begin + p.first);
            detail::iter_swap(it++, begin + p.second);
        }
    } else {    // for really big ranges
        for(auto it = begin; ++it != end;) {
            detail::iter_swap(it, begin + UniDist(0, std::distance(begin, it))(gen));
        }
    }
}

template<typename T>
class uniform_real_distribution {
};

template<typename... T>
struct combine : T... { using T::operator()...; };

template<typename... T>
combine(T...) -> combine<T...>;

/* ==================== output.hpp ====================*/

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

/* ==================== testing.hpp ====================*/

template<typename TestcaseManager>
class Testing : private Output, private TestcaseManager {
    template<typename T>
    auto generate(Generating<T> const & schema) {
        return schema.generate(this->generator());
    }

public:
    using TestcaseManager::TestcaseManager;

    Testing(const Testing &) = delete;
    Testing(Testing &&) = delete;
    Testing & operator=(const Testing &) = delete;
    Testing & operator=(Testing &&) = delete;
    ~Testing() override = default;

    void nextSuite() {
        this->TestcaseManager::nextSuite();
        set(this->stream());
    }

    void nextTest() {
        this->TestcaseManager::nextTest();
        set(this->stream());
    }

    template<typename... T>
    void test(T &&... args) {
        this->TestcaseManager::test(std::forward<T>(args)...);
        set(this->stream());
    }

    template<typename... T>
    void print(T const &... args) {
        dump_output((*this)(args)...);
    }

    template<typename T>
    decltype(auto) operator()(T const & t) { /* decltype(auto) does not decay static arrays to pointers */
        if constexpr(is_generating<T>::value) {
            return generate(t);
        } else {
            return t;
        }
    }

    template<typename T>
    Testing & operator<<(const T & out) {
        static_cast<std::ostream &>(*this) << (*this)(out);
        return *this;
    }
};

/* ==================== manager.hpp ====================*/

enum TestType : int8_t { OCEN };

namespace detail {
struct index {
    unsigned test;
    std::variant<TestType, unsigned> suite;

    [[nodiscard]] constexpr bool operator==(index const & x) const {
        return test == x.test && suite == x.suite;
    }

    class hash {
        constexpr inline static unsigned SHIFT = 10U;

    public:
        [[nodiscard]] constexpr std::size_t operator()(index const & indx) const {
            return static_cast<size_t>(
                (indx.test << SHIFT)
                ^ std::visit(
                    combine{[]([[maybe_unused]] TestType x) {
                                return 0U;
                            },
                            [](unsigned x) {
                                return x + 1;
                            }},
                    indx.suite));
        }
    };
};
} /* namespace detail */

template<typename StreamType = std::ofstream>
class OIOIOIManager {
    bool change_if_taken() {
        if(auto const it = cases.find(curr_index); it != cases.end()) {
            curr_test = &it->second;
            return true;
        }
        return false;
    }

    void change_to_new_stream(std::string const & name) {
        auto const it = cases.try_emplace(curr_index,
                                          std::piecewise_construct,
                                          std::forward_as_tuple(name),
                                          std::forward_as_tuple(source_generator.fork()));
        curr_test = &it.first->second;
    }

    class test_info : private std::pair<StreamType, gen_type> {
    public:
        using std::pair<StreamType, gen_type>::pair;
        [[nodiscard]] constexpr StreamType & stream() noexcept {
            return this->first;
        }
        [[nodiscard]] constexpr gen_type & generator() noexcept {
            return this->second;
        }
    };
    using index = detail::index;
    test_info * curr_test{nullptr};
    index curr_index{};
    std::string abbr;
    std::unordered_map<index, test_info, index::hash> cases{};
    gen_type source_generator{TESTGEN_SEED};

public:
    explicit OIOIOIManager(std::string abbr, bool ocen = false) :
      abbr(std::move(abbr)) {
        if(ocen) {
            test(1, OCEN);    // pro1ocen.in
        } else {
            test(1, 1);    // pro1a.in
        }
    }

    OIOIOIManager() = delete;
    OIOIOIManager(OIOIOIManager const &) = delete;
    OIOIOIManager(OIOIOIManager &&) noexcept = default;
    ~OIOIOIManager() = default;
    OIOIOIManager & operator=(OIOIOIManager const &) = delete;
    OIOIOIManager & operator=(OIOIOIManager &&) noexcept = default;

    [[nodiscard]] constexpr StreamType & stream() const noexcept {
        return curr_test->stream();
    }

    [[nodiscard]] constexpr gen_type & generator() const noexcept {
        return curr_test->generator();
    }

    void nextTest() {
        std::visit(combine{
                       [this](TestType x) { this->test(this->curr_index.test + 1, x); },
                       [this](unsigned x) {
                           this->test(this->curr_index.test + 1, x);
                       }},
                   curr_index.suite);
    }

    void nextSuite() {
        std::visit(combine{
                       [this]([[maybe_unused]] TestType x) { this->test(1, 1); },
                       [this](unsigned x) {
                           this->test(1, x + 1);
                       }},
                   curr_index.suite);
    }

    void test(unsigned test, unsigned suite) {
        curr_index = {test, suite};
        if(change_if_taken()) { return; }
        std::string suffix{};
        auto nr = test - 1;
        constexpr unsigned size = 'z' - 'a' + 1;
        do {
            suffix += 'a' + (nr % size);
            nr /= size;
        } while(nr != 0);
        std::reverse(std::begin(suffix), std::end(suffix));
        change_to_new_stream(abbr + std::to_string(suite) + suffix + ".in");
    }

    void test(unsigned test, TestType ocen) {
        curr_index = {test, ocen};
        if(change_if_taken()) { return; }
        change_to_new_stream(abbr + std::to_string(test) + "ocen.in");
    }
};

/* ==================== sequence.hpp ====================*/

template<typename T>
class Sequence : public std::vector<T> {
    template<typename Gen, std::enable_if_t<std::is_invocable_v<Gen>, int> = 0>
    static void seqGenerate(Sequence & s, Gen && gen) {
        std::generate(s.begin(), s.end(), gen);
    }

    template<typename Gen, std::enable_if_t<std::is_invocable_v<Gen, unsigned>, int> = 0>
    static void seqGenerate(Sequence & s, Gen && gen) {
        std::generate(s.begin(), s.end(), [&gen, cnt = 0U]() mutable { return std::invoke(gen, cnt++); });
    }

public:
    using std::vector<T>::vector;

    template<typename Gen, typename = std::enable_if_t<std::is_invocable_v<Gen> || std::is_invocable_v<Gen, unsigned>>>
    Sequence(std::size_t size, Gen && gen) :
      std::vector<T>(size) {
        seqGenerate(*this, std::forward<Gen>(gen));
    }

    Sequence operator+(Sequence const & x) const {
        Sequence res(this->size() + x.size());
        auto const it = std::copy(this->begin(), this->end(), res.begin());
        std::copy(x.begin(), x.end(), it);
        return res;
    }

    Sequence & operator+=(Sequence const & x) {
        auto const old_size = this->size();
        this->resize(old_size + x.size());
        std::copy(x.begin(), x.end(), this->begin() + old_size);
        return *this;
    }

    friend std::ostream & operator<<(std::ostream & s, Sequence const & x) {
        auto it = x.begin();
        auto const end = x.end();
        if(it == end) { return s; }
        s << *it++;
        while(it != end) {
            s << ' ' << *it++;
        }
        return s;
    }
};

template<typename Dist, typename T>
class DistSequence : Generating<Sequence<T>> {
    std::size_t _N;
    Dist _dist;

public:
    constexpr DistSequence(std::size_t N, T begin, T end) noexcept :
      _N(N), _dist(begin, end) {}
    Sequence<T> generate(gen_type & gen) const override {
        return Sequence<T>(_N, [&] { return _dist(gen); });
    }
};

template<typename T>
using UniSequence = DistSequence<UniDist<T>, T>;

template<typename T, std::size_t S>
class FiniteSequence : Generating<Sequence<T>> {
    std::size_t _N;
    std::array<T, S> _elems;

    template<std::size_t... Indx>
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
    static std::array<T, S> construct(T const (&arr)[S], std::index_sequence<Indx...> /* unused */) {
        return std::array<T, S>({arr[Indx]...});
    }

public:
    constexpr FiniteSequence(std::size_t N, std::array<T, S> const & arr) :
      _N(N), _elems(arr) {}

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
    constexpr FiniteSequence(std::size_t N, T const (&arr)[S]) :
      _N(N), _elems(construct(arr, std::make_index_sequence<S>{})) {}
    Sequence<T> generate(gen_type & gen) const override {
        return Sequence<T>(_N, [&, dist = UniDist<std::size_t>(0, S - 1)] { return _elems[dist(gen)]; });
    }
};

/* ==================== graph.hpp ====================*/

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

#endif /* TESTGEN_HPP_ */
