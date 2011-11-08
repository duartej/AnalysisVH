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
#include "TreeManagerCreator.h"
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
const std::vector<TString> extractdatafiles(TString dataName = "HW160" )
{
	///////////////////////////////
	// INPUT DATA SAMPLE
	// 
	// 1) Load DatasetManager
	DatasetManager* dm = 0;
	
	std::vector<TString> datafiles;
	// 2) Asign the files 
	if(dataName.Contains("LocalWH")) 
	{
		datafiles.push_back( "/hadoop/PrivateProduction/WH_2l_42X/Tree_WH_2l_42X.root" );
	}
	else if(dataName.Contains("Data")) 
	{
		std::vector<TString> data1= 
			DatasetManager::GetRealDataFiles("Data7TeVRun2011A_newJEC", 
					"Tree_DoubleMu_May10_Skim2LPt1010_215.3");
		AddDataFiles(data1,datafiles);
                
		std::vector<TString> data2= 
             		  DatasetManager::GetRealDataFiles("Data7TeVRun2011A_newJEC", 
             				       "Tree_DoubleMuV4_Skim2LPt1010_927.9");
		AddDataFiles(data1,datafiles);
                
		std::vector<TString> data3= 
             		  DatasetManager::GetRealDataFiles("Data7TeVRun2011A_newJEC", 
             				  "Tree_DoubleMuAug5_Skim2LPt1010_334.4");
		AddDataFiles(data1,datafiles);
             	
		std::vector<TString> data4= 
             		  DatasetManager::GetRealDataFiles("Data7TeVRun2011A_newJEC", 
				  "Tree_DoubleMuV6_Skim2LPt1010_662.9");
		AddDataFiles(data1,datafiles);
	
		std::cout << ">> List of Real Data Files:" << std::endl;
		
	}
	else 
	{
		TString folder("Summer11");
		TString skim("Skim2LPt1010");
		//TString folder("Spring11Latinos");
		//TString skim("/");

		if (dataName.Contains("WH")) 
		{
			folder = "Summer11";
			skim = "HWW";
			dataName.Replace(0,2, "WHToWW2L");
		}
		
		//Use DatasetManager
		dm = new DatasetManager(folder, skim);
		
		dm->LoadDataset(dataName);  // Load information about a given dataset
		AddDataFiles(dm->GetFiles(),datafiles); //Find files
		
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
	for(std::vector<TString>::iterator it = datafiles.begin(); it != datafiles.end(); ++it)
	{
		of << it->Data() << std::endl;
	}
	of.close();

	return datafiles;	
}



// Selector creation
std::pair<std::string,treeTypes> createdatamanager(const std::vector<TString> & datafiles)
{
	// 3. Create the selector
	TreeManagerCreator sc(datafiles[0]);
	sc.MakeSimpleSelector();

	return std::pair<std::string,treeTypes>(sc.getSelectorFilename(),sc.getTreeType());
}


int main(int argc, char *argv[])
{
	const char * dataName; // = "WH160";
	const char * cfgfile        = "analisiswh_mmm.ip";
	const char * analysispkgpath = ".";
	//const char * analysisheader = "interface/AnalysisWH_mmm.h";
	
	//Parsing input options
	if(argc == 1)
	{
		std::cout << "usage: datamanagercreator dataname [options]" << std::endl;
		std::cout << "" << std::endl;
		std::cout << "Options:" << std::endl;
		std::cout << "    -c configuration file " << std::endl;
		std::cout << "    -p base path of the analysis package " << std::endl;
		std::cout << "" << std::endl;
		std::cout << "List of known dataname:" << std::endl;
		std::cout << "    Higgs:             WH# (#: Higgs Mass hypothesis)" << std::endl;
		std::cout << "    Z + Jets Madgraph: ZJets_Madgraph" << std::endl;
		std::cout << "    Z + Jets Powheg:   DYee DYmumu Dytautau Zee_Powheg Zmumu_Powheg Ztautau_Powheg" << std::endl;
		std::cout << "    Zbb + Jets:        Zbb" << std::endl;
		std::cout << "    Other backgrounds: WZ ZZ WW TTbar_Madgraph WJetas_Madgraph TW TbarW" << std::endl;
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
			if( strcmp(argv[i],"-c") == 0 )
			{
				cfgfile = argv[i+1];
			}
			if( strcmp(argv[i],"-p") == 0 )
			{
				analysispkgpath = argv[i+1];
			}
			
		}
	}
	
	treeTypes dataType;
	std::vector<TString> datafiles;
	datafiles = extractdatafiles( TString(dataName) );
	// Create datamanager
	std::pair<std::string,treeTypes> selfilenameTreeType = createdatamanager(datafiles);
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

	return 0;
}
