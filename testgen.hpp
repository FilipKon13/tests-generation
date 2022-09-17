#include <type_traits>
#include <algorithm>
#include <memory>
#include <cassert>
#include <vector>
#include <random>
#include <unordered_map>
#include <variant>
#include <ostream>
#include <fstream>
#include <string_view>

namespace test {

using gen_type = std::mt19937;

template<typename T>
struct Generating {
    virtual T generate(gen_type & gen) const = 0;
};

template<typename T>
struct is_generating
{
private:
    template<typename V>
    static decltype(static_cast<const Generating<V>&>(std::declval<T>()), std::true_type{})
    helper(const Generating<V>&);
    
    static std::false_type helper(...); /* fallback */
public:
    static constexpr bool value = decltype(helper(std::declval<T>()))::value;
};

template<typename T>
class UniDist {
    T _begin, _end;
public:
    UniDist(T begin, T end) : _begin(begin), _end(end) {
        assert(begin <= end);
    }
    template<typename Gen>
    T operator()(Gen&& gen) {
        return UniDist::gen(_begin, _end, std::forward<Gen>(gen));
    }
    template<typename Gen>
    static T gen(T begin, T end, Gen&& gen) {
        if constexpr (std::is_integral_v<T>) { // make this better
            auto const range = end - begin + 1;
            auto res = gen() % range;
            return res + begin;
        } else {
            typedef typename std::remove_reference<Gen>::type gen_t;
            auto const urange = gen_t::max() - gen_t::min();
            auto const range = end - begin;
            return (static_cast<T>(gen()) / urange) * range + begin;
        }
    }
};

template<typename T>
class uniform_real_distribution {

};

template<typename... T> struct combine : T... {using T::operator()...;};

template<typename... T> combine(T...) -> combine<T...>; 

namespace detail {
    template<typename T, typename Gen>
    std::pair<T,T> generate_two(T x, T y, Gen&& gen) {
        T v = UniDist<T>::gen(x,y,gen);
        return {v/x, v%x};
    }
} /* namespace detail */

template<typename Iter, typename Gen>
void random_permute(Iter begin, Iter end, Gen&& gen) {
    if(begin == end) {
        return;
    }
    auto const len = end - begin;
    auto const range = Gen::max() - Gen::min();
    if(range / len >= len) { // faster variant
        auto it = begin + 1;
        if(len % 2 == 0) {
            swap(it++, begin + UniDist<decltype(len)>::gen(0, 1, gen));
        }
        while(it != end) {
            auto cnt = it - begin;
            auto p = detail::generate_two(cnt, cnt+1, gen);
            swap(it++, begin + p.first);
            swap(it++, begin + p.second);
        }
    } else { // for really big ranges
        for(auto it = begin; ++it != end;) {
            swap(it, begin + UniDist<decltype(len)>::gen(0, it - begin, gen));
        }
    }
}

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
            G.addEdge(i, UniDist<int>::gen(begin, end, gen));
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

template<typename TestcaseManager>
class Testing : private Output, private TestcaseManager {
    template<typename T>
    auto generate(Generating<T> const & schema) {
        return schema.generate(this->generator());
    }

public:
    using TestcaseManager::TestcaseManager;

    Testing(const Testing&) = delete;
    Testing(Testing&&) = delete;
    Testing& operator=(const Testing&) = delete;
    Testing& operator=(Testing&&) = delete;
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
    void test(T&&... args) {
        this->TestcaseManager::test(std::forward<T>(args)...);
        set(this->stream());
    }

    template<typename... T>
    void print(T const &... args) {
        dump_output((*this)(args)...);
    }

    template<typename T>
    decltype(auto) operator()(T const & t) { /* decltype(auto) does not decay static arrays to pointers */
        if constexpr (is_generating<T>::value) {
            return generate(t);
        } else {
            return t;
        }
    }

    template<typename T>
    Testing& operator<<(const T & out) {
        static_cast<std::ostream&>(*this) << (*this)(out);
        return * this;
    }
};

enum TestType : int8_t { OCEN };

namespace detail {
struct index {
    unsigned test;                          //NOLINT(misc-non-private-member-variables-in-classes)
    std::variant<TestType, unsigned> suite; //NOLINT(misc-non-private-member-variables-in-classes)

    [[nodiscard]] constexpr bool operator==(index const & x) const {
        return test == x.test && suite == x.suite;
    }

    class hash {
        constexpr inline static unsigned SHIFT = 10U;
    public:
        [[nodiscard]] constexpr std::size_t operator()(index const & indx) const {
            return static_cast<size_t>((indx.test << SHIFT) ^ 
            std::visit(combine{
                []([[maybe_unused]] TestType x){return 0U;},
                [](unsigned x){return x+1;}
            }, indx.suite));
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
            std::forward_as_tuple(std::hash<std::string>{}(name)));
        curr_test = &it.first->second;
    }

    class test_info : private std::pair<StreamType, gen_type> {
    public:
        using std::pair<StreamType, gen_type>::pair;
        [[nodiscard]] constexpr StreamType& stream() noexcept {
            return this->first;
        }
        [[nodiscard]] constexpr gen_type& generator() noexcept {
            return this->second;
        }
    };
    using index = detail::index;
    test_info * curr_test{nullptr};
    index curr_index{};
    std::string abbr;
    std::unordered_map<index, test_info, index::hash> cases{};

public:

    explicit OIOIOIManager(std::string abbr, bool ocen = false) : abbr(std::move(abbr)) {
        if(ocen)    {test(1, OCEN);} // pro1ocen.in
        else        {test(1, 1);}    // pro1a.in
    }

    OIOIOIManager() = delete;
    OIOIOIManager(OIOIOIManager const &) = delete;
    OIOIOIManager(OIOIOIManager&&) noexcept = default;
    ~OIOIOIManager() = default;
    OIOIOIManager& operator=(OIOIOIManager const &) = delete;
    OIOIOIManager& operator=(OIOIOIManager&&) noexcept = default;

    [[nodiscard]] constexpr StreamType & stream() const noexcept {
        return curr_test->stream();
    }

    [[nodiscard]] constexpr gen_type & generator() const noexcept {
        return curr_test->generator();
    }

    void nextTest() {
        std::visit(combine{
            [this](TestType x)  {this->test(this->curr_index.test + 1, x);},
            [this](unsigned x)       {this->test(this->curr_index.test + 1, x);}
        },curr_index.suite);
    }

    void nextSuite() {
        std::visit(combine{
            [this]([[maybe_unused]] TestType x) {this->test(1, 1);},
            [this](unsigned x)      {this->test(1, x + 1);}
        },curr_index.suite);
    }

    void test(unsigned test, unsigned suite) {
        curr_index = {test, suite};
        if(change_if_taken())   {return;}
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
        if(change_if_taken()) {return;}
        change_to_new_stream(abbr + std::to_string(test) + "ocen.in");
    }
    
};

template<typename T>
class Sequence : public std::vector<T> {

    template<typename Gen, std::enable_if_t<std::is_invocable_v<Gen>, int> = 0>
    static void seqGenerate(Sequence & s, Gen&& gen) {
        std::generate(s.begin(), s.end(), gen);
    }

    template<typename Gen, std::enable_if_t<std::is_invocable_v<Gen, unsigned>, int> = 0>
    static void seqGenerate(Sequence & s, Gen&& gen) {
        std::generate(s.begin(), s.end(), [&gen, cnt = 0U] () mutable {return std::invoke(gen, cnt++);});
    }

public:
    using std::vector<T>::vector;

    template<typename Gen, typename = std::enable_if_t<std::is_invocable_v<Gen> || std::is_invocable_v<Gen, unsigned>>>
    Sequence(std::size_t size, Gen&& gen) : std::vector<T>(size) {
        seqGenerate(*this, std::forward<Gen>(gen));
    }

    Sequence operator+(Sequence const & x) const {
        Sequence res(this->size() + x.size());
        auto const it = std::copy(this->begin(), this->end(), res.begin());
        std::copy(x.begin(), x.end(), it);
        return res;
    }

    Sequence& operator+=(Sequence const & x) {
        auto const old_size = this->size();
        this->resize(old_size + x.size());
        std::copy(x.begin(), x.end(), this->begin() + old_size);
        return * this;
    }

    friend std::ostream& operator<<(std::ostream & s, Sequence const & x) {
        auto it = x.begin();
        auto const end = x.end();
        if(it == end)   {return s;}
        s << *it++;
        while(it != end) {s << ' ' << *it++;}
        return s;
    }
};

} /* namespace test */