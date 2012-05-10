///////////////////////////////////////////////////////////////////////
//
//    FILE: RunProof.C
// AUTHORS: I. Gonzalez Caballero, A.Y. Rodriguez Marrero
//    DATE: 2010
//
// CONTENT: Main macro to run over MiniTrees or TESCO Trees using PROOF
//          in PROOF-Lite, PROOF-Cluster or Sequential mode
///////////////////////////////////////////////////////////////////////

#ifndef __CINT__

#include<iostream>
#include<sstream>
#include<fstream>
#include<cstdlib>
#include<string>
#include<vector>
#include<queue>
#include<map>
#include<set>

#endif

#include "DatasetManager.h"
#include "InputParameters.h"
#include "TreeTypes.h"
#include "LeptonTypes.h"

#include "AnalysisBuilder.h"
//#include "AnalysisVH.h"
#include "AnalysisBase.h"


// ROOT
#include "TString.h"
#include "TChain.h"
#ifdef TIMERS
#include "TStopwatch.h"
#endif


// Global variables
double      xsection = 0;
int         evtsample= 0;
std::string RUNPERIOD;

// Storing the datafiles
void AddDataFiles(const std::vector<TString> & files, 
		std::vector<TString> & datafiles)
{
	datafiles.reserve( datafiles.size() + files.size() );
	datafiles.insert( datafiles.end(), files.begin(), files.end() );
}

void PrintDataFiles(const std::vector<TString> & dataFiles)
{
	for(unsigned int i = 0; i < dataFiles.size(); ++i)
	{
		std::cout << "* " << dataFiles[i] << std::endl;
	}
}


// Overloaded function to extract the file names from a previous stored file
// It will catch the files inside a subset: if datanamefile != is the name
std::pair<treeTypes,std::vector<TString> > extractdatafiles(const char * dataName, const char * datanamefile = 0 )
{
	TString dataNameprov(dataName);
	const char * filename = 0;
	if( datanamefile == 0 )
	{
		if (dataNameprov.Contains("WH")) 
		{
			dataNameprov.Replace(0,2, "WHToWW2L");
		}
		dataNameprov += "_datanames.dn";
		filename = dataNameprov.Data();
	}
	else
	{
		filename = datanamefile; 
	}

	std::ifstream inputf( filename );
	if( ! inputf.is_open() )
	{
		// Call the extractdatafiles overloaded to create the filename// DEPRECATED
		std::cout << "\033[31mextractdatafiles DEPRECATED\033[m function. Launch "
			<< "'datamanagercreator' instead. Exiting" << std::endl;
		//return extractdatafiles(dataName);
	}
	
	// TreeType
	std::string s_treeType;
	getline(inputf,s_treeType);
	
	int i_treeType = -1;
	std::stringstream ss(s_treeType);
	ss >> i_treeType;
	treeTypes en_treeType = (treeTypes)i_treeType;

	std::vector<TString> datafiles;
	std::string line;

	// TString construction
	while( ! inputf.eof() )
	{	
		getline(inputf,line);
		if( line.find("XS:") != line.npos )
		{
			std::stringstream sXS(line.substr(3));
			sXS >> xsection; 
		}
		else if(line.find("NEvents:") != line.npos)
		{
			std::stringstream sNE(line.substr(8));
			sNE >> evtsample;
		}
		else if(line.find("RUNPERIOD:") != line.npos )
		{
			std::stringstream sRP(line.substr(10));
			sRP >> RUNPERIOD;
		}
				
		else if( line != "" )
		{
			datafiles.push_back( TString(line) );
		}
	}
	inputf.close();


	return std::pair<treeTypes,std::vector<TString> >(en_treeType,datafiles);
}


// From a config file, extract the parameters needed
// FIXME: Necesito incluir una nueva variable fija: tipo de lepton
// lo que devuelva sera un par<Lepton,inputPar>
InputParameters * setparameters(const std::vector<TString> & datafiles, const TString & dataName,
		const char * cfgfile, const char * nameIPinstance="Set Of Parameters")
{
	// Introduce the analysis parameters
	//TreeType treeType = kMiniTrees;
	
	// Parsing the config file
	InputParameters * ip = InputParameters::parser(cfgfile,nameIPinstance);

	//TString MySelector = ip->TheNamedString("MySelector");  // TO BE DEPRECATED

	///////////////////////////////
	// OUTPUT FILE NAME
	// Specify the name of the file where you want your histograms to be saved
	TString outputFile;
	// Mkdir Results if necessary
	system("mkdir -p Results");
	outputFile.Form("Results/%s.root", dataName.Data()); 
	
	TString myTreeName = ip->TheNamedString("NameTree");
 	//
	// + Find the total number of events after skimming and send it to the input parameters
	TChain* chain = new TChain(myTreeName.Data(),myTreeName.Data());
	for(unsigned int i = 0; i < datafiles.size(); i++) 
	{
		chain->Add(datafiles[i]);
	}
	ip->SetNamedInt("NEventsSkim", chain->GetEntries());
	delete chain;
	// + Cross section after skimming
	if((!dataName.Contains("Data"))  && (!dataName.Contains("LocalWH"))) 
	{
		ip->SetNamedDouble("CrossSection", xsection);
		ip->SetNamedInt("NEventsSample", evtsample);
	}
	
	// + Run period
	ip->SetNamedString("RunPeriod",RUNPERIOD);

	// + Data Name
	ip->SetNamedString("DataName", dataName.Data());
	
	///////////////////////////////
	// NAME OF ANALYSIS CLASS. 
	// If 0 the default name schema will be used, i.e. depending on the value
	// of gPAFOptions->treeType: MyAnalysisTESCO or MyAnalsyisMiniTrees
	//
	//TString myAnalysis = MySelector.Data();

	//ip->SetNamedString(std::string("MyAnalysis"),std::string(myAnalysis));  // TO BE DEPRECATED

	// All the datafiles
	for(unsigned int i = 0; i < datafiles.size(); ++i)
	{
		std::stringstream is;
		is << i;
		std::string varfilename("datafilenames_");
		varfilename += is.str(); 
		ip->SetNamedString( varfilename, datafiles[i].Data() );
	}

	return ip;
}

void display_usage()
{
	std::cout << "\033[37musage:\033[m runanalysis dataname [options]" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "    -a <WZ|WH>               Analysis to be done" << std::endl;
	std::cout << "    -c LEPTON:config1[,LEPTON2:config1,...] " << std::endl;
	std::cout << "                             configurations file " << std::endl;
	std::cout << "    -d dataname.dn           filename containing the files for the 'dataname'" << std::endl;
	std::cout << "    -l <mmm|eee|mme|eem>     Final state signature (mmm per default)" << std::endl;
	std::cout << "    -F N,T                   Fake mode activated, N=number of leptons," << 
		"T=number of tight leptons" << std::endl;
	std::cout << "    -o output.root           output root file " << std::endl;
	std::cout << "    -h                       displays this help message and exits " << std::endl;
	std::cout << "" << std::endl;
	std::cout << "List of known dataname:" << std::endl;
	std::cout << "    Higgs:             WH# (#: Higgs Mass hypothesis)" << std::endl;
	std::cout << "    Z + Jets Madgraph: ZJets_Madgraph" << std::endl;
	std::cout << "    Z + Jets Powheg:   DYee DYmumu Dytautau Zee_Powheg Zmumu_Powheg Ztautau_Powheg" << std::endl;
	std::cout << "    Zbb + Jets:        Zbb (NOT IMPLEMENTED)" << std::endl;
	std::cout << "    Other backgrounds: WZ ZZ WW TTbar_Madgraph WJetas_Madgraph TW TbarW Fakes" << std::endl;
}


int main(int argc, char *argv[])
{
	const char * dataName       = 0; // = "WH160";
	std::map<std::string,std::string> cfgfilemap;
	const char * outputfilechar = 0;
	const char * datanamefile   = 0;
	const char * fsSignature    = "mmm";
	const char * antype    = "WH";
	std::vector<int> * fakeable = 0;

	bool getOF = false;

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
		//dataName = argv[1];
		for(int i = 1; i < argc; i++) 
		{
			if( strcmp(argv[i],"-h") == 0 )
			{
				display_usage();
				return 0;
			}
			if( strcmp(argv[i],"-c") == 0 )
			{
				// Extracting if there are more than one
				std::string rawconfigs = argv[i+1];
				char * pch;
				// Note that strtok changes the original
				// string, so I must de-const..
				char *temp = new char[rawconfigs.size()];
				temp = const_cast<char*>(rawconfigs.c_str());
				pch = strtok(temp," ,");
				while( pch != 0)
				{
					std::istringstream insidecomma(pch);
					std::string tempstr;
					std::vector<std::string> tmpvectstr;
					while(std::getline(insidecomma,tempstr,':'))
					{
						tmpvectstr.push_back(tempstr);
					}
					if( tmpvectstr.size() != 2 )
					{
						std::cerr << "\033[31mrunanalysis ERROR\033[m"
							<< " Not able to parse '-c' option, recall the syntaxis:"
							<< " '-c MUON:config_file,ELEC:config_file'" 
							<< std::endl;
						return -1;
					}
					cfgfilemap[tmpvectstr[0]] = tmpvectstr[1];
					pch = strtok(0, " ,");
				}
				// Extract the null
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
			
			if( strcmp(argv[i],"-o") == 0 )
			{
				outputfilechar = argv[i+1];
				getOF = true;
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
			if( strcmp(argv[i],"-d") == 0 )
			{
				datanamefile = argv[i+1];
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
			if( strcmp(argv[i],"-l") == 0 )
			{
				fsSignature = argv[i+1];
				if( strcmp(fsSignature,"mmm") != 0 
						&& strcmp(fsSignature,"eee") != 0 
						&& strcmp(fsSignature,"mme") != 0
						&& strcmp(fsSignature,"eem") != 0 )
				{
					std::cerr << "\033[31mrunanalysis ERROR:\033[m Not implemented '" << fsSignature
						<< "' in '-l' option. Valid arguments are: mmm eee mme eem" << std::endl;
					return -1;
				}
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
			if( strcmp(argv[i],"-F") == 0 )
			{
				// Extracting the number of leptons 
				std::string rawconfigs = argv[i+1];
				char * pch;
				// Note that strtok changes the original
				// string, so I must de-const..
				char *temp = new char[rawconfigs.size()];
				temp = const_cast<char*>(rawconfigs.c_str());
				pch = strtok(temp," ,");
				fakeable = new std::vector<int>;
				while( pch != 0)
				{
					std::stringstream insidecomma(pch);
					int iProv;
					insidecomma >> iProv;
					fakeable->push_back(iProv);
					pch = strtok(0, " ,");
				}
				// Checks
				if( fakeable->size() != 2 )
				{
					std::cerr << "\033[31mrunanalysis ERROR\033[m"
						<< " Not able to parse '-F' option, recall the" 
						<< " syntaxis: '-F N,T' where N=number of total" 
						<< " leptons and T=number of tight leptons" 
						<< std::endl;
					return -1;
				}
				// Ntotal must be >= Ntight
				if( fakeable->at(0) < fakeable->at(1) )
				{
					std::cerr << "\033[31mrunanalysis ERROR\033[m"
						<< " Inconsistency in '-F' option. Recall " 
						<< " syntaxis: '-F N,T' where N=number of total" 
						<< " leptons and T=number of tight leptons" 
						<< std::endl;
					return -1;
				}
				// Extract the null
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
			if( strcmp(argv[i],"-a") == 0 )
			{
				antype = argv[i+1];
				if( strcmp(antype,"WZ") != 0 
						&& strcmp(antype,"WH") != 0 )
				{
					std::cerr << "\033[31mrunanalysis ERROR:\033[m Not implemented '" << antype
						<< "' in '-a' option. Valid arguments are: WH WZ" << std::endl;
					return -1;
				}
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
		}
	}
	// Extracting the data name
        for(int i=1; i < argc; i++)
	{
		if(usedargs.find(i) == usedargs.end())
		{
			dataName = argv[i];
			break;
		}
	}
	if( dataName == 0 )
	{
		std::cerr << "\033[31mrunanalysis ERROR:\033[m The 'dataname' argument is mandatory!"
			<< std::endl;
		display_usage();
		return -1;
	}

	// Case FAKE: some checks
	if( strcmp(dataName,"Fakes") == 0 )
	{
		// The -F option must be active
		if( fakeable == 0 )
		{
			std::cerr << "\033[31mrunanalysis ERROR\033[m"
				<< " Inconsistency in the executable options: the 'Fakes' dataname" 
				<< " needs the '-F' option" << std::endl;
			return -1;
		}
	}
/*	else{  FIXME: PROVISIONAL
		if( fakeable != 0)
		{
			std::cerr << "\033[31mrunanalysis ERROR\033[m"
				<< " Inconsistency in the executable options: the '-F' option is" 
				<< " used only by the 'Fakes' dataName. Can not be used with'"
				<< dataName << "'" << std::endl;
			return -1;
		}
	}*/


#ifdef TIMERS
	TStopwatch timer;
	timer.Start();
	double t1, t2, t3, t4, t5, t6;
#endif
	
#ifdef TIMERS
	//T1
	t1 = timer.RealTime();
	timer.Start();
#endif
	// Extract the datafiles from the file which it contains them
	std::pair<treeTypes,std::vector<TString> > dum = extractdatafiles(dataName, datanamefile );
	std::vector<TString> datafiles = dum.second;
	treeTypes dataType  = dum.first;
	
#ifdef TIMERS
	//T2
	t2 = timer.RealTime();
	timer.Start();
#endif
	//-- Configuration of the analysis
	// Sanity check
	if( cfgfilemap.size() > 2 )
	{
		std::cerr << "\033[31mrunanalysis ERROR\033[m It has been"
		<< " introduced 2 configuration files which it has no sense:"
		<< " There are 2 different stable lepton flavours!"
		<< std::endl;
		exit(-1);
	}

	// Initialize the analysis specific parameters using the config file
	std::map<LeptonTypes,InputParameters*> ipmap;
	for(std::map<std::string,std::string>::iterator cfgfile = cfgfilemap.begin(); 
			cfgfile != cfgfilemap.end(); ++cfgfile)
	{
		// finding the pairs type of lepton -> analysis config file
		LeptonTypes lepton;
		if( cfgfile->first == "MUON" || cfgfile->first == "MU" ||
				cfgfile->first == "muon" || cfgfile->first == "mu" )
		{
			lepton = MUON;
		}
		else if( cfgfile->first == "ELECTRON" || cfgfile->first == "ELEC" ||
				cfgfile->first == "elec" || cfgfile->first == "elec" )
		{
			lepton = ELECTRON;
		}
		else
		{
			std::cerr << "\033[31mrunanalysis ERROR\033[m Not recognized"
				<< " '" << cfgfile->first << "' as a valid lepton to associate"
				<< " a config file (-c option). Valid names are:"
				<< "\nMUON MU muon mu ELECTRON ELE electron ele"
				<< std::endl;
			return -1;
		}
		ipmap[lepton] = setparameters(datafiles,TString(dataName),cfgfile->second.c_str());
	}
	TChain * tchaindataset = 0;
	// Data: FIXME: Extract this info from a centralized way (TreeManager?)
	//              TreeTypes mejor
	if(dataType == MiniTrees || dataType == Latinos)
	{
		tchaindataset = new TChain("Tree");
	}
	else if(dataType == TESCO)
	{
		tchaindataset = new TChain("analyze/Analysis");
	}
	else
	{
		std::cerr << " ERROR: ROOT tree file contains an unrecognized Tree" << std::endl;
		// Freeing memory as the future owner of InputParameters it is not initialized even
		for(std::map<LeptonTypes,InputParameters*>::iterator ip = ipmap.begin(); ip != ipmap.end(); 
				++ip)
		{
			if( ip->second != 0)
			{
				delete ip->second;
				ip->second = 0;
			}
		}
		return -1;
	}
	for(std::vector<TString>::iterator it = datafiles.begin(); it != datafiles.end(); ++it)
	{
		tchaindataset->Add(*it);
	}


	// Extract entries now before the InputParameters is going to 
	// be used by the Builder (the Analysis will be the owner and
        // there is going to delete some of them

	// Entries
	int nEvents = -1;
	// Take whatever, they contain the same values
	ipmap.begin()->second->TheNamedInt("nEvents",nEvents);
	int firstEvent = -1 ;
	ipmap.begin()->second->TheNamedInt("firstEvent",firstEvent);
	// Processing
	if( nEvents < 0 )
	{
		nEvents = TChain::kBigNumber;
	}
	if( firstEvent < 0 )
	{
		std::cout << "WARNING: firstEvent is not defined in your configuration, "
			<< "using 0 as default" << std::endl;
		firstEvent = 0; 
	}

#ifdef TIMERS
	//T3
	t3 = timer.RealTime();
	timer.Start();
#endif
	// Creating Analysis
	AnalysisBase * analysis = AnalysisBuilder::Build( antype, dataType, fsSignature,
			ipmap, fakeable ); 

	// 
	if( fakeable != 0 )
	{
		delete fakeable;
		fakeable = 0;
	}

#ifdef TIMERS
	//T4
	t4 = timer.RealTime();
	timer.Start();
#endif
	// The mother of everything...
	tchaindataset->Process(analysis,0,nEvents,firstEvent);
	
#ifdef TIMERS
	//T5
	t5 = timer.RealTime();
	timer.Start();
#endif
	// Storing the output
	analysis->SaveOutput( outputfilechar );

	if( analysis != 0 )
	{
		delete analysis;
		analysis = 0;
	}
	if( tchaindataset != 0 )
	{
		delete tchaindataset;
		tchaindataset = 0;
	}
#ifdef TIMERS
  t6 = timer.RealTime();

  std::cout << "Tiempos de ejecucion:" << std::endl;
  std::cout << t1 << ", " << t2 << ", " << t3 << ", " << t4 << ", " << t5 
	  << ", " << t6 <<std::endl;
#endif

}
