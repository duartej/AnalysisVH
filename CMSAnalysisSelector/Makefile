SHELL     = /bin/sh

PACKAGE   := $(shell basename `pwd`)

CXX       = g++
CXXFLAGS  = -Wall #-DDEBUGCMSANALYSISSELECTOR=3

LDFLAGS   = -shared -fPIC

SOURCES    := $(wildcard src/*.cc)
HEADERS    := $(wildcard interface/*.h)
# VH_Analysis include dir
INCDIR     := -I$(shell OLDPWD=pwd && cd .. && pwd & cd ${OLDPWD})
INCDIR     += -I$(shell pwd)/interface
#-- Needed for the dict
INCDIR     += -I$(shell pwd)

### Dependencies
DEPPKG     := InputParameters TCounterUI TreeManager
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

#### Variables to generate the dictionaries
DICTSRC    := $(shell echo $(SOURCES)| cut -d. -f1 | tr A-Z a-z )dict.cxx
DICTHEADER := $(patsubst src/%.cxx,interface/%.h,$(DICTSRC))
DICTHEADERNAME := $(patsubst interface/%.h,%.h,$(DICTHEADER))

OBJDIR     := build
OBJECTS    := $(patsubst src/%.cc,$(OBJDIR)/%.o,$(wildcard src/*.cc))
###### Dictionary object
DICTOBJECT := $(patsubst src/%.cxx,$(OBJDIR)/%.o,$(DICTSRC))

### Extracting the dict header (if proceed)
HEADERS := $(filter-out $(DICTHEADER), $(HEADERS))

### Macro to extract the path to the dependency libraries
getpkgbasedir  = $(foreach var,$(1), $(filter %/$(var),$(DEPSDIR))) 

all: lib/lib$(PACKAGE).so

### Creating the dictionaries
$(DICTSRC): $(HEADERS)
	@echo "=== $(PACKAGE): ++++++ Creating dictionary"
	@if [ -f $(DICTSRC) ]; then \
		rm $(DICTSRC) ; \
	fi
	@if [ -f $(DICTHEADER) ]; then \
		rm $(DICTHEADER) ; \
	fi
	@$(ROOTCINT) $(DICTSRC) -c $(DEPSINC) $(HEADERS)
	@if [ -f src/$(DICTHEADERNAME) ]; then \
		mv src/$(DICTHEADERNAME) $(DICTHEADER); \
	else \
	        echo "ERROR: Not generated the dictionary!" \
		exit -2; \
	fi


$(DICTOBJECT): $(DICTSRC)
	@echo "=== $(PACKAGE): +++++ Creating dictionary objects"
	@$(shell mkdir -p build)
	@$(CXX) $(LDFLAGS) -MD -c $(CXXFLAGS) $(DEPSINC) $(INCDIR) $(ROOTINCDIR) -o $@ $<


-include $(patsubst src/%.cc,$(OBJDIR)/%.d,$(SOURCES))


$(OBJECTS): $(SOURCES) $(HEADERS)
	@echo "=== $(PACKAGE): ++++ Creating objects"
	@$(shell mkdir -p build)
	@$(CXX) $(LDFLAGS) -MD -c $(CXXFLAGS) $(DEPSINC) $(INCDIR) $(ROOTINCDIR) -o $@ $<


lib/lib$(PACKAGE).so: $(DICTOBJECT) $(OBJECTS) 
	@echo "=== $(PACKAGE): +++ Linking"
	@$(shell mkdir -p lib)
	@$(CXX) $(LDFLAGS) $(CXXFLAGS) $(DEPSLIBDIR) $(ROOTLIBDIR) $(ROOTLIBS) $^ -o $@  
	@echo "... Done $(PACKAGE)"


.PHONY: clean

clean:
	@if [ -f lib/lib$(PACKAGE).so ]; then \
		echo "++++++ $(PACKAGE): Cleaning ...." ; \
		rm lib/lib$(PACKAGE).so $(OBJECTS) $(DICTOBJECT); \
		rmdir lib; \
		rm -rf $(OBJDIR); \
		rm $(DICTHEADER) ; \
		rm $(DICTSRC) ; \
	else  \
		echo "make: Nothing to be done for \`clean'."; \
	fi;

