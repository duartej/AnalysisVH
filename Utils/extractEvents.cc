//
// Utility to extract the number of events 
// of a filename or set of filenames (the Tree is fixed to 
// be called "Tree"--> FIXME: it can be changed
//

#ifndef __CINT__
#include<iostream>
#include<stdlib.h>
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
	
	const char *files = argv[1];
	// 
	if( files == "" )
	{
		std::cerr << "ERROR: you must enter a valid file name" << std::endl;
		exit(-1);
	}

	TChain * chain = new TChain("Tree");

	chain->Add(files);

	std::cout << chain->GetEntries() << std::endl;

	delete chain;
	chain = 0;

	return 0;
}
#endif

