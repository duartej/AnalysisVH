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
#include<map>

#endif

#include "DatasetManager.h"
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

std::map<std::string,std::vector<std::string> > getdatapathfiles(const char * runperiod,const char * finalstate)
{
	if( finalstate == 0 )
	{
		std::cerr << "\033[31mgetdatapathfile ERROR\033[m Need to kwown the final"
			<< " state to extract the data filenames."
			<< " See \033[37datamanagercreator -h\033[m" << std::endl;
		exit(-1);
	}
	std::string production = "LatinosSkims";
	std::string production2012 = "MC_Summer12/WH";
	std::string runpath;
	std::vector<std::string> filenames;
	std::map<std::string,std::vector<std::string> > mappathfiles;

	if( strcmp(runperiod,"2011A") == 0 )
	{
		runpath = "../"+production+"/Data7TeVRun2011A";
		// O Quizas este --> runpath = "../"+production+"/Data7TeVRun2011A";
		if( strcmp(finalstate,"mmm") == 0 )
		{
			// DoubleMuon
			filenames.push_back("Tree_DoubleMuMay10_Latinos_211.4");
			filenames.push_back("Tree_DoubleMuV4_Latinos_929.7");
			filenames.push_back("Tree_DoubleMuAug5_Latinos_317.8");
			filenames.push_back("Tree_DoubleMuV6_Latinos_658.9");
		}
		else if( strcmp(finalstate,"eee") == 0 )
		{
			filenames.push_back("Tree_DoubleElectronMay10_Latinos_211.4");
			filenames.push_back("Tree_DoubleElectronV4_Latinos_929.7"); // OJO hay dos, lo hara el DM
			filenames.push_back("Tree_DoubleElectronAug5_Latinos_317.8"); 
			filenames.push_back("Tree_DoubleElectronV6_Latinos_658.9");
		}
		else if( strcmp(finalstate,"mme") == 0  ) 
		{
			std::map<std::string,std::vector<std::string> > mapmuons = 
				getdatapathfiles("2011A","mmm");
			// Just checking things are consistent
			if( mapmuons.size() != 1)
			{
				std::cerr << "\033[31mgetdatapathfiles ERROR\033[m Some weird error;"
					<< " this shows some inconsistency in the code. Contact the developer"
					<< std::endl;
				exit(-4);
			}
			filenames.insert( filenames.end(), 
					mapmuons.begin()->second.begin(),mapmuons.begin()->second.end() );
			/*filenames.push_back("Tree_MuEGMay10_210.5");
			filenames.push_back("Tree_MuEGV4_927.9"); // OJO hay dos, lo hara el DM
			filenames.push_back("Tree_MuEGAug5_334.4"); 
			filenames.push_back("Tree_MuEGV6_662.9");*/
		}
		else if( strcmp(finalstate,"eem") == 0  ) 
		{
			std::map<std::string,std::vector<std::string> > mapelec = 
				getdatapathfiles("2011A","eee");
			// Just checking things are consistent
			if( mapelec.size() != 1)
			{
				std::cerr << "\033[31mgetdatapathfiles ERROR\033[m Some weird error;"
					<< " this shows some inconsistency in the code. Contact the developer"
					<< std::endl;
				exit(-4);
			}
			filenames.insert( filenames.end(), 
					mapelec.begin()->second.begin(),mapelec.begin()->second.end() );
		}
		else
		{
			std::cerr << "\033[31mgetdatapathfile ERROR\033[m Not recognized"
				" finalstate '"  << finalstate << "'"
				<< " See \033[37datamanagercreator -h\033[m" << std::endl;
			exit(-1);
		}
		mappathfiles[runpath] = filenames;
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
		else if( strcmp(finalstate,"mme") == 0  )
		{
			std::map<std::string,std::vector<std::string> > mapmuons = 
				getdatapathfiles("2011B","mmm");
			// Just checking things are consistent
			if( mapmuons.size() != 1)
			{
				std::cerr << "\033[31mgetdatapathfiles ERROR\033[m Some weird error;"
					<< " this shows some inconsistency in the code. Contact the developer"
					<< std::endl;
				exit(-4);
			}
			filenames.insert( filenames.end(), 
					mapmuons.begin()->second.begin(),mapmuons.begin()->second.end() );
			//filenames.push_back("Tree_MuEGV1_Latinos_2509");
		}
		else if( strcmp(finalstate,"eem") == 0  )
		{
			std::map<std::string,std::vector<std::string> > mapelec = 
				getdatapathfiles("2011B","eee");
			// Just checking things are consistent
			if( mapelec.size() != 1)
			{
				std::cerr << "\033[31mgetdatapathfiles ERROR\033[m Some weird error;"
					<< " this shows some inconsistency in the code. Contact the developer"
					<< std::endl;
				exit(-4);
			}
			filenames.insert( filenames.end(), 
					mapelec.begin()->second.begin(),mapelec.begin()->second.end() );
		}
		else
		{
			std::cerr << "\033[31mgetdatapathfile ERROR\033[m Not recognized"
				" finalstate '"  << finalstate << "'"
				<< " See \033[37datamanagercreator -h\033[m" << std::endl;
			exit(-1);
		}
		mappathfiles[runpath] = filenames;
	}
	else if( strcmp(runperiod,"2011") == 0 )
	{
		std::map<std::string,std::vector<std::string> > map2011A = 
			getdatapathfiles("2011A",finalstate);
		std::map<std::string,std::vector<std::string> > map2011B = 
			getdatapathfiles("2011B",finalstate);
		// Just checking things are consistent
		if( map2011A.size() != 1 and map2011B.size() != 1)
		{
			std::cerr << "\033[31mgetdatapathfiles ERROR\033[m Some weird error;"
				<< " this shows some inconsistency in the code. Contact the developer"
				<< std::endl;
			exit(-4);
		}
		mappathfiles[map2011A.begin()->first] = map2011A.begin()->second;
		mappathfiles[map2011B.begin()->first] = map2011B.begin()->second;
	}
	else if( strcmp(runperiod,"2012A") == 0 )
	{
		runpath = "../"+production2012;
		if( strcmp(finalstate,"mmm") == 0 )
		{
			// DoubleMuon
			filenames.push_back("Tree_DoubleMuonA");
		}
		else if( strcmp(finalstate,"eee") == 0 )
		{
			filenames.push_back("Tree_DoubleElectronA");
		}
		else if( strcmp(finalstate,"mme") == 0  ) 
		{
			std::map<std::string,std::vector<std::string> > mapmuons = 
				getdatapathfiles("2012A","mmm");
			// Just checking things are consistent
			if( mapmuons.size() != 1)
			{
				std::cerr << "\033[31mgetdatapathfiles ERROR\033[m Some weird error;"
					<< " this shows some inconsistency in the code. Contact the developer"
					<< std::endl;
				exit(-4);
			}
			filenames.insert( filenames.end(), 
					mapmuons.begin()->second.begin(),mapmuons.begin()->second.end() );
		}
		else if( strcmp(finalstate,"eem") == 0  ) 
		{
			std::map<std::string,std::vector<std::string> > mapelec = 
				getdatapathfiles("2012A","eee");
			// Just checking things are consistent
			if( mapelec.size() != 1)
			{
				std::cerr << "\033[31mgetdatapathfiles ERROR\033[m Some weird error;"
					<< " this shows some inconsistency in the code. Contact the developer"
					<< std::endl;
				exit(-4);
			}
			filenames.insert( filenames.end(), 
					mapelec.begin()->second.begin(),mapelec.begin()->second.end() );
		}
		else
		{
			std::cerr << "\033[31mgetdatapathfile ERROR\033[m Not recognized"
				" finalstate '"  << finalstate << "'"
				<< " See \033[37datamanagercreator -h\033[m" << std::endl;
			exit(-1);
		}
		mappathfiles[runpath] = filenames;
	}
	else if( strcmp(runperiod,"2012B") == 0 )
	{
		runpath = "../"+production2012;
		if( strcmp(finalstate,"mmm") == 0 )
		{
			// DoubleMuon
			filenames.push_back("Tree_DoubleMuB_24X");
		}
		else if( strcmp(finalstate,"eee") == 0 )
		{
			filenames.push_back("Tree_DoubleElectronB");
		}
		else if( strcmp(finalstate,"mme") == 0  )
		{
			std::map<std::string,std::vector<std::string> > mapmuons = 
				getdatapathfiles("2012B","mmm");
			// Just checking things are consistent
			if( mapmuons.size() != 1)
			{
				std::cerr << "\033[31mgetdatapathfiles ERROR\033[m Some weird error;"
					<< " this shows some inconsistency in the code. Contact the developer"
					<< std::endl;
				exit(-4);
			}
			filenames.insert( filenames.end(), 
					mapmuons.begin()->second.begin(),mapmuons.begin()->second.end() );
			//filenames.push_back("Tree_MuEGV1_Latinos_2509");
		}
		else if( strcmp(finalstate,"eem") == 0  )
		{
			std::map<std::string,std::vector<std::string> > mapelec = 
				getdatapathfiles("2012B","eee");
			// Just checking things are consistent
			if( mapelec.size() != 1)
			{
				std::cerr << "\033[31mgetdatapathfiles ERROR\033[m Some weird error;"
					<< " this shows some inconsistency in the code. Contact the developer"
					<< std::endl;
				exit(-4);
			}
			filenames.insert( filenames.end(), 
					mapelec.begin()->second.begin(),mapelec.begin()->second.end() );
		}
		else
		{
			std::cerr << "\033[31mgetdatapathfile ERROR\033[m Not recognized"
				" finalstate '"  << finalstate << "'"
				<< " See \033[37datamanagercreator -h\033[m" << std::endl;
			exit(-1);
		}
		mappathfiles[runpath] = filenames;
	}
	else if( strcmp(runperiod,"2012") == 0 )
	{
		std::map<std::string,std::vector<std::string> > map2011A = 
			getdatapathfiles("2012A",finalstate);
		std::map<std::string,std::vector<std::string> > map2011B = 
			getdatapathfiles("2012B",finalstate);
		// Just checking things are consistent
		if( map2011A.size() != 1 and map2011B.size() != 1)
		{
			std::cerr << "\033[31mgetdatapathfiles ERROR\033[m Some weird error;"
				<< " this shows some inconsistency in the code. Contact the developer"
				<< std::endl;
			exit(-4);
		}
		mappathfiles[map2011A.begin()->first] = map2011A.begin()->second;
		mappathfiles[map2011B.begin()->first] = map2011B.begin()->second;
	}
	else
	{
		std::cout << "\033[34mgetdatapathfile\033[m Not Implemented yet ('"
			<< runperiod << "')" << std::endl;
		exit(-3);
	}

	return mappathfiles;
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
	if( dataName.Contains("Data") || dataName.Contains("Fakes") )
	{
		// 1) Take the files which corresponds to each run period
		std::map<std::string,std::vector<std::string> > pathfiles =
			getdatapathfiles(runperiod,finalstate);
		// 2) Asign the files 
		for(std::map<std::string,std::vector<std::string> >::iterator it=pathfiles.begin();
				it != pathfiles.end(); ++it)
		{
			for(unsigned int i = 0; i < it->second.size(); ++i)
			{
				const std::vector<TString> data = DatasetManager::GetRealDataFiles(
						it->first.c_str(),it->second.at(i).c_str() );
				AddDataFiles(data,*datafiles);
			}
		}
	}
	else if(dataName.Contains("LocalWH")) // TO BE DEPRECATED
	{
		datafiles->push_back( "/hadoop/PrivateProduction/WH_2l_42X/Tree_WH_2l_42X.root" );
	}
	else   // Monte Carlo samples
	{
		storeXSE = true;

		TString folder;		
		if( strncmp(runperiod,"2011",4) == 0 )
		{
			folder = "Fall11 Latinos";
		}
		else if( strncmp(runperiod,"2012",4) == 0 )
		{
			folder = "Summer12"; // Latinos";
		}
		else
		{
			std::cerr << "\033[33mextractdatafiles WARNING\033[m"
				<< " Not implemented the run period '"
				<< runperiod << "'. Exiting..."	<< std::endl;
		}
		TString skim("/");

		if (dataName.Contains("WH")) 
		{
			folder = std::string("HWW "+std::string(folder)).c_str();
			skim = "/";
			dataName.Replace(0,2, "WHToWW2L");
		}
		
		// 1) Load DatasetManager
		DatasetManager* dm = 0;
		dm = new DatasetManager(folder, skim);
		
		dm->LoadDataset(dataName);  // Load information about a given dataset
		if( strncmp(runperiod,"2012",4) == 0 )
		{
			// The MC_Summer12 datafiles are not following the standard
			// notation of the 2011 files (the rush of production...), so
			// I'm patching everywhere to deal with it. This is another patch...
			dm->SetSkim("WH");
		}
		AddDataFiles(dm->GetFiles(),*datafiles); //Find files
		
		xsection = dm->GetCrossSection();
		evtsample= dm->GetEventsInTheSample();
		if( evtsample == 0 )
		{
			std::cerr << "\033[33mextractdatafiles WARNING\033[m"
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

	// Run period
	of << "RUNPERIOD:" << runperiod << std::endl;
	
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
	std::cout << "usage: datamanagercreator [dataname] [options]" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "    -r <201*A|201*B|201*> *=1,2  Run period to extract datafiles" << std::endl;
	std::cout << "    -f <mmm|mme|eee|eem>         Final state to be used in the data case" << std::endl;
	std::cout << "    -h                           Displays this help message and exits " << std::endl;
	std::cout << "" << std::endl;
	std::cout << "The 'dataname' must be one of the shown below. If 'dataname' is not used, all the dataname"
		<< " in the list will be extracted" << std::endl;
	std::cout << "List of known dataname:" << std::endl;
	std::cout << "    Higgs:             WH# (#: Higgs Mass hypothesis)" << std::endl;
	std::cout << "    Z + Jets Madgraph: ZJets_Madgraph" << std::endl;
	std::cout << "    Z + Jets Powheg:   DYee_Powheg DYmumu_Powheg Dytautau_Powheg Zee_Powheg Zmumu_Powheg Ztautau_Powheg" << std::endl;
	std::cout << "    VGamma + Jets  :   PhotonVJets_Madgraph" << std::endl;
	std::cout << "    VGamma         :   ZgammaToMuMuMad ZgammaToElElMad ZgammaToTauTauMad WgammaToMuNuMad WgammaToElNuMad WgammaToTauNuMad" << std::endl;
	std::cout << "    Zbb + Jets:        Zbb" << std::endl;
	std::cout << "    Other backgrounds: -WZ --> PYTHIA SAMPLE TO BE DEPRECATED-\n" 
		<<   "                       WZTo3LNu ZZ WW TTbar_Madgraph WJets_Madgraph TW TbarW Fakes" << std::endl;
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
						&& strcmp(runperiod,"2011") != 0
						&& strcmp(runperiod,"2012A") != 0
						&& strcmp(runperiod,"2012B") != 0
						&& strcmp(runperiod,"2012") != 0 )

				{
					std::cerr << "\033[31mdatamanager ERROR\033[m "
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
		std::cout << "\033[34mdatamanagercreator INFO\033[m All the known datasets" 
			<< " will be created" << std::endl;
		//std::cout << "List of known datasets:"
	}
	if( runperiod == 0 )
	{
		std::cerr << "\033[31mdatamanagercreator ERROR:\033[m The '-r' argument is mandatory!"
			<< std::endl;
		display_usage();
		return -1;
	}
	if( finalstate == 0 )
	{
		std::cerr << "\033[31mdatamanagercreator ERROR:\033[m The '-f' argument is mandatory!"
			<< std::endl;
		display_usage();
		return -1;
	}

	std::set<std::string> knowndata;
	// Signal
        knowndata.insert("WH120");
        knowndata.insert("WH130");
	// Z+jets
	knowndata.insert("ZJets_Madgraph");
	knowndata.insert("DYee_Powheg");
	knowndata.insert("DYmumu_Powheg");
	knowndata.insert("DYtautau_Powheg");
	knowndata.insert("Zee_Powheg");
	knowndata.insert("Zmumu_Powheg");
	knowndata.insert("Ztautau_Powheg");
	// VGamma + Jets
	knowndata.insert("PhotonVJets_Madgraph");
	// VGamma 
	knowndata.insert("ZgammaToMuMuMad");
	knowndata.insert("ZgammaToElElMad");
	knowndata.insert("ZgammaToTauTauMad");
	knowndata.insert("WgammaToMuNuMad");
	knowndata.insert("WgammaToElNuMad");
	knowndata.insert("WgammaToTauNuMad");
	// Zbb+jets
	knowndata.insert("Zbb");
	// Other background
	// knowndata.insert("WZ"); --> Pythia sample to be deprecated
	knowndata.insert("WZTo3LNu");
	knowndata.insert("ZZ");
	knowndata.insert("WW");
	//knowndata.insert("TTbar_Madgraph");
	knowndata.insert("TTbar_2L2Nu_Powheg");
	knowndata.insert("WJets_Madgraph");
	knowndata.insert("TW_DR");
	knowndata.insert("TbarW_DR");
	knowndata.insert("Fakes");

	//Data 
	knowndata.insert("Data");

	// Creating the .dn
	bool wasprocessed = false;
	for(std::set<std::string>::iterator it = knowndata.begin(); it != knowndata.end(); ++it)
	{
		if( dataName != 0 )
		{
			// Checking the validity of the input dataname
			if( knowndata.find(std::string(dataName)) == knowndata.end() )
			{
				std::cerr << "\033[31mdatamanagercreator ERROR\033[m: dataname '" << dataName << "'"
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
	}

	if( ! wasprocessed )
	{
		std::cerr << "\033[31mdatamanagercreator ERROR\033[m: Could not find dataset '" 
			<< dataName << "' in the list of known datasets. You should update this code..." << std::endl;
		return -2;
	}

	return 0;
}
