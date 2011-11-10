SHELL = /bin/sh

PACKAGE   := $(shell basename `pwd`)


### Dependencies
DEPPKG     := VectorUtils TResultsTable TCounterUI PUWeight \
	DatasetManager InputParameters TreeManager CutManager \
	CMSAnalysisSelector WH_Analysis Utils
BASEDIR    := $(shell pwd )
DEPSDIR    := $(foreach DEPPKG,$(DEPPKG),$(BASEDIR)/$(DEPPKG))

### Macro to extract the path to the dependency libraries
getpkgbasedir  = $(foreach var,$(1), $(filter %/$(var),$(DEPSDIR))) 


.PHONY: all cleanall
 
all: 
	@# Checking if it is necessary to compile the dependency libraries
	@echo "======= $(PACKAGE): Compiling packages ======"
	@echo $(DEPSDIR)
	@for libdir in $(DEPSDIR); do \
		$(MAKE) -C $$libdir; \
	done
	@echo "======= $(PACKAGE): Creating library links ======"
	@mkdir -p $(BASEDIR)/libs 
	@for libdir in $(DEPSDIR); do \
		if [ ! -h libs/lib`basename $$libdir`.so ]; then \
		       ln -s $$libdir/lib/lib`basename $$libdir`.so libs/lib`basename $$libdir`.so; \
		fi; \
	done


cleanall: 
	@for libdir in $(DEPSDIR); do \
		$(MAKE) -C $$libdir clean; \
	done


	
