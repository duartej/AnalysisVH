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
#include<set>

#endif

#include "DatasetManager.h"
//#include "TreeManagerCreator.h"
#include "TreeTypes.h"


// ROOT
#include "TString.h"
#include "TChain.h"

// Storing the datafiles
void AddDataFiles(const std::vector<TString> & files, 
		std::vector<TString> & datafiles)
{
	datafiles.reserve( datafiles.size() + files.size() );
	datafiles.insert( datafiles.end(), files.begin(), files.end() );
}

// Giving a dataset extract all the files
const std::vector<TString> * extractdatafiles(TString dataName = "HW160" )
{
	double xsection = 0;
	int    evtsample= 0;
	bool storeXSE = false;

	///////////////////////////////
	// INPUT DATA SAMPLE
	// 
	// 1) Load DatasetManager
	DatasetManager* dm = 0;
	
	std::vector<TString> * datafiles = new std::vector<TString>;
	// 2) Asign the files 
	if(dataName.Contains("LocalWH")) 
	{
		datafiles->push_back( "/hadoop/PrivateProduction/WH_2l_42X/Tree_WH_2l_42X.root" );
	}
	else if(dataName.Contains("Data")) 
	{
		std::vector<TString> data1= 
			DatasetManager::GetRealDataFiles("../LatinosSkims/Data7TeVRun2011A_newJEC_Reload", 
					"Tree_DoubleMuMay10_210.5");
		AddDataFiles(data1,*datafiles);
                
		std::vector<TString> data2= 
             		  DatasetManager::GetRealDataFiles("../LatinosSkims/Data7TeVRun2011A_newJEC_Reload", 
             				       "Tree_DoubleMuV4_927.9");
		AddDataFiles(data2,*datafiles);
                
		std::vector<TString> data3= 
             		  DatasetManager::GetRealDataFiles("../LatinosSkims/Data7TeVRun2011A_newJEC_Reload", 
             				  "Tree_DoubleMuAug5_334.4");
		AddDataFiles(data3,*datafiles);
             	
		std::vector<TString> data4= 
             		  DatasetManager::GetRealDataFiles("../LatinosSkims/Data7TeVRun2011A_newJEC_Reload", 
				  "Tree_DoubleMuV6_662.9");
		AddDataFiles(data4,*datafiles);
	}
	else 
	{
		storeXSE = true;

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
		AddDataFiles(dm->GetFiles(),*datafiles); //Find files
		
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
		// Freeing memory
		delete dm; 
		dm = 0;
	}
	
	if(datafiles->size() == 0) 
	{
		return 0;
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
	if( (*datafiles)[0].Contains("TESCO") )
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
	// Datafiles
	for(std::vector<TString>::iterator it = datafiles->begin(); it != datafiles->end(); ++it)
	{
		of << it->Data() << std::endl;
	}
	of.close();

	return datafiles;	
}



// Selector creation
/*std::pair<std::string,treeTypes> createdatamanager(const std::vector<TString> & datafiles)
{
	// 3. Create the selector
	TreeManagerCreator sc(datafiles[0]);
	sc.MakeSimpleSelector();

	return std::pair<std::string,treeTypes>(sc.getSelectorFilename(),sc.getTreeType());
}*/


int main(int argc, char *argv[])
{
	const char * dataName; // = "WH160";
	const char * analysispkgpath = ".";
	bool justdatafiles = false;
	const char * creadn = dataName;
	
	//Parsing input options
	if(argc == 1)
	{
		std::cout << "usage: datamanagercreator dataname [options]" << std::endl;
		std::cout << "" << std::endl;
		std::cout << "Options:" << std::endl;
		std::cout << "    -p <base_path_of_the_analysis_package> " << std::endl;
		std::cout << "    -c Avoid the TreeManager concrete class creation " << std::endl;
		std::cout << "       generating just the dataname.dn file" << std::endl;
		std::cout << "    -a Using with the '-c' flag create all the known datanames" << std::endl;
		std::cout << "" << std::endl;
		std::cout << "List of known dataname:" << std::endl;
		std::cout << "    Higgs:             WH# (#: Higgs Mass hypothesis)" << std::endl;
		std::cout << "    Z + Jets Madgraph: ZJets_Madgraph" << std::endl;
		std::cout << "    Z + Jets Powheg:   DYee DYmumu Dytautau Zee_Powheg Zmumu_Powheg Ztautau_Powheg" << std::endl;
		std::cout << "    Zbb + Jets:        Zbb" << std::endl;
		std::cout << "    Other backgrounds: WZ ZZ WW TTbar_Madgraph WJets_Madgraph TW TbarW" << std::endl;
		return -1;
	}
	else if( argc == 2)
	{
		dataName = argv[1];
	}
	else
	{
		//Argument 1 must be a valid input fileName
		dataName = argv[1];
		for(int i = 2; i < argc; i++) 
		{
			if( strcmp(argv[i],"-p") == 0 )
			{
				analysispkgpath = argv[i+1];
			}
			
		}
		for(int i = 2; i < argc; i++) 
		{
			if( strcmp(argv[i],"-p") == 0 )
			{
				analysispkgpath = argv[i+1];
			}
			if( strcmp(argv[i],"-c") == 0 )
			{
				justdatafiles = true;
			}	
			if( strcmp(argv[i],"-a") == 0 )
			{
				creadn = 0;
			}
		}
	}

	std::set<std::string> knowndata;
	// Signal
        knowndata.insert("WH160");
        knowndata.insert("WH120");
        knowndata.insert("WH130");
        knowndata.insert("WH140");
        knowndata.insert("WH150");
        knowndata.insert("WH170");
        knowndata.insert("WH180");
        knowndata.insert("WH190");
        knowndata.insert("WH200");
	// Z+jets
	knowndata.insert("ZJets_Madgraph");
	knowndata.insert("DYee");
	knowndata.insert("DYmumu");
	knowndata.insert("DYtautau");
	knowndata.insert("Zee_Powheg");
	knowndata.insert("Zmumu_Powheg");
	knowndata.insert("Ztautau_Powheg");
	// Zbb+jets
	knowndata.insert("Zbb");
	// Other background
	knowndata.insert("WZ");
	knowndata.insert("ZZ");
	knowndata.insert("WW");
	knowndata.insert("TTbar_Madgraph");
	knowndata.insert("WJets_Madgraph");
	knowndata.insert("TW");
	knowndata.insert("TbarW");

	//Data 
	knowndata.insert("Data");

	// Checking the validity of the input dataname
	if( knowndata.find(std::string(dataName)) == knowndata.end() )
	{
		std::cerr << "datamanagercreator: ERROR dataname '" << dataName << "'"
			<< " not implemented! Exiting..." << std::endl;
		exit(-1);
	}


	//treeTypes dataType;
	const std::vector<TString> * datafiles = extractdatafiles( TString(dataName) );
	if( datafiles == 0 )
	{
		std::cerr << "ERROR: Could not find dataset '" 
			<< dataName << "' with DatasetManager!!! " << std::endl;
		std::cerr << "       Exiting!" << std::endl;      
		exit(-1);
	}

	if( justdatafiles )
	{
		if( creadn == 0 )
		{
			// Creating all the .dn
			for(std::set<std::string>::iterator it = knowndata.begin();
					it != knowndata.end(); ++it)
			{
				const std::vector<TString> * dummy = extractdatafiles( TString(*it) );
				if( dummy != 0 )
				{
					delete dummy;
					dummy = 0;
				}
			}
		}
		//return 0;
	}
	return 0;
	/*
	// Create datamanager
	std::pair<std::string,treeTypes> selfilenameTreeType = createdatamanager(*datafiles);
	std::string selectorfilename = selfilenameTreeType.first;
	dataType = selfilenameTreeType.second;
	
	std::string srcfilename( selectorfilename );
	srcfilename.replace(srcfilename.find(".h"),3,std::string(".cc")) ;

	std::string command1("mv "+selectorfilename+" "+analysispkgpath+"/interface/"+selectorfilename);
	std::string command2("mv "+srcfilename+" "+analysispkgpath+"/src/"+srcfilename);
	
	system( command1.c_str() );
	system( command2.c_str() );
	// Compilation stuff
	//system("make");

	// Freeing mem
	if( datafiles != 0)
	{
		delete datafiles;
	}

	return 0;*/
}
