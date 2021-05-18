-include ../../common.mk
CHARMC = $(CHARM_HOME)/bin/charmc
BINARY = secdest

OPTS:=$(OPTS) -g -O2

TESTOPTS:=$(TESTOPTS) 256

CHARMCFLAGS = $(OPTS)
CHARMCLINKFLAGS = -language charm++ $(OPTS) -lSegFault

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
	$(CHARM_HOME)/bin/testrun $(CATCHSEGV) +p4 ./$(BINARY) ++ppn 4  $(TESTOPTS)

clean:
	rm -f *.o *.decl.h *.def.h charmrun $(BINARY)
