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
#ifdef TIMERS
#include "TStopwatch.h"
#endif


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
#ifdef TIMERS
	TStopwatch timer;
	timer.Start();
	double t1, t2, t3, t4, t5, t6, t7, t8;
#endif
	
#ifdef TIMERS
	//T1
	t1 = timer.RealTime();
	timer.Start();
#endif
	// Extract the datafiles from the file created in the "local" path
	std::pair<treeTypes,std::vector<TString> > dum = extractdatafiles( dataName );
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

#ifdef TIMERS
	//T3
	t3 = timer.RealTime();
	timer.Start();
#endif
	// Creating selector
	AnalysisVH * analysis = AnalysisBuilder::Build( dataType, ip );

#ifdef TIMERS
	//T4
	t4 = timer.RealTime();
	timer.Start();
#endif
	// Processing
	tchaindataset->Process(analysis);
	
#ifdef TIMERS
	//T5
	t5 = timer.RealTime();
	timer.Start();
#endif
	//
	// Create the ouptut file and fill it
	//
/*	std::string outputfile("outputtest.root");
	std::cout << ">> Saving results to " << outputfile << " ..." << std::endl;
	if(gSystem->FindFile(".", outputfile.c_str() )) 
	{
		std::cout << "WARNING: File " << outputfile << " already exits!" << std::endl;
		TString outputFileBak = outputfile + ".bak";
		std::cout << "         Moving it to " << outputFileBak << std::endl;
		gSystem->CopyFile(outputfile, outputFileBak, kTRUE);
		gSystem->Unlink(outputfile);
	}
	TFile histoAnalysis(outputfile, "NEW");
	if (histoAnalysis.IsOpen()) 
	{
		TList* li = 0;
		TList* lo = 0;
		li = inputlist;
		lo = analysis->GetOutputList();
		li->Write();
		lo->Write();
		histoAnalysis.Close();
	}*/

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
#ifdef TIMERS
  t8 = timer.RealTime();

  std::cout << "Tiempos de ejecucion:" << std::endl;
  std::cout << t1 << ", " << t2 << ", " << t3 << std::endl;
#endif

}
