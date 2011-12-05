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

std::pair<std::string,std::vector<std::string> > getdatapathfiles(const char * runperiod,const char * finalstate)
{
	if( finalstate == 0 )
	{
		std::cerr << "\033[1;31mgetdatapathfile ERROR\033[1;m Need to kwown the final"
			<< " state to extract the data filenames."
			<< " See \033[1;37datamanagercreator -h\033[1;m" << std::endl;
		exit(-1);
	}
	std::string production = "LatinosSkims";
	std::string runpath;
	std::vector<std::string> filenames;

	if( strcmp(runperiod,"2011A") == 0 )
	{
		runpath = "../"+production+"/Data7TeVRun2011A_newJEC_Reload";
		// O Quizas este --> runpath = "../"+production+"/Data7TeVRun2011A";
		if( strcmp(finalstate,"mmm") == 0 )
		{
			// DoubleMuon
			filenames.push_back("Tree_DoubleMuMay10_210.5");
			filenames.push_back("Tree_DoubleMuV4_927.9");
			filenames.push_back("Tree_DoubleMuAug5_334.4");
			filenames.push_back("Tree_DoubleMuV6_662.9");
		}
		else if( strcmp(finalstate,"eee") == 0 )
		{
			filenames.push_back("Tree_DoubleElectronMay10_210.5");
			filenames.push_back("Tree_DoubleElectronV4_927.9"); // OJO hay dos, lo hara el DM
			filenames.push_back("Tree_DoubleElectronAug5_334.4"); 
			filenames.push_back("Tree_DoubleElectronV6_662.9");
		}
		else if( strcmp(finalstate,"mme") == 0  ||
			strcmp(finalstate,"eem") == 0 )
		{
			filenames.push_back("Tree_MuEGMay10_210.5");
			filenames.push_back("Tree_MuEGV4_927.9"); // OJO hay dos, lo hara el DM
			filenames.push_back("Tree_MuEGAug5_334.4"); 
			filenames.push_back("Tree_MuEGV6_662.9");
		}
		else
		{
			std::cerr << "\033[1;31mgetdatapathfile ERROR\033[1;m Not recognized"
				" finalstate '"  << finalstate << "'"
				<< " See \033[1;37datamanagercreator -h\033[1;m" << std::endl;
			exit(-1);
		}
	}
	else if( strcmp(runperiod,"2011B") == 0 )
	{
		runpath = "../"+production+"/Data7TeVRun2011B";
		if( strcmp(finalstate,"mmm") == 0 )
		{
			// DoubleMuon
			filenames.push_back("Tree_DoubleMuV1_Latinos_2509");
		}
		else if( strcmp(finalstate,"eee") == 0 )
		{
			filenames.push_back("Tree_DoubleElectronV1_Latinos_2509");
		}
		else if( strcmp(finalstate,"mme") == 0  ||
			strcmp(finalstate,"eem") == 0 )
		{
			filenames.push_back("Tree_MuEGV1_Latinos_2509");
		}
		else
		{
			std::cerr << "\033[1;31mgetdatapathfile ERROR\033[1;m Not recognized"
				" finalstate '"  << finalstate << "'"
				<< " See \033[1;37datamanagercreator -h\033[1;m" << std::endl;
			exit(-1);
		}
	}
	else
	{
		std::cout << "\033[1;34mgetdatapathfile\033[1;m Not Implemented yet ('"
			<< runperiod << "')" << std::endl;
		exit(-3);
	}

	return std::pair<std::string,std::vector<std::string> >(runpath,filenames);
}


// Giving a dataset extract all the files
const std::vector<TString> * extractdatafiles(TString dataName, const char * runperiod, 
		const char * finalstate )
{
	double xsection = 0;
	int    evtsample= 0;
	bool storeXSE = false;

	///////////////////////////////
	// INPUT DATA SAMPLE
	// 
	

	std::vector<TString> * datafiles = new std::vector<TString>;
	if( dataName.Contains("Data") )
	{
		// 1) Take the files which corresponds to each run period
		std::pair<std::string,std::vector<std::string> > pathfiles =
			getdatapathfiles(runperiod,finalstate);
		// 2) Asign the files 
		for(unsigned int i = 0; i < pathfiles.second.size(); ++i)
		{
			const std::vector<TString> data = DatasetManager::GetRealDataFiles(
					pathfiles.first.c_str(),pathfiles.second.at(i).c_str() );
			AddDataFiles(data,*datafiles);
		}
	}
	else if(dataName.Contains("LocalWH")) // TO BE DEPRECATED
	{
		datafiles->push_back( "/hadoop/PrivateProduction/WH_2l_42X/Tree_WH_2l_42X.root" );
	}
	else   // Monte Carlo samples
	{
		storeXSE = true;


		//TString folder("Summer11 Latinos");
		TString folder("Fall11 Latinos");
		TString skim("/");

		if (dataName.Contains("WH")) 
		{
			folder = "HWW Fall11 Latinos";
			//folder = "HWW Summer11 Latinos";
			skim = "/";
			dataName.Replace(0,2, "WHToWW2L");
		}
		
		// 1) Load DatasetManager
		DatasetManager* dm = 0;
		dm = new DatasetManager(folder, skim);
		
		dm->LoadDataset(dataName);  // Load information about a given dataset
		AddDataFiles(dm->GetFiles(),*datafiles); //Find files
		
		xsection = dm->GetCrossSection();
		evtsample= dm->GetEventsInTheSample();
		if( evtsample == 0 )
		{
			std::cerr << "\033[1;33mextractdatafiles WARNING\033[1;m"
				<< " Not found the dataset '" << dataName << "'. Skipping the"
				<< " '" << dataName << "_datanames.dn' creation."
				<< std::endl;
			if( dm != 0)
			{
				delete dm;
			}
			if( datafiles != 0 )
			{
				delete datafiles;
			}

			return 0;
		}
		
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
		delete datafiles;
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

void display_usage()
{
	std::cout << "usage: datamanagercreator dataname [options]" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "    -r <2011A|2011B|2011>       Run period to extract datafiles" << std::endl;
	std::cout << "    -f <mmm|mme|eee|eem>        Final state to be used in the data case" << std::endl;
	std::cout << "    -h                          Displays this help message and exits " << std::endl;
	std::cout << "" << std::endl;
	std::cout << "List of known dataname:" << std::endl;
	std::cout << "    Higgs:             WH# (#: Higgs Mass hypothesis)" << std::endl;
	std::cout << "    Z + Jets Madgraph: ZJets_Madgraph" << std::endl;
	std::cout << "    Z + Jets Powheg:   DYee DYmumu Dytautau Zee_Powheg Zmumu_Powheg Ztautau_Powheg" << std::endl;
	std::cout << "    Zbb + Jets:        Zbb" << std::endl;
	std::cout << "    Other backgrounds: WZ ZZ WW TTbar_Madgraph WJets_Madgraph TW TbarW" << std::endl;
}

int main(int argc, char *argv[])
{
	const char * dataName  = 0; 
	const char * runperiod = 0; 
	const char * finalstate= 0;
	
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
		for(int i = 1; i < argc; i++) 
		{
			if( strcmp(argv[i],"-h") == 0 )
			{
				display_usage();
				return 0;
			}	
			if( strcmp(argv[i],"-r") == 0 )
			{
				runperiod = argv[i+1];
				if( strcmp(runperiod,"2011A") != 0 
						&& strcmp(runperiod,"2011B") != 0 
						&& strcmp(runperiod,"2011") != 0 )
				{
					std::cerr << "\033[1;31mdatamanager ERROR\033[1;m "
						<< "Not a valid run period: '" << runperiod
						<< "'" << std::endl;
					display_usage();
					return -1;
				}
				usedargs.insert(i);
				usedargs.insert(i+1);
				i++;
			}
			if( strcmp(argv[i],"-f") == 0 )
			{
				finalstate = argv[i+1];
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
		std::cout << "\033[1;34mdatamanagercreator INFO\033[1;m All the known datasets" 
			<< " will be created" << std::endl;
		//std::cout << "List of known datasets:"
	}
	if( runperiod == 0 )
	{
		std::cerr << "\033[1;31mdatamanagercreator ERROR:\033[1;m The '-r' argument is mandatory!"
			<< std::endl;
		display_usage();
		return -1;
	}
	if( finalstate == 0 )
	{
		std::cerr << "\033[1;31mdatamanagercreator ERROR:\033[1;m The '-f' argument is mandatory!"
			<< std::endl;
		display_usage();
		return -1;
	}

	std::set<std::string> knowndata;
	// Signal
        knowndata.insert("WH120");
        knowndata.insert("WH130");
        //knowndata.insert("WH140");
        //knowndata.insert("WH150");
        //knowndata.insert("WH160");
        //knowndata.insert("WH170");
        //knowndata.insert("WH180");
        //knowndata.insert("WH190");
        //knowndata.insert("WH200");
	// Z+jets
	knowndata.insert("ZJets_Madgraph");
	knowndata.insert("DYee_Powheg");
	knowndata.insert("DYmumu_Powheg");
	knowndata.insert("DYtautau_Powheg");
	knowndata.insert("Zee_Powheg");
	knowndata.insert("Zmumu_Powheg");
	knowndata.insert("Ztautau_Powheg");
	// Zbb+jets
	knowndata.insert("Zbb");
	// Other background
	knowndata.insert("WZ"); // WZTo3LNu ??? Mejor este
	knowndata.insert("ZZ");
	knowndata.insert("WW");
	knowndata.insert("TTbar_Madgraph");
	knowndata.insert("WJets_Madgraph");
	knowndata.insert("TW_DR");
	knowndata.insert("TbarW_DR");

	//Data 
	knowndata.insert("Data");



	//treeTypes dataType;
	// Creating the .dn
	bool wasprocessed = false;
	for(std::set<std::string>::iterator it = knowndata.begin(); it != knowndata.end(); ++it)
	{
		if( dataName != 0 )
		{
			// Checking the validity of the input dataname
			if( knowndata.find(std::string(dataName)) == knowndata.end() )
			{
				std::cerr << "datamanagercreator: ERROR dataname '" << dataName << "'"
					<< " not implemented! Exiting..." << std::endl;
				return -1;
			}
			if( strcmp(dataName, (*it).c_str() ) != 0 )
			{
				continue;
			}
		}
		const std::vector<TString> * dummy = extractdatafiles( TString(*it),runperiod,finalstate );
		if( dummy != 0 )
		{
			delete dummy;
			dummy = 0;
			wasprocessed = true;
		}
//		else
//		{
//			std::cerr << "\033[1;31mdatamanagercreator ERROR\033[1;m: Could not find dataset '" 
//				<< *it << "'. Is it a valid name? " << std::endl;
//			exit(-1);
//		}
	}

	if( ! wasprocessed )
	{
		std::cerr << "\033[1;31mdatamanagercreator ERROR\033[1;m: Could not find dataset '" 
			<< dataName << "' in the list of known datasets. You should update this code..." << std::endl;
		return -2;
	}

	return 0;
}
