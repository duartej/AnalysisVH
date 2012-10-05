SHELL     = /bin/sh

PACKAGE     := $(shell basename `pwd`)
GENERALPATH := $(shell cd .. && pwd)
BINPATH     := $(GENERALPATH)/bin
MODULEPATH  := $(GENERALPATH)/python

CXX       = g++
CXXFLAGS  = -Wall

LDFLAGS   = -shared -fPIC

SOURCES    := $(wildcard *.cc)
EXE        := $(patsubst %.cc,$(BINPATH)/%,$(SOURCES))
PYSOURCES  := $(filter-out __init__.py,$(filter-out $(wildcard *_mod.py),$(wildcard *.py)))
PYSRCMOD   := $(wildcard *_mod.py)
PYSRCMOD   += $(wildcard __init__.py)
PYEXE      := $(patsubst %.py,$(BINPATH)/%,$(PYSOURCES))
PYMODULES  := $(patsubst %.py,$(MODULEPATH)/%.py,$(PYSRCMOD))
SHSOURCES  := $(wildcard *.sh)
SHEXE      := $(patsubst %.sh,$(BINPATH)/%,$(SHSOURCES))

# executables which need some of our libraries
CPPSOURCES := $(wildcard *.cpp)
CPPEXE     := $(patsubst %.cpp,$(BINPATH)/%,$(CPPSOURCES))

CPPINCDIR  := -I$(GENERALPATH)/InputParameters/interface -I$(GENERALPATH)
CPPLIBDIR  := -L$(GENERALPATH)/libs
CPPLIBS    := -lInputParameters -lCutLevels

ROOTINCDIR ?= -I$(shell root-config --incdir)
ROOTLIBDIR ?= -L$(shell root-config --libdir)
ROOTLIBS   ?= $(shell root-config --libs)


all: | $(PYEXE) $(PYMODULES) $(EXE) $(CPPEXE) $(SHEXE) 

$(SHEXE): $(SHSOURCES)
	@echo "++++ Copying bash scripts ($@)"
	@$(shell mkdir -p $(BINPATH))
	@cp $(patsubst $(BINPATH)/%,%.sh,$@) $@
	@chmod 755 $@

$(EXE): $(SOURCES)
	@echo "++++ Creating Executable ($@)"
	@$(shell mkdir -p $(BINPATH))
	@$(CXX) $(CXXFLAGS) $(ROOTINCDIR) $(ROOTLIBDIR) $(ROOTLIBS) -o $@ $(patsubst $(BINPATH)/%,%.cc,$@)

$(PYMODULES): $(PYSRCMOD)
	@echo "++++ Copying python modules ($@)"
	@$(shell mkdir -p $(MODULEPATH))
	@cp $(patsubst $(MODULEPATH)/%,%,$@) $@
	@echo "++++ Creating python modules ($(MODULEPATH)/__init__.py)"
	@if [ ! -f $(MODULEPATH)/__init__.py ]; then \
		touch $(MODULEPATH)/__init__.py;\
	fi;


$(PYEXE): $(PYSOURCES)
	@echo "++++ Copying python scripts ($@)"
	@$(shell mkdir -p $(BINPATH))
	@cp $(patsubst $(BINPATH)/%,%.py,$@) $@
	@chmod 755 $@

$(CPPEXE): $(CPPSOURCES)
	@echo "++++ Creating Executable ($@)"
	@$(shell mkdir -p $(BINPATH))
	@$(CXX) $(CXXFLAGS) $(ROOTINCDIR) $(ROOTLIBDIR) $(ROOTLIBS) $(CPPINCDIR) \
		$(CPPLIBDIR) $(CPPLIBS) -o $@ $(patsubst $(BINPATH)/%,%.cpp,$@)

