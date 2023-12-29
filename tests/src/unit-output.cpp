#include "doctest.h"
#include <sstream>

#include <testgen/output.hpp>
using namespace test;

TEST_CASE("test_empty") {
    std::stringstream stream;
    Output out(stream);

    out.dumpOutput();

    CHECK(stream.str() == "");
}

TEST_CASE("test_single_int") {
    std::stringstream stream;
    Output out(stream);

    out.dumpOutput(1);

    CHECK(stream.str() == "1\n");
}

TEST_CASE("test_single_space") {
    std::stringstream stream;
    Output out(stream);

    out.dumpOutput(SPACE);

    CHECK(stream.str() == " ");
}

TEST_CASE("test_multiple_no_space") {
    std::stringstream stream;
    Output out(stream);

    out.dumpOutput(1, "22", std::string("333"));

    CHECK(stream.str() == "1\n22\n333\n");
}

TEST_CASE("test_multiple_space") {
    std::stringstream stream;
    Output out(stream);

    out.dumpOutput(1, SPACE, "22", SPACE, std::string("333"));

    CHECK(stream.str() == "1 22 333\n");
}

TEST_CASE("test_space_at_end") {
    std::stringstream stream;
    Output out(stream);

    out.dumpOutput(1, SPACE, "22", std::string("333"), SPACE);

    CHECK(stream.str() == "1 22\n333 ");
}

TEST_CASE("output-change-test") {
    Output out;
    std::stringstream out1{};
    std::stringstream out2{};

    out.set(out1);
    out << 12 << '\n';
    out.set(out2);
    out << 13 << '\n';
    out.set(out1);
    out << 14 << '\n';
    out.set(out2);
    out << 15 << '\n';

    CHECK(out1.str() == "12\n14\n");
    CHECK(out2.str() == "13\n15\n");
}

TEST_CASE("test-print-graph") {
    std::stringstream out{};
    Graph const G = Clique(3).generate();
    printEdges(out, G);
    CHECK(out.str() == "0 1\n0 2\n1 2\n");
}

TEST_CASE("test-print-graph-shift") {
    std::stringstream out{};
    Graph const G = Clique(3).generate();
    printEdges(out, G, 1);
    CHECK(out.str() == "1 2\n1 3\n2 3\n");
}

TEST_CASE("test-print-tree") {
    std::stringstream out{};
    Graph G(4);
    G.addEdge(0, 1);
    G.addEdge(0, 2);
    G.addEdge(0, 3);
    printEdgesAsTree(out, G);
    CHECK(out.str() == "0\n0\n0\n");
}

TEST_CASE("test-print-tree-shift") {
    std::stringstream out{};
    Graph G(4);
    G.addEdge(0, 1);
    G.addEdge(0, 2);
    G.addEdge(0, 3);
    printEdgesAsTree(out, G, 1);
    CHECK(out.str() == "1\n1\n1\n");
}
