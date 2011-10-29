SHELL     = /bin/sh

PACKAGE   := $(shell basename `pwd`)

CXX       = g++
CXXFLAGS  = -Wall

LDFLAGS   = -shared -fPIC

SOURCES    := $(wildcard src/*.cc)
HEADERS    := $(wildcard interface/*.h)
# VH_Analysis include dir
INCDIR     := -I$(shell OLDPWD=pwd && cd .. && pwd & cd ${OLDPWD})
INCDIR     += -I$(shell pwd)/interface
#-- Needed for the dict
INCDIR     += -I$(shell pwd)

ROOTINCDIR ?= -I$(shell root-config --incdir)
ROOTLIBDIR ?= -L$(shell root-config --libdir)
ROOTLIBS   ?= $(shell root-config --libs)

OBJDIR     := build
OBJECTS    := $(patsubst src/%.cc,$(OBJDIR)/%.o,$(wildcard src/*.cc))


all: lib/lib$(PACKAGE).so


-include $(patsubst src/%.cc,$(OBJDIR)/%.d,$(SOURCES))


$(OBJECTS): $(SOURCES) $(HEADERS)
	@echo "=== $(PACKAGE): ++++ Creating objects"
	@$(shell mkdir -p build)
	@$(CXX) $(LDFLAGS) -MD -c $(CXXFLAGS) $(INCDIR) $(ROOTINCDIR) -o $@ $<


lib/lib$(PACKAGE).so: $(DICTOBJECT) $(OBJECTS) 
	@echo "=== $(PACKAGE): +++ Linking"
	@$(shell mkdir -p lib)
	@$(CXX) $(LDFLAGS) $(CXXFLAGS) $(ROOTLIBDIR) $(ROOTLIBS) $^ -o $@  
	@echo "... Done $(PACKAGE)"


.PHONY: clean

clean:
	@if [ -f lib/lib$(PACKAGE).so ]; then \
		echo "++++++ $(PACKAGE): Cleaning ...." ; \
		rm lib/lib$(PACKAGE).so $(OBJECTS); \
		rmdir lib; \
		rm -rf $(OBJDIR); \
	else  \
		echo "make: Nothing to be done for \`clean'."; \
	fi;

