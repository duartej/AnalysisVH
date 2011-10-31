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

#include "AnalysisBuilder.h"
#include "AnalysisVH.h"


// ROOT
#include "TString.h"
#include "TChain.h"


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
	const char * analysisheader = "interface/AnalysisWH_mmm.h";
	const char * cfgfile = "analisiswh_mmm.ip";
	const char * runtype = "local"; //"local"; // "grid"
	
	// Extract the datafiles from the file created in the "local" path
	std::pair<treeTypes,std::vector<TString> > dum = extractdatafiles( dataName );
	std::vector<TString> datafiles = dum.second;
	treeTypes dataType  = dum.first;
	
	// Initialize the analysis specific parameters using a config file
	InputParameters * ip = setparameters(datafiles,TString(dataName),cfgfile); 
	//ip->DumpParms();

	// Run analysis: call command
	//AnalysisWH * analisis = AnalysisBuilder::Create( treeType );
	std::cout << dataType << std::endl;
	AnalysisVH * analysis = AnalysisBuilder::Build( dataType, ip );
	

	if( ip != 0 )
	{
		delete ip;
		ip=0;
	}

	if( analysis != 0 )
	{
		delete analysis;
		analysis = 0;
	}

}
