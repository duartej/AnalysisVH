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

#ifndef __CINT__
int main( int argc, const char * argv[])
{
	if( argc == 1)
	{
		std::cerr << "usage: extractEvents filename*" << std::endl;
		std::cerr << "" << std::endl;
		std::cerr << "filename* could be a wildcard pattern" << std::endl;
		exit(-1);
	}
	
	std::vector<std::string> files;
	for(int i = 1; i < argc; ++i)
	{
		files.push_back( argv[i] );
	}
	// 
	if( files.size() == 0 )
	{
		std::cerr << "ERROR: you must enter a valid file name" << std::endl;
		exit(-1);
	}

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

