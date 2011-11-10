//
// Utility to extract the number of events 
// of a filename or set of filenames (the Tree is fixed to 
// be called "Tree"--> FIXME: it can be changed
//

#ifndef __CINT__
#include<iostream>
#include<stdlib.h>
#include<vector>
#include<cstring>
#endif

#include "TFile.h"
#include "TChain.h"
#include "TH1D.h"

#ifndef __CINT__
int main( int argc, const char * argv[])
{
	bool histofile = false;

	if( argc == 1)
	{
		std::cerr << "usage: extractEvents filename* [options]" << std::endl;
		std::cerr << "" << std::endl;
		std::cerr << "filename* could be a wildcard pattern" << std::endl;
		std::cerr << "Options:" << std::endl;
		std::cerr << "    -h  flag to use final datafiles (already processed) " << std::endl;
		exit(-1);
	}
	
	std::vector<std::string> files;
	for(int i = 1; i < argc; ++i)
	{
		if( strcmp(argv[i],"-h") == 0 )
		{
			histofile = true;
			continue;
		}
		else
		{
			files.push_back( argv[i] );
		}
	}
	// 
	if( files.size() == 0 )
	{
		std::cerr << "ERROR: you must enter a valid file name" << std::endl;
		exit(-1);
	}
	
	// Processed files, just look into the histograms
	if( histofile )
	{
		TFile * f = new TFile(files[0].c_str());
		TH1D *  h = (TH1D*)f->Get("fHProcess");
		std::cout << int(h->GetEntries()) << std::endl;

		f->Close();
		delete f;

		return 0;
	}

	//Else look a chain
	TChain * chain = new TChain("Tree");

	for(unsigned int i = 0; i < files.size(); ++i)
	{
		chain->Add(files[i].c_str());
	}

	std::cout << chain->GetEntries() << std::endl;

	delete chain;
	chain = 0;

	return 0;
}
#endif

