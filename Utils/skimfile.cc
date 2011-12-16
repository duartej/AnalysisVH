/*
   skimfile.cpp

   copies the contents of a File, but only
   storing the events which pass the cut N

Compilation: gcc `root-config --libs` -I`root-config --incdir` -o skimfile skimfile.cc

Arguments: Name of the cut
Options:   -o Name of the output root file
           -i Name of the input root file
	   


   J. Duarte Campderros, 12-12-2011 (Based in pytnp/procfiles/CowboysNTuple.cpp)

   duarte@ifca.unican.es
*/
#ifndef __CINT__

#include<string>
#include<iostream>
#include<sstream>
#include<stdio.h>
#include<stdlib.h>
#include<cmath>
#include<set>
#include<vector>

//#include "TDirectory.h"
//#include "TROOT.h"
#endif

//#include "TKey.h"
#include "TFile.h"
#include "TChain.h"
//#include "TSystem.h"
#include "TTree.h"
//#include <exception>

void CopyFiles(const char *inputName, const char *outputName, const char * treename, 
		const char * cutfile, const int & cutThreshold, const std::pair<int,int> & events) 
{
      	TChain * t = new TChain(treename);
      	const int nfiles = t->Add(inputName);
	if( nfiles <= 0 )
	{
		std::cerr << "\033[1;31m skimfile ERROR\033[1;mNot found files: '" << inputName << std::endl;
		delete t;
	    	return;
	}
	// if the file exists or not has already been checked before
	t->AddFriend("cuts",cutfile);
	int cutIndex = -1;
	int eventnumber = -1;
	t->SetBranchAddress("cuts.cuts",&cutIndex);
	t->SetBranchAddress("cuts.EventNumber",&eventnumber);

	TFile * foutput = new TFile(outputName,"RECREATE");
	TTree * newT = t->CloneTree(0);
	// Also put the cuts as a new branch to the new tree
	//TBranch * newB =
	newT->Branch("cuts", &cutIndex);
	newT->Branch("EventNumber", &eventnumber);

	// Begin the copy
	std::cout << "\033[1;34mskimfile INFO\033[1;m Total number of events: ";	
	int goodEvents = 0;
	
	int iEntry = 0;
	int nEntries = 0;
	int pEntries = 0;
	if( events.first >= events.second )
	{
		nEntries = t->GetEntries();
		pEntries = nEntries;
	}
	else
	{
		iEntry = events.first;
		nEntries = events.second;
		pEntries = nEntries-iEntry;
	}
	std::cout << pEntries << std::endl; 
      	for(int event=iEntry; event < nEntries; ++event)
	{
		t->GetEntry(event);
		if( event % (pEntries/10) == 0 )
		{
			std::cout << "   + Event:" << event << std::endl;
		}

		if( cutIndex >= cutThreshold ) 
		{
			++goodEvents;
			//newB->Fill();
		  	newT->Fill();
	    	}
      	}

	if (goodEvents == 0)
	{
		std::cout << std::endl;
		std::cout << "\033[1;33mskimfile WARNING\033[1;m" << std::endl;
		std::cout << "--------> Not found any event fulfilling the requirements <--------" << std::endl;
	}
	else
	{
		std::cout << "\033[1;34mskimfile INFO\033[1;m Number of events: " << goodEvents << std::endl;
	}

	foutput->cd();
	newT->Write();
	foutput->Close();

      	delete t;
      	delete foutput;
}


#ifndef __CINT__
void display_usage()
{
	std::cout << "\033[1;37musage:\033[1;m skimfile cutindex [options]" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "    -i inputfile.root  Input file" << std::endl;
	std::cout << "    -o outputfile.root Output file [default: inputfile_skimcuts.root]" << std::endl;
	std::cout << "    -t treenane        TTree name to be copied [default: Tree]" << std::endl;
	std::cout << "    -c cutfile         root file containing the 'cuts' Tree" << std::endl;
	std::cout << "    -e evI,evF         Event init and event final to be processed" << std::endl;
	std::cout << "    -h                 displays this help message and exits " << std::endl;
	std::cout << "" << std::endl;
}

int main( int argc, const char* argv[] )
{
	const char * _cutindex = 0;
	const char * _output   = 0;
	const char * _input    = 0;
	const char * _treename = 0;
	const char * _cutfile  = 0;
	const char * _events   = 0;
	
	// Arguments used
	std::set<int> usedargs;
	//Parsing input options
	if(argc == 1)
	{
		display_usage();
		return -1;
	}
	else
	{
		//Argumet 1 must be a valid input fileName
		for (int i = 1; i < argc; i++) 
		{
			if( strcmp(argv[i],"-o") == 0 )
			{
				_output = argv[i+1];
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
			if( strcmp(argv[i],"-i") == 0 )
			{
				_input = argv[i+1];
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
			if( strcmp(argv[i],"-t") == 0 )
			{
				_treename = argv[i+1];
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
			if( strcmp(argv[i],"-c") == 0 )
			{
				_cutfile = argv[i+1];
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
			if( strcmp(argv[i],"-e") == 0 )
			{
				_events = argv[i+1];
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
			if( strcmp(argv[i],"-h") == 0 ||
					strcmp(argv[i],"--help") == 0 )
			{
				display_usage();
				return 0;
			}
		}
	}
	// Extracting the cut index
        for(int i=1; i < argc; i++)
	{
		if(usedargs.find(i) == usedargs.end())
		{
			_cutindex = argv[i];
			break;
		}
	}

	if( _input == 0 )
	{
		std::cerr << "\033[1;31mskimfile ERROR:\033[1;m The '-i' option is mandatory!"
			<< std::endl;
		display_usage();
		return -1;
	}

	if( _cutindex == 0 )
	{
		std::cerr << "\033[1;31mskimfile ERROR:\033[1;m The 'cutindex' argument is mandatory!"
			<< std::endl;
		display_usage();
		return -1;
	}
	
	if( _cutfile == 0 )
	{
		std::cerr << "\033[1;31mskimfile ERROR:\033[1;m The '-c' option is mandatory!"
			<< std::endl;
		display_usage();
		return -1;
	}

	// reassigning
	std::string inputf(_input);
	std::string cutfile(_cutfile);
	std::string outputf;
	std::string treename;
	std::pair<int,int> events;

	if( _output == 0 )
	{
		outputf = inputf.substr(0,inputf.find(".root"))+"_skimcuts.root";
	}
	else 
	{
		outputf = _output;
	}

	if( _treename == 0 )
	{
		treename = "Tree";
	}
	else 
	{
		treename = _treename;
	}

	if( _events == 0 )
	{
		events = std::pair<int,int>(0,-1);
	}
	else
	{
		std::string eventsstr(_events);
		std::string initS;
		std::string endS;
		initS.assign(eventsstr.begin(),eventsstr.begin()+eventsstr.find(","));
		endS.assign(eventsstr.begin()+eventsstr.find(",")+1,eventsstr.end());
		std::stringstream iSS(initS);
		std::stringstream eSS(endS);
		int eventInit;
		int eventLast;
		if( (iSS >> eventInit).fail() )
		{
			std::cerr << "Invalid Initial event '"  << iSS << std::endl;
			return -1;
		}
		if( (eSS >> eventLast).fail() )
		{
			std::cerr << "Invalid last event '"  << eSS << std::endl;
			return -1;
		}
		if( eventInit >= eventLast )
		{
			std::cerr << "Invalid order in the init,last event of '-e' option '" << std::endl;
			return -1;
		}

		events = std::pair<int,int>(eventInit,eventLast);
	}

	int cut = -1;
	std::stringstream ss(_cutindex);
	ss >> cut;

	// Checking the existence of cut file
	TFile * dum = new TFile(cutfile.c_str());
	if( dum->IsZombie() )
	{
		std::cerr << "\033[1;31mskimfile ERROR:\033[1;m Not found the 'cuts' file '" 
			<< cutfile << "'. Check the path..." << std::endl;
		return -1;
	}

	std::cout<< "\033[1;34mskimfile INFO\033[1;m Copying " 
		<< inputf << " to " << outputf << ", using events satisfying a cut index >= " << cut << std::endl;
	CopyFiles(inputf.c_str(),outputf.c_str(),treename.c_str(),cutfile.c_str(),cut,events);

}
#endif

