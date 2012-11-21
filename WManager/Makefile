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

### Dependencies
DEPPKG     := LeptonRel CutManager
BASEDIR    := $(shell cd .. && pwd && cd ..)
DEPSDIR    := $(foreach DEPPKG,$(DEPPKG),$(BASEDIR)/$(DEPPKG))
DEPSLIBS   := $(foreach DEPPKG,$(DEPPKG),-l$(DEPPKG))
DEPSLIBDIR := $(foreach DEPSDIR,$(DEPSDIR),-L$(DEPSDIR)/lib)
DEPSSEARCHLIBDIRS := $(foreach DEPSDIR,$(DEPSDIR),$(DEPSDIR)/lib)
DEPSINC    := $(foreach DEPSDIR,$(DEPSDIR),-I$(DEPSDIR) -I$(DEPSDIR)/interface)

# Directory to search the libraries (-l) dependencies 
vpath %.so $(subst  ,:,$(DEPSSEARCHLIBDIRS))

### ROOT-stuff
ROOTINCDIR ?= -I$(shell root-config --incdir)
ROOTLIBDIR ?= -L$(shell root-config --libdir)
ROOTLIBS   ?= $(shell root-config --libs)
ROOTCINT   ?= rootcint

OBJDIR     := build
OBJECTS    := $(patsubst src/%.cc,$(OBJDIR)/%.o,$(wildcard src/*.cc))
###### Dictionary object
DICTOBJECT := $(patsubst src/%.cxx,$(OBJDIR)/%.o,$(DICTSRC))

### Macro to extract the path to the dependency libraries
getpkgbasedir  = $(foreach var,$(1), $(filter %/$(var),$(DEPSDIR))) 

all: lib/lib$(PACKAGE).so

-include $(patsubst src/%.cc,$(OBJDIR)/%.d,$(SOURCES))

$(OBJECTS): $(SOURCES) $(HEADERS)
	@echo "=== $(PACKAGE): ++++ Creating objects ($@)"
	@$(shell mkdir -p build)
	@$(CXX) $(LDFLAGS) -MD -c $(CXXFLAGS) $(DEPSINC) $(INCDIR) $(ROOTINCDIR) -o $@ \
		$(patsubst $(OBJDIR)/%.o,src/%.cc,$@)


lib/lib$(PACKAGE).so: $(OBJECTS)
	@echo "=== $(PACKAGE): +++ Linking"
	@$(shell mkdir -p lib)
	@$(CXX) $(LDFLAGS) $(CXXFLAGS) $(DEPSLIBDIR) $(ROOTLIBDIR) $(ROOTLIBS) $^ -o $@  
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

