BASEDIR:=.
SRCDIR:= $(BASEDIR)/src
TESTDIR:= $(BASEDIR)/test
TESTS:= $(wildcard $(TESTDIR)/*.t.cpp)
CPPC:= g++
CPPFLAGS:= -O3 -std=c++17 -Wall -Werror
TESTFLAGS:= -g -I $(SRCDIR)
TESTTARGET:= $(patsubst $(TESTDIR)/%.t.cpp,%.test,$(TESTS))
TESTOBJ:= $(patsubst %.test,$(TESTDIR)/out/%.t,$(TESTTARGET))
EMPTY:=
SPACE:= $(EMPTY) $(EMPTY)
COMMA:= ,$(SPACE)
SRCFILES:= $(wildcard $(SRCDIR)/*.hpp)
CLANG_TIDY_FLAGS:= -checks=*,-modernize-use-trailing-return-type,-llvm-header-guard,-fuchsia-overloaded-operator,-fuchsia-multiple-inheritance,-misc-definitions-in-headers,-fuchsia-default-arguments-declarations,-fuchsia-default-arguments-calls,-google-runtime-references,-fuchsia-trailing-return

test: $(TESTTARGET)

tidy:
	clang-tidy $(CLANG_TIDY_FLAGS) $(SRCFILES) -- -std=c++17

%.test: $(TESTDIR)/out/%.t
	./$<

$(TESTDIR)/out/%.t: $(TESTDIR)/%.t.cpp $(SRCFILES)
	$(CPPC) $(CPPFLAGS) $(TESTFLAGS) -o $@ $<

clean:
	rm -f $(TESTOBJ)

.SECONDARY: $(TESTOBJ)

.PHONY: test, $(subst $(SPACE),$(COMMA),$(TESTTARGET)), clean, tidy

