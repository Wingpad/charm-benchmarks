-include ../../common.mk
CHARMC = $(CHARM_HOME)/bin/charmc
BINARY = tester

CHARMCFLAGS = $(OPTS)
CHARMCLINKFLAGS = -language charm++ $(OPTS)

CMK_NUM_PES?=2

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
	./charmrun +p$(CMK_NUM_PES) ./$(BINARY) $(TESTOPTS)

clean:
	rm -f *.o *.decl.h *.def.h charmrun $(BINARY)
