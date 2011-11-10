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

ROOTINCDIR ?= -I$(shell root-config --incdir)
ROOTLIBDIR ?= -L$(shell root-config --libdir)
ROOTLIBS   ?= $(shell root-config --libs)
# To include the package libraries
ROOTLIBDIR += -L$(GENERALPATH)/libs   


all: $(EXE) $(PYEXE)


$(EXE): $(SOURCES)
	@echo "++++ Creating Executable ($@)"
	@$(shell mkdir -p $(BINPATH))
	@$(CXX) $(CXXFLAGS) $(ROOTINCDIR) $(ROOTLIBDIR) $(ROOTLIBS) -o $@ $(patsubst $(BINPATH)/%,%.cc,$@)

$(PYEXE): $(PYSOURCES)
	@echo "++++ Copying python scripts ($@)"
	@cp $(patsubst $(BINPATH)/%,%.py,$@) $@
	@chmod 755 $@


