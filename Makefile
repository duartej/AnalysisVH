SHELL = /bin/sh

PACKAGE   := $(shell basename `pwd`)

### Dependencies
DEPPKG     := VectorUtils TResultsTable TCounterUI PUWeight \
	DatasetManager InputParameters TreeManager CutLevels CutManager \
	CMSAnalysisSelector WManager FOManager WH_Analysis Utils
BASEDIR    := $(shell pwd)
DEPSDIR    := $(foreach DEPPKG,$(DEPPKG),$(BASEDIR)/$(DEPPKG))

### Macro to extract the path to the dependency libraries
getpkgbasedir  = $(foreach var,$(1), $(filter %/$(var),$(DEPSDIR))) 

### Setup content
SETUPFILE  := "\#!/bin/bash"  \
"BASEDIR=$(BASEDIR)" \
"export PATH=\$$PATH:\$$BASEDIR/bin:\$$BASEDIR/WH_Analysis/bin" \
"export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:\$$BASEDIR/libs" \
"export VHSYS=\$$BASEDIR" \
"export ANALYSISSYS=\$$BASEDIR/WH_Analysis"


.PHONY: all cleanall linkage compile
 
all: compile linkage setup.sh

compile:
	@# Checking if it is necessary to compile the dependency libraries
	@echo "======= $(PACKAGE): Compiling packages ======"
	@for libdir in $(DEPSDIR); do \
		$(MAKE) -C $$libdir; \
	done

linkage:
	@echo "======= $(PACKAGE): Creating library links ======"
	@mkdir -p $(BASEDIR)/libs 
	@for libdir in $(DEPSDIR); do \
		if [ ! -h libs/lib`basename $$libdir`.so ]; then \
		       ln -s $$libdir/lib/lib`basename $$libdir`.so libs/lib`basename $$libdir`.so; \
		fi; \
	done
	@rm -f $(BASEDIR)/libs/libUtils.so

setup.sh: cleansetup
	@echo "======= $(PACKAGE): Creating setup script ======"
	@for line in $(SETUPFILE) ; do echo $$line >> setup.sh.tmp; done
	@mv setup.sh.tmp setup.sh
	@chmod 755 setup.sh


cleanall: 
	@for libdir in $(DEPSDIR); do \
		$(MAKE) -C $$libdir clean; \
	done

cleansetup:
	@rm -f setup.sh


	
