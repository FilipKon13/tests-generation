
/* Source: https://github.com/FilipKon13/tests-generation */

#ifndef TESTGEN_HPP_
#define TESTGEN_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <functional>
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
    if(!value) { exit(EXIT_FAILURE); }
}

/* ==================== rand.hpp ====================*/

constexpr uint64_t TESTGEN_SEED = 0;

class Xoshiro256pp {
    // Suppress magic number linter errors (a lot of that in here and that is normal for a RNG)
public:
    using result_type = uint64_t;

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
        static constexpr unsigned RESULT_TYPE_WIDTH = 64;
        std::array<result_type, 4> t{};
        for(auto jump : JUMP) {
            for(unsigned b = 0; b < RESULT_TYPE_WIDTH; b++) {
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
    explicit Xoshiro256pp(result_type seed) noexcept {
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

    [[nodiscard]] Xoshiro256pp fork() noexcept {
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

using gen_type = Xoshiro256pp;

template<typename T>
class GeneratorWrapper {
    T * gen{};

public:
    explicit GeneratorWrapper(T & gen) noexcept :
      gen(&gen) {}
    GeneratorWrapper & operator=(T & gen) {
        this->gen = &gen;
        return *this;
    }
    ~GeneratorWrapper() noexcept = default;
    GeneratorWrapper(GeneratorWrapper const &) noexcept = default;
    GeneratorWrapper(GeneratorWrapper &&) noexcept = default;
    GeneratorWrapper & operator=(GeneratorWrapper const &) noexcept = default;
    GeneratorWrapper & operator=(GeneratorWrapper &&) noexcept = default;

    operator T &() { //NOLINT allow nonexplicit use as reference to generator inside
        return *gen;
    }

    typename T::result_type operator()() noexcept {
        return (*gen)();
    }

    T & generator() {
        return *gen;
    }
};

template<typename T>
class Generating {
public:
    virtual T generate(gen_type & gen) const = 0;
    virtual ~Generating() noexcept = default;
};

template<typename T>
struct is_generating { //NOLINT(readability-identifier-naming)
private:
    template<typename V>
    static decltype(static_cast<const Generating<V> &>(std::declval<T>()), std::true_type{})
    helper(const Generating<V> &);
    static std::false_type helper(...); /* fallback */
public:
    //NOLINTNEXTLINE readability-identifier-naming and c-vararg
    static constexpr bool value = decltype(helper(std::declval<T>()))::value;
};

template<typename T>
inline constexpr bool is_generating_v = is_generating<T>::value; //NOLINT(readability-identifier-naming)

template<typename T>
struct uni_dist {
private:
    static_assert(std::is_integral_v<T>);
    T begin, end;

public:
    uni_dist(T begin, T end) :
      begin(begin), end(end) {
        assume(begin <= end);
    }
    template<typename Gen>
    T operator()(Gen && gen) const {
        return uni_dist::gen(begin, end, std::forward<Gen>(gen));
    }
    template<typename Gen>
    static T gen(T begin, T end, Gen && gen) {
        if constexpr(std::is_integral_v<T>) { // make this better
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
uni_dist(U, V) -> uni_dist<std::common_type_t<U, V>>;

namespace detail {

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
    for(auto it = begin; ++it != end;) {
        detail::iter_swap(it, begin + uni_dist(0, std::distance(begin, it))(gen));
    }
}

template<typename T>
struct uniform_real_distribution {
};

/* CRTP, assumes Derived has 'generator()' method/field */
template<typename Derived>
class RngUtilities {
public:
    template<typename Iter>
    void shuffle(Iter b, Iter e) {
        shuffle_sequence(b, e, static_cast<Derived *>(this)->generator());
    }
};

template<typename... T>
struct combine : T... { using T::operator()...; };

template<typename... T>
combine(T...) -> combine<T...>;

/* ==================== graph.hpp ====================*/

namespace detail {
std::vector<uint> get_permutation(int n, gen_type & gen) {
    std::vector<uint> V(n);
    std::iota(std::begin(V), std::end(V), 0U);
    shuffle_sequence(std::begin(V), std::end(V), gen);
    return V;
}
} /* namespace detail */

class Graph {
    using container_t = std::vector<std::vector<uint>>;
    using edges_container_t = std::vector<std::pair<uint, uint>>;
    container_t g;

public:
    Graph() :
      Graph(0) {}
    explicit Graph(container_t::size_type n) :
      g{n} {}

    [[nodiscard]] std::vector<uint> & operator[](uint i) {
        return g[i];
    }

    [[nodiscard]] std::vector<uint> const & operator[](uint i) const {
        return g[i];
    }

    void addEdge(uint a, uint b) {
        g[a].push_back(b);
        if(a != b) {
            g[b].push_back(a);
        }
    }

    void permute(gen_type & gen) {
        auto const n = g.size();
        auto const per = detail::get_permutation(n, gen);
        Graph new_G(g.size());
        for(uint w = 0; w < n; ++w) {
            for(auto v : (*this)[w]) {
                new_G[per[w]].push_back(per[v]);
            }
        }
        for(uint w = 0; w < n; ++w) {
            shuffle_sequence(std::begin(new_G[w]), std::end(new_G[w]), gen);
        }
        *this = std::move(new_G);
    }

    int contract(int a, int b) {
        if(a == b) { return a; }
        for(auto v : g[b]) {
            addEdge(a, v);
        }
        g[b].clear();
        for(auto & V : g) {
            V.resize(remove(V.begin(), V.end(), b) - V.begin());
        }
        return a;
    }

    void makeSimple() {
        auto const n = size();
        for(auto i = 0U; i < n; i++) {
            // remove loops
            g[i].resize(std::remove(g[i].begin(), g[i].end(), i) - g[i].begin());
            // remove multi-edges
            std::sort(g[i].begin(), g[i].end());
            g[i].resize(std::unique(g[i].begin(), g[i].end()) - g[i].begin());
        }
    }

    void removeIsolated() {
        auto const n = size();
        std::vector<int> translate(n, -1);
        container_t new_G;
        for(auto i = 0U; i < n; i++) {
            if(!g[i].empty()) {
                translate[i] = new_G.size();
                new_G.emplace_back(std::move(g[i]));
            }
        }
        for(auto & V : new_G) {
            for(auto & v : V) {
                v = translate[v];
            }
        }
        std::swap(g, new_G);
    }

    [[nodiscard]] auto begin() {
        return std::begin(g);
    }

    [[nodiscard]] auto end() {
        return std::end(g);
    }

    [[nodiscard]] auto begin() const {
        return std::cbegin(g);
    }

    [[nodiscard]] auto end() const {
        return std::cend(g);
    }

    [[nodiscard]] container_t::size_type size() const {
        return g.size();
    }

    [[nodiscard]] edges_container_t getEdges() const {
        edges_container_t res;
        auto const n = size();
        for(auto a = 0U; a < n; a++) {
            for(auto b : g[a]) {
                if(a <= b) {
                    res.emplace_back(a, b);
                }
            }
        }
        return res;
    }
};

template<typename List>
Graph merge(Graph const & ag, Graph const & bg, List const & new_edges) {
    auto const As = ag.size();
    auto const Bs = bg.size();
    Graph R(As + Bs);
    for(auto [a, b] : ag.getEdges()) {
        R.addEdge(a, b);
    }
    for(auto [a, b] : bg.getEdges()) {
        R.addEdge(As + a, As + b);
    }
    for(auto [a, b] : new_edges) {
        R.addEdge(a, As + b);
    }
    return R;
}

Graph merge(Graph const & ag, Graph const & bg, std::initializer_list<std::pair<int, int>> const & new_edges = {}) {
    return merge<std::initializer_list<std::pair<int, int>>>(ag, bg, new_edges);
}

template<typename List>
Graph identify(Graph const & ag, Graph const & bg, List const & vertices) {
    auto As = ag.size();
    Graph R = merge(ag, bg, {});
    for(auto [a, b] : vertices) {
        R.contract(a, As + b);
    }
    R.removeIsolated();
    R.makeSimple();
    return R;
}

Graph identify(Graph const & ag, Graph const & bg, std::initializer_list<std::pair<int, int>> const & vertices) {
    return identify<std::initializer_list<std::pair<int, int>>>(ag, bg, vertices);
}
class Tree : public Generating<Graph> {
    uint n;
    uint range;

public:
    //NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    explicit constexpr Tree(uint n, uint range) :
      n{n}, range{range} {
        assume(n >= 1U);
        assume(range >= 1U);
    }

    explicit constexpr Tree(uint n) :
      Tree{n, n} {}

    [[nodiscard]] Graph generate(gen_type & gen) const override {
        Graph G(n);
        for(auto i = 1U; i < n; ++i) {
            const auto begin = static_cast<uint>(std::max(0, static_cast<int>(i) - static_cast<int>(range)));
            const auto end = i - 1;
            G.addEdge(i, uni_dist<uint>::gen(begin, end, gen));
        }
        return G;
    }
};

class Path : public Generating<Graph> {
    uint n;

public:
    explicit constexpr Path(uint n) :
      n{n} {
        assume(n >= 1U);
    }
    [[nodiscard]] Graph generate() const {
        Graph G(n);
        for(auto w = 0U; w < n - 1; ++w) {
            G.addEdge(w, w + 1);
        }
        return G;
    }
    [[nodiscard]] Graph generate(gen_type & gen) const override {
        Graph G = generate();
        G.permute(gen);
        return G;
    }
};

class Clique : public Generating<Graph> {
    uint n;

public:
    explicit constexpr Clique(uint n) :
      n{n} {
        assume(n >= 1U);
    }
    [[nodiscard]] Graph generate() const {
        Graph G(n);
        for(uint i = 0; i < n; i++) {
            for(uint j = i + 1; j < n; j++) {
                G.addEdge(i, j);
            }
        }
        return G;
    }
    [[nodiscard]] Graph generate(gen_type & gen) const override {
        Graph G = generate();
        G.permute(gen);
        return G;
    }
};

class Cycle : Generating<Graph> {
    uint n;

public:
    explicit constexpr Cycle(uint n) :
      n{n} {
        assume(n >= 3U);
    }
    [[nodiscard]] Graph generate() const {
        Graph G(n);
        for(auto i = 0U; i < n - 1; i++) {
            G.addEdge(i, i + 1);
        }
        G.addEdge(n - 1, 0);
        return G;
    }
    [[nodiscard]] Graph generate(gen_type & gen) const override {
        Graph G = generate();
        G.permute(gen);
        return G;
    }
};

/* ==================== output.hpp ====================*/

class Space {
    friend std::ostream & operator<<(std::ostream & s, Space const & /* unused */) {
        return s << ' ';
    }
};
[[maybe_unused]] constexpr static Space SPACE{};

class Output : public std::ostream {
public:
    explicit Output(std::ostream && stream) :
      std::ostream(std::move(stream)) {}
    explicit Output(std::ostream & stream) {
        set(stream);
    }
    Output() = default;
    Output(Output const &) = delete;
    Output(Output &&) = delete;
    Output & operator=(Output const &) = delete;
    Output & operator=(Output &&) = delete;
    ~Output() override = default;

    void set(std::ostream & stream) {
        rdbuf(stream.rdbuf());
    }

private:
    enum DumpState : int8_t { WAS_SPACE,
                              NON_SPACE };

public:
    template<typename... Args>
    void dumpOutput(Args &&... outs) {
        if constexpr(sizeof...(outs) != 0) {
            auto state = WAS_SPACE;
            ([&] {
                constexpr auto IS_SPACE = std::is_same_v<std::remove_cv_t<std::remove_reference_t<Args>>, Space>;
                if(state == NON_SPACE && !IS_SPACE) {
                    *this << '\n';
                }
                *this << std::forward<Args>(outs);
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

/* ==================== manager.hpp ====================*/

enum TestType : uint8_t { OCEN = UINT8_MAX };

namespace detail {
struct index {
    unsigned test;
    std::variant<TestType, unsigned> suite;

    [[nodiscard]] constexpr bool operator==(index const & x) const {
        return test == x.test && suite == x.suite;
    }

    struct hash {
        [[nodiscard]] constexpr std::size_t operator()(index const & indx) const {
            constexpr auto SHIFT = 10U;
            return static_cast<size_t>(
                (indx.test << SHIFT) ^ std::visit([](auto x) { return x + 1U; }, indx.suite));
        }
    };
};
} /* namespace detail */

enum Verbocity {
    SILENT = 0,
    VERBOSE = 1
};

template<Verbocity Verbose = VERBOSE, typename StreamType = std::ofstream>
class OIOIOIManager {
    bool changeIfTaken() {
        if(auto const it = cases.find(curr_index); it != cases.end()) {
            curr_test = &it->second;
            return true;
        }
        return false;
    }

    void changeToNewStream(std::string const & name) {
        if constexpr(Verbose == Verbocity::VERBOSE) {
            std::cout << "Printing to: " << name << '\n';
        }
        auto const it = cases.try_emplace(curr_index,
                                          std::piecewise_construct,
                                          std::forward_as_tuple(name),
                                          std::forward_as_tuple(source_generator.fork()));
        curr_test = &it.first->second;
    }

    struct test_info : private std::pair<StreamType, gen_type> {
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
    index curr_index;
    std::string abbr;
    std::unordered_map<index, test_info, index::hash> cases{};
    gen_type source_generator{TESTGEN_SEED};

public:
    // CUSTOM: change default first suite to 0U if used for example tests
    explicit OIOIOIManager(std::string abbr, bool ocen = false) :
      curr_index{0, ocen ? std::variant<TestType, unsigned>(OCEN) : std::variant<TestType, unsigned>(/* CUSTOM */ 1U)}, abbr(std::move(abbr)) {}

    OIOIOIManager() = delete;
    OIOIOIManager(OIOIOIManager const &) = delete;
    OIOIOIManager(OIOIOIManager &&) noexcept = default;
    ~OIOIOIManager() = default;
    OIOIOIManager & operator=(OIOIOIManager const &) = delete;
    OIOIOIManager & operator=(OIOIOIManager &&) noexcept = default;

    void setMainSeed(gen_type::result_type seed) {
        source_generator = gen_type(seed);
    }

    [[nodiscard]] constexpr StreamType & stream() const noexcept {
        return curr_test->stream();
    }

    [[nodiscard]] constexpr gen_type & generator() const noexcept {
        return curr_test->generator();
    }

    void nextTest() {
        std::visit([this](auto x) { this->test(this->curr_index.test + 1, x); },
                   curr_index.suite);
    }

    void nextSuite() {
        std::visit(combine{
                       [this]([[maybe_unused]] TestType) {
                           this->test(1, 1);
                       },
                       [this](unsigned x) {
                           this->test(1, x + 1);
                       }},
                   curr_index.suite);
    }

    void test(unsigned test, unsigned suite) {
        curr_index = {test, suite};
        if(changeIfTaken()) { return; }
        changeToNewStream(getFilename());
    }

    void test(unsigned test, TestType ocen) {
        curr_index = {test, ocen};
        if(changeIfTaken()) { return; }
        changeToNewStream(getFilename());
    }

    [[nodiscard]] std::string getFilename() const {
        if(unsigned const * suite = std::get_if<unsigned>(&curr_index.suite)) {
            std::string suffix{};
            auto nr = curr_index.test - 1;
            constexpr unsigned SIZE = 'z' - 'a' + 1;
            do {
                suffix += 'a' + (nr % SIZE);
                nr /= SIZE;
            } while(nr != 0);
            std::reverse(std::begin(suffix), std::end(suffix));
            return abbr + std::to_string(*suite) + suffix + ".in";
        }
        return abbr + std::to_string(curr_index.test) + "ocen.in";
    }
};

/* ==================== assumptions.hpp ====================*/

template<typename TestcaseT>
class AssumptionManager {
public:
    // using assumption_t = bool (*)(TestcaseT const &);
    using assumption_t = std::function<bool(TestcaseT const &)>;

private:
    static bool empty(TestcaseT const & /*unused*/) {
        return true;
    }
    assumption_t global = empty;
    assumption_t suite = empty;
    assumption_t test = empty;

public:
    template<typename AssT>
    void setGlobal(AssT && fun) {
        global = std::forward<AssT>(fun);
    }
    template<typename AssT>
    void setSuite(AssT && fun) {
        suite = std::forward<AssT>(fun);
    }
    template<typename AssT>
    void setTest(AssT && fun) {
        test = std::forward<AssT>(fun);
    }
    void resetGlobal() {
        global = empty;
    }
    void resetSuite() {
        suite = empty;
    }
    void resetTest() {
        test = empty;
    }
    bool check(TestcaseT const & testcase) {
        return test(testcase) && suite(testcase) && global(testcase);
    }
};

template<typename Fun1T, typename Fun2T>
auto operator&&(Fun1T && fun1, Fun2T && fun2) {
    return [f1 = std::forward<Fun1T>(fun1), f2 = std::forward<Fun2T>(fun2)](auto const & testcase) mutable {
        return f1(testcase) && f2(testcase);
    };
}

template<typename Fun1T, typename Fun2T>
auto operator||(Fun1T && fun1, Fun2T && fun2) {
    return [f1 = std::forward<Fun1T>(fun1), f2 = std::forward<Fun2T>(fun2)](auto const & testcase) mutable {
        return f1(testcase) || f2(testcase);
    };
}

/* ==================== testing.hpp ====================*/

class DummyTestcase {};

template<typename TestcaseManagerT, typename TestcaseT = DummyTestcase, template<typename> typename AssumptionsManagerT = AssumptionManager>
class Testing : private TestcaseManagerT, public RngUtilities<Testing<TestcaseManagerT, TestcaseT, AssumptionsManagerT>> {
    template<typename T>
    auto generate(Generating<T> const & schema) {
        return schema.generate(generator());
    }

    TestcaseT updateTestcase() {
        output.set(this->stream());
        return TestcaseT{};
    }

    Output output;
    AssumptionsManagerT<TestcaseT> assumptions;

public:
    using TestcaseManagerT::TestcaseManagerT;

    Testing(const Testing &) = delete;
    Testing(Testing &&) = delete;
    Testing & operator=(const Testing &) = delete;
    Testing & operator=(Testing &&) = delete;
    ~Testing() = default;

    GeneratorWrapper<gen_type> generator() {
        return GeneratorWrapper<gen_type>{TestcaseManagerT::generator()};
    }

    TestcaseT nextSuite() {
        TestcaseManagerT::nextSuite();
        assumptions.resetSuite();
        assumptions.resetTest();
        return updateTestcase();
    }

    TestcaseT nextTest() {
        TestcaseManagerT::nextTest();
        assumptions.resetTest();
        return updateTestcase();
    }

    template<typename... T>
    TestcaseT test(T &&... args) {
        TestcaseManagerT::test(std::forward<T>(args)...);
        return updateTestcase();
    }

    template<typename... T>
    void print(T const &... args) {
        output.dumpOutput((*this)(args)...);
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
    Testing & operator<<(T const & out) {
        if constexpr(std::is_same_v<T, TestcaseT>) {
            if(!checkSoft(out)) {
                std::cerr << "Assumption failed for " << this->TestcaseManagerT::getFilename() << '\n';
                assume(false);
            }
        }
        output << (*this)(out);
        return *this;
    }

    template<typename AssT>
    void assumptionGlobal(AssT && fun) {
        assumptions.setGlobal(std::forward<AssT>(fun));
    }

    template<typename AssT>
    void assumptionSuite(AssT && fun) {
        assumptions.setSuite(std::forward<AssT>(fun));
    }

    template<typename AssT>
    void assumptionTest(AssT && fun) {
        assumptions.setTest(std::forward<AssT>(fun));
    }

    bool checkSoft(TestcaseT const & tc) {
        return assumptions.check(tc);
    }

    bool checkHard(TestcaseT const & tc) {
        assume(checkHard(tc));
        return true;
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
    std::size_t n;
    Dist dist;

public:
    constexpr DistSequence(std::size_t n, T begin, T end) :
      n(n), dist(begin, end) {}
    Sequence<T> generate(gen_type & gen) const override {
        return Sequence<T>(n, [&] { return dist(gen); });
    }
};

template<typename T>
using UniSequence = DistSequence<uni_dist<T>, T>;

template<typename T, std::size_t S>
class FiniteSequence : Generating<Sequence<T>> {
    std::size_t n;
    std::array<T, S> elems;

    template<std::size_t... Indx>
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
    static std::array<T, S> construct(T const (&arr)[S], std::index_sequence<Indx...> /* unused */) {
        return std::array<T, S>({arr[Indx]...});
    }

public:
    constexpr FiniteSequence(std::size_t n, std::array<T, S> const & arr) :
      n(n), elems(arr) {}

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
    constexpr FiniteSequence(std::size_t n, T const (&arr)[S]) :
      n(n), elems(construct(arr, std::make_index_sequence<S>{})) {}
    Sequence<T> generate(gen_type & gen) const override {
        return Sequence<T>(n, [&, dist = uni_dist<std::size_t>(0, S - 1)] { return elems.at(dist(gen)); });
    }
};

} /* namespace test */

#endif /* TESTGEN_HPP_ */
