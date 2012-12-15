// buildbalcklist.cpp
// Create a file containing the entry number of the run-eventID whose have not
// to be processed

#include<iostream>
#include<sstream>
#include<fstream>
#include<string>
#include<vector>
#include<cstdio>

#include "TChain.h"

void display_usage()
{
	std::cout << "\033[37musage:\033[m buildblacklist Run:EventID,Run:EventID,... DATANAME.dn" 
		<< std::endl;
	std::cout << std::endl;
	std::cout << "The DATANAME.dn file contain a list of root files where to apply the vetoed" <<
		" Run:EventID list" << std::endl;
	std::cout << "" << std::endl;
}

std::vector<std::string> extractdatafiles(const char * filename )
{
	std::ifstream inputf( filename );
	// TreeType
	std::string s_treeType;
	getline(inputf,s_treeType);
	
	std::vector<std::string> datafiles;
	std::string line;

	// TString construction
	while( ! inputf.eof() )
	{	
		getline(inputf,line);
		if( line.find("XS:") != line.npos || line.find("NEvents:") != line.npos 
				|| (line.find("RUNPERIOD:") != line.npos ) )
		{
			continue;
		}
		else if( line != "" )
		{
			datafiles.push_back( line );
		}
	}
	inputf.close();
	
	return datafiles;
}

int main(int argc, char *argv[])
{
	//Parsing input options
	if(argc < 3)
	{
		display_usage();
		return -1;
	}

	const std::string rawvetolist(argv[1]);
	const std::string datanamefile(argv[2]);

	// Parsing the veto events
	std::vector<std::pair<long int,long int> > vetolist;

	char * pch;
	// Note that strtok changes the original
	// string, so I must de-const..
	char *temp = new char[rawvetolist.size()];
	temp = const_cast<char*>(rawvetolist.c_str());
	pch = strtok(temp," ,");
	while( pch != 0)
	{
		std::istringstream insidecomma(pch);
		std::string tempstr;
		std::vector<long int> tmpvectstr;
		while(std::getline(insidecomma,tempstr,':'))
		{
			std::istringstream tmpi(tempstr);
			long int iProv;
			tmpi >> iProv;
			tmpvectstr.push_back(iProv);
		}
		if( tmpvectstr.size() != 2 )
		{
			std::cerr << "\033[31mbuildblacklist ERROR\033[m"
				<< " Not able to parse option, recall the syntaxis:"
				<< " '-c MUON:config_file,ELEC:config_file'" 
				<< std::endl;
			return -1;
		}
		vetolist.push_back(std::pair<long int,long int>(tmpvectstr[0],tmpvectstr[1]));
		pch = strtok(0, " ,");
	}

	if(vetolist.size() == 0)
	{
		std::cerr << "\033[31mrunanalysis ERROR\033[m"
			<< " Not able to parse '-c' option, recall the syntaxis:"
			<< " '-c MUON:config_file,ELEC:config_file'" 
			<< std::endl;
		display_usage();
		return -1;
	}
	
	// Datafiles to check the events
	const std::vector<std::string> datafiles = extractdatafiles(datanamefile.c_str());
	
	// Working with Latinos/MiniTrees
	TChain * ch = new TChain("Tree");
	for(std::vector<std::string>::const_iterator it = datafiles.begin(); it != datafiles.end(); ++it)
	{
		ch->Add(it->c_str());
	}
	
	// Put the run,event,entry info in a file
	std::ofstream blacklistf("blacklist.evt");
	// Build a index to extract the entry corresponding the given run,event
	// XXX CAVEAT: This introduce a dependence of the Tree structure!! XXX
	ch->BuildIndex("T_Event_RunNumber","T_Event_EventNumber");

	for(std::vector<std::pair<long int,long int> >::const_iterator it = vetolist.begin();
			it != vetolist.end(); ++it)
	{
		const long int run   = it->first;
		const long int evtid = it->second;

		const long int entry = ch->GetEntryNumberWithIndex(run,evtid);
		if( entry == -1 )
		{
			// Not in these files
			continue;
		}
		std::cout << "\033[33mTree Entry vetoed: \033[m" << entry 
			<< " (Run: " << run << " EvtID: " << evtid << ")" 
			<< std::endl;
		blacklistf << run << ":" << evtid << "@" << entry << std::endl;
	}

	blacklistf.close();
	// REmoving if there aren't values
	if( vetolist.size() == 0 )
	{
		remove("blacklist.evt");
	}

	if( ch != 0 )
	{
		delete ch;
		ch = 0;
	}

	return 0;
}
