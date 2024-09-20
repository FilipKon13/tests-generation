#include <bits/stdc++.h>

#include "testgen.hpp"

using namespace std;
using namespace test;

struct Testcase {
    int n;
    Graph G;
    friend ostream & operator<<(ostream & s, Testcase const & t) {
        s << t.n << '\n';
        for(auto [a, b] : t.G.getEdges())
            s << a << ' ' << b << '\n';
        return s;
    }
};

int main() {
    Testing<OIOIOIManager<>, Testcase> test("gra", true);
    test.assumptionGlobal([](Testcase t) {
        return t.G.size() == t.n && t.G.getEdges().size() == t.n - 1;
    });
    test.getTest(); // suite OCEN

    { // gra1ocen.in
        auto out = "3\n1 2\n1 3\n";
        test << out; // usage as normal stream
    }

    test.skipTest();
    { // gra2ocen.in
        // int n = 10;
        // test << n << '\n'
        //      << Tree(n) << '\n'; // using generator dedicated to particular testcase to generate random Tree
    }

    { // gra3ocen.in
        test.nextTest();
        Testcase t;
        t.n = 100;
        t.G = test.generateFromSchema(Tree(t.n)); // using generator dedicated to particular testcase to generate random Tree
        test << t;                                // checks assumptions
    }

    test.nextSuite(); // changes suite to 1st

    test.assumptionSuite([](Testcase t) {
        return t.n <= 1000;
    });

    { // gra1a.in
        test.nextTest();
        int n = 900;
        Graph p1 = Path(n / 3).generate();                 // deterministic path
        Graph p2 = test.generateFromSchema(Path(n / 3));   // random path
        Graph p3 = Path(n / 3).generate(test.generator()); // same as above
        test << Testcase{n, merge(merge(p1, p2, {{0, 0}}), p3, {{0, 0}})};
    }

    { // gra1b.in
        auto tc = test.getTest();
        auto & [n, G] = tc;
        n = test.randInt(1, 1000);
        G = test.generateFromSchema(Tree(n));
        test << tc;
    }

    { // gra1c.in
        test.nextTest();
        int n = 1000;
        Graph p1 = Path(n / 2).generate();
        Graph p2 = test.generateFromSchema(Tree(n / 2));
        // result of below is stable, i.e. does not depend on what was generated in previous testcases
        // or any changes in it
        test << Testcase{n, merge(p1, p2, {{0, 0}})};
    }

    test.nextSuite(); // drops previous suite assumptions, keeps the global one

    { // gra2a.in
        test.nextTest();
        test.assumptionTest([](Testcase const & t) { // we can take Testcase by value or by reference
            return t.n == 1;
        });
        // assumption from above will not be used in the next test (even if in the same suite)
        test << Testcase{1, Graph(1)};
    }
    { // gra2b.in

        auto tc = test.getTest(); // get testcase from getTest function
        auto & [n, G] = tc;
        n = 1000;
        G = test.generateFromSchema(Tree(n));
        test << tc;
    }
}