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

#endif

#include "DatasetManager.h"
#include "InputParameters.h"
#include "TreeTypes.h"

#include "AnalysisBuilder.h"
#include "AnalysisVH.h"


// ROOT
#include "TString.h"
#include "TChain.h"
#ifdef TIMERS
#include "TStopwatch.h"
#endif


// Global variables
double xsection = 0;
int    evtsample= 0;

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

// Giving a dataset extract all the files and stores them in a file
const std::vector<TString> extractdatafiles(TString dataName = "HW160" )
{
	bool storeXSE = false;

	// 1) Load the files
	DatasetManager * dm = 0;

	std::vector<TString> datafiles;
	// 2) Asign the files 
	if(dataName.Contains("LocalWH")) 
	{
		datafiles.push_back( "/hadoop/PrivateProduction/WH_2l_42X/Tree_WH_2l_42X.root" );
	}
	else if(dataName.Contains("Data")) 
	{
		std::vector<TString> data1= 
			DatasetManager::GetRealDataFiles("../LatinosSkims/Data7TeVRun2011A_newJEC_Reload", 
					"Tree_DoubleMuMay10_210.5");
		AddDataFiles(data1,datafiles);
                
		std::vector<TString> data2= 
             		  DatasetManager::GetRealDataFiles("../LatinosSkims/Data7TeVRun2011A_newJEC_Reload", 
             				       "Tree_DoubleMuV4_927.9");
		AddDataFiles(data2,datafiles);
                
		std::vector<TString> data3= 
             		  DatasetManager::GetRealDataFiles("../LatinosSkims/Data7TeVRun2011A_newJEC_Reload", 
             				  "Tree_DoubleMuAug5_334.4");
		AddDataFiles(data3,datafiles);
             	
		std::vector<TString> data4= 
             		  DatasetManager::GetRealDataFiles("../LatinosSkims/Data7TeVRun2011A_newJEC_Reload", 
				  "Tree_DoubleMuV6_662.9");
		AddDataFiles(data4,datafiles);
		
		if(datafiles.size() == 0) 
		{
			std::cerr << "ERROR: Could not find dataset " 
				<< dataName << " with DatasetManager!!!" << std::endl;
			std::cerr << "       Exiting!" << std::endl;      
			exit(-1);
		}
		
		PrintDataFiles(datafiles);
	}
	else 
	{
		storeXSE=true;

		//TString folder("Summer11");
		//TString skim("Skim2LPt1010");
		TString folder("Summer11 Latinos");
		TString skim("/");

		if (dataName.Contains("WH")) 
		{
			//folder = "Summer11"; MINITREES
			//skim = "HWW";        MINITREES
			folder = "HWW Summer11 Latinos";
			skim = "/";
			dataName.Replace(0,2, "WHToWW2L");
		}
		
		//Use DatasetManager
		dm = new DatasetManager(folder, skim);
		
		dm->LoadDataset(dataName);  // Load information about a given dataset
		AddDataFiles(dm->GetFiles(),datafiles); //Find files

		xsection = dm->GetCrossSection();
		evtsample= dm->GetEventsInTheSample();
		
#ifdef DEBUG
		std::cout << std::endl;
		std::cout << "      x-section = " << dm->GetCrossSection()      << std::endl;
		//std::cout << "     luminosity = " << G_Event_Lumi               << std::endl; --> NO DEFINIDA
		std::cout << "        nevents = " << dm->GetEventsInTheSample() << std::endl;
		std::cout << " base file name = " << dm->GetBaseFileName()      << std::endl;
		std::cout << std::endl;
		dm->Dump();
#endif
		
		if(datafiles.size() == 0) 
		{
			std::cerr << "ERROR: Could not find dataset " 
				<< dataName << " with DatasetManager for folder " 
				<< folder << "!!!" << std::endl;
			std::cerr << "       Exiting!" << std::endl;      
			exit(-1);
		}

		// Freeing memory
		delete dm; 
		dm = 0;
	}

	// Persistency
	std::string filename( dataName+"_datanames.dn" );
	std::ofstream of( filename.c_str() );
	if( ! of.is_open() )
	{
		std::cerr << "ERROR: Imposible to open the file '" << filename 
			<< "' to store file names information" << std::endl;
		exit(-1);
	}
	// TreeType
	treeTypes treeType;
	if( datafiles[0].Contains("TESCO") )
	{
		treeType = TESCO;
	}
	else 
	{
		treeType = MiniTrees;
	}

	of << treeType << std::endl;
	// CrossSection and number of events if proceed
	if( storeXSE )
	{
		of << "XS:" << xsection << std::endl;
		of << "NEvents:" << evtsample << std::endl;
	}

	for(std::vector<TString>::iterator it = datafiles.begin(); it != datafiles.end(); ++it)
	{
		of << it->Data() << std::endl;
	}
	of.close();

	return datafiles;	
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
		// Call the extractdatafiles overloaded to create the filename
		return extractdatafiles(dataName);
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
		else if( line != "" )
		{
			datafiles.push_back( TString(line) );
		}
	}
	inputf.close();


	return std::pair<treeTypes,std::vector<TString> >(en_treeType,datafiles);
}


// From a config file, extract the parameters needed
InputParameters * setparameters(const std::vector<TString> & datafiles, const TString & dataName,
		const char * cfgfile)
{
	// Introduce the analysis parameters
	//TreeType treeType = kMiniTrees;
	
	// Parsing the config file
	InputParameters * ip = InputParameters::parser(cfgfile);

	TString MySelector = ip->TheNamedString("MySelector");

	///////////////////////////////
	// OUTPUT FILE NAME
	// Specify the name of the file where you want your histograms to be saved
	TString outputFile;
	// Mkdir Results if necessary
	system("mkdir -p Results");
	outputFile.Form("Results/%s_%s.root", MySelector.Data(), dataName.Data()); 
	
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

	// + Data Name
	ip->SetNamedString("DataName", dataName.Data());
	
	///////////////////////////////
	// NAME OF ANALYSIS CLASS. 
	// If 0 the default name schema will be used, i.e. depending on the value
	// of gPAFOptions->treeType: MyAnalysisTESCO or MyAnalsyisMiniTrees
	//
	TString myAnalysis = MySelector.Data();

	ip->SetNamedString(std::string("MyAnalysis"),std::string(myAnalysis));

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

void help_usage()
{
	std::cout << "usage: runanalysis dataname [options]" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "    -c configuration file " << std::endl;
	std::cout << "    -d dataname file" << std::endl;
	std::cout << "    -l <mmm|eee>  Final state signature (mmm per default)" << std::endl;
	std::cout << "    -o output root file " << std::endl;
	std::cout << "" << std::endl;
	std::cout << "List of known dataname:" << std::endl;
	std::cout << "    Higgs:             WH# (#: Higgs Mass hypothesis)" << std::endl;
	std::cout << "    Z + Jets Madgraph: ZJets_Madgraph" << std::endl;
	std::cout << "    Z + Jets Powheg:   DYee DYmumu Dytautau Zee_Powheg Zmumu_Powheg Ztautau_Powheg" << std::endl;
	std::cout << "    Zbb + Jets:        Zbb" << std::endl;
	std::cout << "    Other backgrounds: WZ ZZ WW TTbar_Madgraph WJetas_Madgraph TW TbarW" << std::endl;
}


int main(int argc, char *argv[])
{
	const char * dataName       = 0; // = "WH160";
	const char * cfgfile        = "analisiswh_mmm.ip";
	const char * outputfilechar = 0;
	const char * datanamefile   = 0;
	const char * fsSignature    = "mmm";

	bool getOF = false;
	//Parsing input options
	if(argc == 1)
	{
		help_usage();
		return -1;
	}
	else if( argc == 2)
	{
		dataName = argv[1];
	}
	else
	{
		//Argumet 1 must be a valid input fileName
		dataName = argv[1];
		for(int i = 2; i < argc; i++) 
		{
			if( strcmp(argv[i],"-c") == 0 )
			{
				cfgfile = argv[i+1];
			}
			
			if( strcmp(argv[i],"-o") == 0 )
			{
				outputfilechar = argv[i+1];
				getOF = true;
			}
			if( strcmp(argv[i],"-d") == 0 )
			{
				datanamefile = argv[i+1];
			}
			if( strcmp(argv[i],"-l") == 0 )
			{
				fsSignature = argv[i+1];
				if( strcmp(fsSignature,"mmm") != 0 &&
						strcmp(fsSignature,"eee") != 0 )
				{
					std::cerr << "runanalysis ERROR: Not implemented '" << fsSignature
						<< "' in '-l' option. Valid arguments are: mmm eee" << std::endl;
					return -1;
				}
			}
		}
	}

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
	// Extract the datafiles from the file created in the "local" path
	std::pair<treeTypes,std::vector<TString> > dum = extractdatafiles(dataName, datanamefile );
	std::vector<TString> datafiles = dum.second;
	treeTypes dataType  = dum.first;
	
#ifdef TIMERS
	//T2
	t2 = timer.RealTime();
	timer.Start();
#endif
	// Initialize the analysis specific parameters using a config file
	InputParameters * ip = setparameters(datafiles,TString(dataName),cfgfile); 
	//ip->DumpParms();

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
		std::cerr << " ERROR: ROOT tree file contains an unrecongnized Tree" << std::endl;
		if( ip != 0)
		{
			delete ip;
		}
		exit(-1);
	}
	for(std::vector<TString>::iterator it = datafiles.begin(); it != datafiles.end(); ++it)
	{
		tchaindataset->Add(*it);
	}

	//tchaindataset->GetListOfFiles()->Print();


#ifdef TIMERS
	//T3
	t3 = timer.RealTime();
	timer.Start();
#endif
	// Creating Analysis
	AnalysisVH * analysis = AnalysisBuilder::Build( dataType, fsSignature, ip, tchaindataset ); 

#ifdef TIMERS
	//T4
	t4 = timer.RealTime();
	timer.Start();
#endif
	// Processing
	// Entries
	int nEvents = -1;
	ip->TheNamedInt("nEvents",nEvents);
	int firstEvent = -1 ;
	ip->TheNamedInt("firstEvent",firstEvent);
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
	//std::cout << tchaindataset->GetEntries() << std::endl;
	tchaindataset->Process(analysis,0,nEvents,firstEvent);
	
#ifdef TIMERS
	//T5
	t5 = timer.RealTime();
	timer.Start();
#endif
	//
	// Create the output file and fill it
	// FIXME: Esto va aqui?? o mejor en el destructor del AnalysisVH
	// Putting the outputfile name per default
	std::string outputfile;
	if( ! getOF )
	{
		// Extract the name of the file and get the last 
		std::string filename( ip->TheNamedString("datafilenames_0") );
		size_t barlastpos = filename.rfind("/")+1;
		if( barlastpos == filename.npos )
		{
			// all the string is valid
			barlastpos = 0;
		}
		// Extracting the .root suffix
		const size_t rootpos = filename.find(".root");
		const size_t length  = rootpos-barlastpos;
		std::string almostfinalname = filename.substr(barlastpos,length);
		// And extract the Tree_
		size_t prefix = almostfinalname.rfind("Tree_")+5;
		if( prefix == almostfinalname.npos )
		{
			prefix = 0;
		}
		std::string finalname = almostfinalname.substr(prefix);;

		outputfile = "Results/"+std::string(ip->TheNamedString("MyAnalysis"))+"_"
			+finalname+".root";
	}
	else
	{
		outputfile = outputfilechar;
	}
	std::cout << ">> Saving results to " << outputfile << " ..." << std::endl;
	TString outputfileTS = TString(outputfile);
	if(gSystem->FindFile(".", outputfileTS)) 
	{
		std::cout << "WARNING: File " << outputfile << " already exits!" << std::endl;
		TString outputFileBak = outputfile + ".bak";
		std::cout << "         Moving it to " << outputFileBak << std::endl;
		gSystem->CopyFile(outputfile.c_str(), outputFileBak.Data(), kTRUE);
		gSystem->Unlink(outputfile.c_str());
	}
	TFile histoAnalysis(outputfile.c_str(), "NEW");
	if (histoAnalysis.IsOpen()) 
	{
		TList* li = 0;
		TList* lo = 0;
		li = analysis->GetInputList();
		lo = analysis->GetOutputList();
		li->Write();
		lo->Write();
		histoAnalysis.Close();
	}

	// Now the class is in charge of deleting the InputParameter
	// as is one of its datamembers
	/*if( ip != 0 )
	{
		delete ip;
		ip=0;
	}*/

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
