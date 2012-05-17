/*
   pdfsystematic.cpp

   copies the contents of a File, but only
   storing the events which pass the cut N

Compilation: gcc `root-config --libs` -I`root-config --incdir` -o pdfsystematic pdfsystematic.cc

Arguments: Name of the cut
Options:   -o Name of the output root file
           -i Name of the input root file
	   


   J. Duarte Campderros, 12-12-2011 (Based in pytnp/procfiles/CowboysNTuple.cpp)

   duarte@ifca.unican.es
*/
#ifndef __CINT__

#include<string>
#include<iostream>
#include<iomanip>
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

void pdfUncertainties(const int & totalEvents, const int & selectEvents,
		const std::vector<float> & total, const std::vector<float> & total2,
		const std::vector<float> & selected, const std::vector<float> & selected2)
{
	float originalAcceptance = float(selectEvents)/float(totalEvents);
	
	float acc_central = selected[0]/total[0];
	float acc2_central = selected2[0]/total2[0];


	std::cout << ">>>>> PDF UNCERTAINTIES ON ACCEPTANCE >>>>>>" << std::endl;
	double waverage = total[0]/totalEvents;
	
	std::cout << "\tEstimate for central PDF member acceptance: [" << acc_central*100.0 << " +- " << 
            100.0*sqrt((acc2_central/waverage-acc_central*acc_central)/totalEvents)
	    << "] %" << std::endl;
	
	double xi = acc_central-originalAcceptance;
	double deltaxi = (acc2_central-(originalAcceptance+2.0*xi+xi*xi))/totalEvents;
	if(deltaxi>0)
	{
		deltaxi = sqrt(deltaxi);
	}
	std::cout << "\ti.e. [" << std::setprecision(4) << 100.0*xi/originalAcceptance
		<< " +- " << std::setprecision(4) << 100*deltaxi/originalAcceptance 
		<< "] % relative variation with respect to the original PDF" << std::endl;

	
	unsigned int npairs = (total.size()-1)/2;
	double wplus = 0.;
	double wminus = 0.;
	// Master equations
	for(unsigned int j=0; j<npairs; ++j) 
	{
		float wa = 0.;
		if(total[2*j+1]>0)
		{
			wa = (selected[2*j+1]/total[2*j+1])/acc_central-1.0;
		}
		float wb = 0.;
		if(total[2*j+2]>0)
		{
			wb = (selected[2*j+2]/total[2*j+2])/acc_central-1.0;
		}

		if(wa>wb) 
		{
			if(wa<0.)
			{
				wa = 0.;
			}
			if(wb>0.)
			{
				wb = 0.;
			}
			wplus += wa*wa;
			wminus += wb*wb;
		} 
		else 
		{
			if(wb<0.) 
			{
				wb = 0.;
			}
			if(wa>0.)
			{
				wa = 0.;
			}
			wplus += wb*wb;
			wminus += wa*wa;
		}
	}
	
	if(wplus>0)
	{
		wplus = sqrt(wplus);
	}
	
	if(wminus>0)
	{
		wminus = sqrt(wminus);
	}
	std::cout << "\tRelative uncertainty with respect to central member: +" 
		<< std::setprecision(4) << 100.*wplus << " / -" << std::setprecision(4) 
		<< 100.*wminus << " [%]" << std::endl;
}

void processEvents(const char *inputName, const char * treename, const char * cutfile, 
		const int & cutThreshold =13) 
{
      	TChain * t = new TChain(treename);
	// Extracting the list of files (in the case is a list)
	// must be comma separated
	std::string iN = inputName;
	char *pch;
	char *temp = new char[iN.size()];
	temp = const_cast<char*>(iN.c_str());
	pch = strtok(temp," ,");
	std::vector<const char*> fileNames;
	while(pch != 0)
	{
		fileNames.push_back(pch);
		pch = strtok(0," ,");
	}
	// Populating the TChain, using different arguments depending if is a 
	// list of files or not
	int nfiles = 0;
	if(fileNames.size() == 0)
	{
		nfiles = t->Add(inputName);
	}
	else
	{
		for(unsigned int j = 0; j < fileNames.size(); ++j)
		{
			nfiles += t->Add(fileNames[j]);
		}
	}

	if( nfiles <= 0 )
	{
		std::cerr << "\033[1;31m pdfsystematic ERROR\033[1;mNot found files: '" << inputName << std::endl;
		delete t;
	    	return;
	}

	int evtnumber = -1;
	int runnumber = -1;
	t->SetBranchAddress("T_Event_EventNumber",&evtnumber);
	t->SetBranchAddress("T_Event_RunNumber",&runnumber);
	// Pdf- Weights
	std::vector<float> *pdfweightCTEQ66 = 0;
	t->SetBranchAddress("T_Event_pdfWeightCTEQ66",&pdfweightCTEQ66);

	// Just assuming the entry numbers are okey. Check if they are okey
	t->AddFriend("cuts",cutfile);
	int cutIndex = -1;
	int evtnumber_f = -1;
	int runnumber_f = -1;
	t->SetBranchAddress("cuts",&cutIndex);
	t->SetBranchAddress("EventNumber",&evtnumber_f);
	t->SetBranchAddress("RunNumber",&runnumber_f);

	int initialevents= 0;
	std::vector<float> weightedInitial;
	std::vector<float> weighted2Initial;
	int selectevents = 0;
	std::vector<float> weightedevents;
	std::vector<float> weighted2events;
	std::vector<float> weightsum;
	// Initialize (in this way to avoid the if inside the event loop)
	t->GetEntry(0);
	for(unsigned int i = 0; i < pdfweightCTEQ66->size(); ++i)
	{
		weightedInitial.push_back(0.0);
		weighted2Initial.push_back(0.0);
		weightedevents.push_back(0.0);
		weighted2events.push_back(0.0);
		weightsum.push_back(0.0);
	}

	int nEntries = t->GetEntries();
      	for(int event=0; event < nEntries; ++event)
	{
		t->GetEntry(event);
		// FIXME: To be deleted
		if( evtnumber != evtnumber_f || runnumber != runnumber_f )
		{
			std::cout << " CAGADA! " << std::endl;
			exit(1);
		}
		if( event % (nEntries/10) == 0 )
		{
			std::cout << "  + Event:" << event << std::endl;
		}
		// Initial events
		if( cutIndex > 0 )
		{
			++initialevents;
			for(unsigned int i = 0; i < pdfweightCTEQ66->size(); ++i)
			{
				weightedInitial[i] += (*pdfweightCTEQ66)[i];
				weighted2Initial[i] += weightedInitial[i]*weightedInitial[i];
			}
		}

		// Selected events
		if( cutIndex >= cutThreshold ) 
		{
			++selectevents;
			for(unsigned int i = 0; i < pdfweightCTEQ66->size(); ++i)
			{
				weightedevents[i] += (*pdfweightCTEQ66)[i];
				weighted2events[i] += weightedevents[i]*weightedevents[i];
			}
	    	}
      	}

	if(selectevents == 0)
	{
		std::cout << "\033[1;33mpdfsystematic WARNING\033[1;m" << std::endl;
		std::cout << "--------> Not found any event fulfilling the requirements <--------" << std::endl;
	}

      	delete t;
	delete pdfweightCTEQ66;

	pdfUncertainties(initialevents,selectevents,weightedInitial,weighted2Initial,
			weightedevents,weighted2events);
}


#ifndef __CINT__
void display_usage()
{
	std::cout << "\033[1;37musage:\033[1;m pdfsystematic cutindex [options]" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "    -i inputfile.root  Input file" << std::endl;
	std::cout << "    -t treenane        TTree name to be copied [default: Tree]" << std::endl;
	std::cout << "    -c cutfile         root file containing the 'cuts' Tree" << std::endl;
	std::cout << "    -e evI,evF         Event init and event final to be processed" << std::endl;
	std::cout << "    -h                 displays this help message and exits " << std::endl;
	std::cout << "" << std::endl;
}

int main( int argc, const char* argv[] )
{
	const char * _cutindex = 0;
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
		std::cerr << "\033[1;31mpdfsystematic ERROR:\033[1;m The '-i' option is mandatory!"
			<< std::endl;
		display_usage();
		return -1;
	}

	if( _cutindex == 0 )
	{
		std::cerr << "\033[1;31mpdfsystematic ERROR:\033[1;m The 'cutindex' argument is mandatory!"
			<< std::endl;
		display_usage();
		return -1;
	}
	
	if( _cutfile == 0 )
	{
		std::cerr << "\033[1;31mpdfsystematic ERROR:\033[1;m The '-c' option is mandatory!"
			<< std::endl;
		display_usage();
		return -1;
	}

	// reassigning
	std::string inputf(_input);
	std::string cutfile(_cutfile);
	std::string treename;
	std::pair<int,int> events;
	
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
		std::cerr << "\033[1;31mpdfsystematic ERROR:\033[1;m Not found the 'cuts' file '" 
			<< cutfile << "'. Check the path..." << std::endl;
		return -1;
	}

	std::cout<< "\033[1;34mpdfsystematic INFO\033[1;m Evaluating the PDF systematics for the WZ Monte Carlo sample " 
		<< std::endl;
	processEvents(inputf.c_str(),treename.c_str(),cutfile.c_str(),cut);

}
#endif

