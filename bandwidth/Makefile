-include ../../common.mk
CHARMC=$(CHARM_HOME)/bin/charmc
BINARY=tester

USE_SDAG?=1
OPTS:=$(OPTS) -DUSE_SDAG=$(USE_SDAG)
CHARMCFLAGS=$(OPTS) -Ofast -g3
CHARMCLINKFLAGS=$(OPTS) -language charm++

CMK_NUM_PES?=2
TESTOPTS?=128 128

%.o: %.cc

all: $(BINARY)
$(BINARY): $(patsubst %.cc,%.o,$(wildcard *.cc))
	$(CHARMC) $(CHARMCLINKFLAGS) -o $@ $+ 

.SECONDARY: $(patsubst %.cc,%.decl.h,$(wildcard *.cc))
.SECONDARY: $(patsubst %.cc,%.def.h,$(wildcard *.cc))

%.o: %.cc %.decl.h %.def.h
	$(CHARMC) $(CHARMCFLAGS) $<

%.def.h: %.decl.h ;

%.decl.h: %.ci
	$(CHARMC) $(CHARMCFLAGS) $<

test: $(BINARY)
	./charmrun +p$(CMK_NUM_PES) ./$(BINARY) $(TESTOPTS)

clean:
	rm -f *.o *.decl.h *.def.h charmrun $(BINARY)
