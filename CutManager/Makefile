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
DEPPKG     := TreeManager InputParameters LeptonRel
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

### Macro to convert a the header name to a dictionary .cc filename
getdictname   = $(patsubst interface/%,%,$(shell echo $(1)|cut -d. -f1| tr A-Z a-z)dict.cxx)

#### Variables to generate the dictionaries:
#- Extract all the headers containing ClassDef call
TARGETDICTHEADER := $(shell grep -H -R ClassDef ./interface/*.h | sed -e 's/[^:]:*ClassDef[^ ]*//' -e 's/://' -e 's/\.\///')
#- Creating the dict names: .h and .cc
DICTHEADER       := $(foreach VAR,$(TARGETDICTHEADER),$(shell echo $(VAR) | cut -d. -f1| tr A-Z a-z)dict.h )
DICTSRC          := $(patsubst interface/%.h,src/%.cxx,$(DICTHEADER))
DICTHEADERNAME   := $(patsubst interface/%.h,%.h,$(DICTHEADER))
#- Useful to link the header with the dict.cc
PAIRDICTHEADER := $(foreach VAR,$(TARGETDICTHEADER), $(VAR):$(call getdictname,$(VAR)))

###### Objects
OBJDIR     := build
OBJECTS    := $(patsubst src/%.cc,$(OBJDIR)/%.o,$(wildcard src/*.cc))
###### Dictionary object
DICTOBJECT := $(patsubst src/%.cxx,$(OBJDIR)/%.o,$(DICTSRC))

### Extracting the dict header (if proceed)
HEADERS := $(filter-out $(DICTHEADER), $(HEADERS))

### Macro to extract the path to the dependency libraries
getpkgbasedir  = $(foreach var,$(1),$(filter %/$(var),$(DEPSDIR))) 

### Macro to extract the header counterpart to a dictionary source name
getheaderdict = $(patsubst %:$(1),%,$(filter %:$(1), $(PAIRDICTHEADER)))


all: lib/lib$(PACKAGE).so


### Creating the dictionaries
$(DICTSRC): $(TARGETDICTHEADER)
	@echo "=== $(PACKAGE): ++++++ Creating dictionary ($@)"
	@rm -f $@ $(patsubst src/%.cxx,interface/%.h,$@)
	@$(ROOTCINT) $@ -c $(DEPSINC) $(call getheaderdict,$(patsubst src/%,%,$@))
	@if [ -f src/$(patsubst src/%.cxx,%.h,$@) ]; then \
		mv src/$(patsubst src/%.cxx,%.h,$@) interface/$(patsubst src/%.cxx,%.h,$@); \
	else \
	        echo "ERROR: Not generated the dictionary!" \
		exit -2; \
	fi


$(DICTOBJECT): $(DICTSRC)
	@echo "=== $(PACKAGE): +++++ Creating dictionary objects ($@)"
	@$(shell mkdir -p build)
	@$(CXX) $(LDFLAGS) -MD -c $(CXXFLAGS) $(DEPSINC) $(INCDIR) $(ROOTINCDIR) -o $@ $(patsubst build/%.o,src/%.cxx,$@)


-include $(patsubst src/%.cc,$(OBJDIR)/%.d,$(SOURCES))


$(OBJECTS): $(SOURCES) $(HEADERS)
	@echo "=== $(PACKAGE): ++++ Creating objects ($@)"
	@$(shell mkdir -p build)
	@$(CXX) $(LDFLAGS) -MD -c $(CXXFLAGS) $(DEPSINC) $(INCDIR) $(ROOTINCDIR) -o $@ \
		$(patsubst $(OBJDIR)/%.o,src/%.cc,$@)


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

