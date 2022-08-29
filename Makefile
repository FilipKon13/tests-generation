BASEDIR:=.
SRCDIR:= $(BASEDIR)/src
TESTDIR:= $(BASEDIR)/test
TESTS:= $(wildcard $(TESTDIR)/*.t.cpp)
CPPC:= g++
CPPFLAGS:= -g -std=c++17 -Wall -Werror
TESTFLAGS:= -I $(SRCDIR)
TESTTARGET:= $(patsubst $(TESTDIR)/%.t.cpp,%.test,$(TESTS))
TESTOBJ:= $(patsubst %.test,$(TESTDIR)/out/%.t,$(TESTTARGET))

test: $(TESTTARGET)

%.test: $(TESTDIR)/out/%.t
	./$<

$(TESTDIR)/out/%.t: $(TESTDIR)/%.t.cpp
	$(CPPC) $(CPPFLAGS) $(TESTFLAGS) -o $@ $^

clean:
	rm -f $(TESTOBJ)

.SECONDARY: $(TESTOBJ)

EMPTY:=
SPACE:= $(EMPTY) $(EMPTY)
COMMA:= ,$(SPACE)
.PHONY: test, $(subst $(SPACE),$(COMMA),$(TESTTARGET)), clean

