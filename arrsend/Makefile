-include ../../common.mk
CHARMC = $(CHARM_HOME)/bin/charmc
BINARY = tester

OPTS:=$(OPTS) -Ofast

CHARMCFLAGS = $(OPTS)
CHARMCLINKFLAGS = -language charm++ $(OPTS)

CATCHSEGV="$(shell which catchsegv)"

%.o: %.C

all: $(BINARY)
$(BINARY): $(patsubst %.C,%.o,$(wildcard *.C))
	$(CHARMC) $(CHARMCLINKFLAGS) -o $@ $+ 

.SECONDARY: $(patsubst %.C,%.decl.h,$(wildcard *.C))
.SECONDARY: $(patsubst %.C,%.def.h,$(wildcard *.C))

%.o: %.C %.decl.h %.def.h
	$(CHARMC) $(CHARMCFLAGS) $<

%.def.h: %.decl.h ;

%.decl.h: %.ci
	$(CHARMC) $(CHARMCFLAGS) $<

test: $(BINARY)
	$(CHARM_HOME)/bin/testrun +p1 ./$(BINARY) $(TESTOPTS)

clean:
	rm -f *.o *.decl.h *.def.h charmrun $(BINARY)
