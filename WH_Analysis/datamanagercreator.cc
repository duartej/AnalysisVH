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
#include "InputParameters.h"
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

void PrintDataFiles(const std::vector<TString> & dataFiles)
{
	for(unsigned int i = 0; i < dataFiles.size(); ++i)
	{
		std::cout << "* " << dataFiles[i] << std::endl;
	}
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
		
		PrintDataFiles(datafiles);
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

// Overloaded function to extract the file names from a previous stored file
// It will catch the files inside a subset
std::pair<treeTypes,std::vector<TString> > extractdatafiles( const char * dataName, const int & subset = 0 )
{
	TString dataNameprov(dataName);
	if (dataNameprov.Contains("WH")) 
	{
		dataNameprov.Replace(0,2, "WHToWW2L");
	}
	dataNameprov += "_datanames.dn";
	const char * filename = dataNameprov.Data();

	std::ifstream inputf( filename );
	if( ! inputf.is_open() )
	{
		std::cerr << "ERROR: Imposible to open the file '" << filename 
			<< "' to retrieve file names information.\n"
			<< "Run this code previously with the '-t local' option to create the file"
			<< std::endl;
		exit(-1);
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
		if( line != "")
		{
			datafiles.push_back( TString(line) );
		}
	}
	inputf.close();


	return std::pair<treeTypes,std::vector<TString> >(en_treeType,datafiles);
}


// Selector creation
std::pair<std::string,treeTypes> createdatamanager(const std::vector<TString> & datafiles)
{
	// 3. Create the selector
	TreeManagerCreator sc(datafiles[0]);
	sc.MakeSimpleSelector();

	return std::pair<std::string,treeTypes>(sc.getSelectorFilename(),sc.getTreeType());
}

// Change the header file of the analysis class in order to inherit from the
// new Selector input dependent class
/*void modifyheader(const std::string & analysisfilename, const std::string & selectorfilename, 
		const bool & doBackup = true)
{
	// Extracting the name of the classes assuming they have the same names as its headers
	std::string classname = selectorfilename.substr(0,selectorfilename.rfind(".h"));
	if( classname.find("/") > 0 )
	{
		classname = classname.substr(classname.rfind("/")+1);
	}
	std::string anclassname = analysisfilename.substr(0,analysisfilename.rfind(".h"));
	// Note to extract the total path name
	if( anclassname.find("/") > 0 )
	{
		anclassname = anclassname.substr(anclassname.rfind("/")+1);
	}

	std::ifstream af(analysisfilename.c_str());
	if (!af.is_open()) 
	{
		std::cerr << "ERROR: Cannot open input file '" << analysisfilename
			<< "'" << std::endl;
		af.close();
		exit(-1);
	}
	
	//Searching the #define directive
	std::string line;
	std::queue<std::string> outlines;
	bool isChecked = false;
	bool foundClass= false;
	while( ! af.eof() )
	{
		getline(af,line);
		// Create the inheritage 
		if( !foundClass && line.find("class "+anclassname) != std::string::npos )
		{
			outlines.push( "class "+anclassname+" : public "+classname+" { "
					+"   //Inheritage created by 'selectorcreator'");
			foundClass = true;
			continue;
		}

		outlines.push(line);
		if( (!isChecked) && line.find("#define") != std::string::npos )
		{
			outlines.push( "\n" );
			outlines.push( "// Line below added by 'selectorcreator'. Do not change, instead run again" );
			outlines.push( "// the 'selectcreator' executable after inspect a new (MiniTree) file.") ;
			outlines.push( "#include \""+selectorfilename+"\"");
			outlines.push( "\n" );
			isChecked = true;
			continue;
		}
		// If already contains the include skip it
		if( line.find(selectorfilename) != std::string::npos )
		{
			af.close();
			return;
		}
	}
	af.close();

	// Make a backup copy if necessary
	if( doBackup )
	{
		std::ifstream ifs(analysisfilename.c_str(), std::ios::binary);
		std::ofstream ofs((analysisfilename+"_BACKUP").c_str(), std::ios::binary);
		ofs << ifs.rdbuf();
		ofs.close();
		ifs.close();
	}

	// Writing the file
	std::ofstream aof(analysisfilename.c_str());
	while( outlines.size() > 0)
	{
		aof << outlines.front().c_str() << std::endl;
		outlines.pop();
	}
}*/




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

	// + Data Name
	ip->SetNamedString("DataName", dataName.Data());
	// + Luminosity
	// + Cuts
	//   - Pt
	
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



int main(int argc, char *argv[])
{
	// FIXME: Argumento de entrada: tipo de muestra
	//                              path al header de analisis
	//                              
	const char * dataName = "WH160";
	//const char * analysisheader = "interface/AnalysisWH_mmm.h";
	const char * cfgfile = "analisiswh_mmm.ip";
	
	treeTypes dataType;
	std::vector<TString> datafiles;
	datafiles = extractdatafiles( TString(dataName) );
	// Create datamanager
	std::pair<std::string,treeTypes> selfilenameTreeType = createdatamanager(datafiles);
	std::string selectorfilename = selfilenameTreeType.first;
	dataType = selfilenameTreeType.second;
	// Introduces the new selector to the analysis class
	// modifyheader(std::string(analysisheader),selectorfilename); --> Ya no es necesario
	
	std::string srcfilename( selectorfilename );
	srcfilename.replace(srcfilename.find(".h"),3,std::string(".cc")) ;

	std::string command1("mv "+selectorfilename+" interface/"+selectorfilename);
	std::string command2("mv "+srcfilename+" src/"+srcfilename);
	
	system( command1.c_str() );
	system( command2.c_str() );
	// Compilation stuff
	//system("make");
	// And out
	exit(0);
	
	//std::cout << "List of datafiles associated to the data '"<< dataName << "'" 
	//	<< "[" << treeType << "]:" << std::endl;
	for(std::vector<TString>::iterator it = datafiles.begin(); it != datafiles.end(); ++it)
	{
		std::cout << "*" << *it << std::endl;
	}
	// Initialize the analysis specific parameters using a config file
	InputParameters * ip = setparameters(datafiles,TString(dataName),cfgfile); 
	//ip->DumpParms();

	// Run analysis: call command
	//AnalysisWH * analisis = AnalysisBuilder::Create( treeType );
	//std::cout << dataType << std::endl;
	//AnalysisVH * analysis = AnalysisBuilder::Build( dataType, ip );
	

	if( ip != 0 )
	{
		delete ip;
		ip=0;
	}

	//if( analysis != 0 )
	//{
	//	delete analysis;
	//	analysis = 0;
	//}

}
