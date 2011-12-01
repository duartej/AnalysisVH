SHELL     = /bin/sh

PACKAGE     := $(shell basename `pwd`)
GENERALPATH := $(shell cd .. && pwd)
BINPATH     := $(GENERALPATH)/bin

CXX       = g++
CXXFLAGS  = -Wall

LDFLAGS   = -shared -fPIC

SOURCES    := $(wildcard *.cc)
EXE        := $(patsubst %.cc,$(BINPATH)/%,$(SOURCES))
PYSOURCES  := $(wildcard *.py)
PYEXE      := $(patsubst %.py,$(BINPATH)/%,$(PYSOURCES))

# executables which need some of our libraries
CPPSOURCES := $(wildcard *.cpp)
CPPEXE     := $(patsubst %.cpp,$(BINPATH)/%,$(CPPSOURCES))

CPPINCDIR  := -I$(GENERALPATH)/InputParameters/interface -I$(GENERALPATH)
CPPLIBDIR  := -L$(GENERALPATH)/libs
CPPLIBS    := -lInputParameters -lTResultsTable -lCutLevels

ROOTINCDIR ?= -I$(shell root-config --incdir)
ROOTLIBDIR ?= -L$(shell root-config --libdir)
ROOTLIBS   ?= $(shell root-config --libs)

all: $(EXE) $(PYEXE) $(CPPEXE)

$(EXE): $(SOURCES)
	@echo "++++ Creating Executable ($@)"
	@$(shell mkdir -p $(BINPATH))
	@$(CXX) $(CXXFLAGS) $(ROOTINCDIR) $(ROOTLIBDIR) $(ROOTLIBS) -o $@ $(patsubst $(BINPATH)/%,%.cc,$@)

$(PYEXE): $(PYSOURCES)
	@echo "++++ Copying python scripts ($@)"
	@cp $(patsubst $(BINPATH)/%,%.py,$@) $@
	@chmod 755 $@

$(CPPEXE): $(CPPSOURCES)
	@echo "++++ Creating Executable ($@)"
	@$(shell mkdir -p $(BINPATH))
	@$(CXX) $(CXXFLAGS) $(ROOTINCDIR) $(ROOTLIBDIR) $(ROOTLIBS) $(CPPINCDIR) \
		$(CPPLIBDIR) $(CPPLIBS) -o $@ $(patsubst $(BINPATH)/%,%.cpp,$@)

