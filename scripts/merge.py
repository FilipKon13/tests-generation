import os
import re
import sys

# This simple script is basically limited preprocessor
# Its task is to merge all .hpp files in SRC_PATH folder into single 'testgen.hpp' file

SRC_PATH = sys.argv[1]
TARGET_PATH = sys.argv[2]
reg_local = re.compile(R' *#include +".+"')
reg_normal = re.compile(R' *#include +<.+>')

filemap = dict()
includes_set = set()

class file:
    def __init__(self, path, name):
        self.depen = []
        self.name = name
        self.path = path
        with open(path, 'r') as file:
            for line in file:
                if reg_local.match(line):
                    self.depen.append(line.split('\"')[1])
                elif reg_normal.match(line):
                    inside = line.split("<")[1].split(">")[0]
                    includes_set.add(f"#include <{inside}>\n")

for root, dirs, files in os.walk(SRC_PATH):
    for name in files:
        path = os.path.join(root, name)
        filemap[name] = file(path, name)

visited = set()
last = False
def dfs(file : file, output):
    global last
    visited.add(file.name)
    for v in file.depen:
        if v not in visited:
            dfs(filemap[v], output)
    output.write("/* ")
    output.write("=" * 20)
    output.write(" " + file.name + " ")
    output.write("=" * 20)
    output.write("*/\n\n");
    last = True
    with open(file.path, 'r') as inp:
        for line in inp:
            if line == '\n':
                if last:
                    continue
                else:
                    last = True
            if not line.startswith("#") and not "namespace test" in line:
                if line != '\n':
                    last = False
                output.write(line)

with open(TARGET_PATH, 'w') as out:
    out.write("\n/* Source: https://github.com/FilipKon13/tests-generation */\n\n")
    out.write("#ifndef TESTGEN_HPP_\n")
    out.write("#define TESTGEN_HPP_\n\n")
    for include in sorted(includes_set):
        out.write(include)
    out.write('\nnamespace test {\n')
    for file in filemap.values():
        if file.name not in visited:
            dfs(file, out)
    out.write('} /* namespace test */')
    out.write("\n\n#endif /* TESTGEN_HPP_ */\n")