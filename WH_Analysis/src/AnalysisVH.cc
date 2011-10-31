#include<sstream>
#include<stdlib.h>
#include<cstring>
#include<iostream>

#include "AnalysisVH.h"
#include "InputParameters.h"
#include "CutManager.h"

#include "TTree.h"

// To be changed: FIXME
const double kZMass = 91.1876;
//const unsigned int kNMuons = 3; 
const unsigned int kNMuons = 2; 
const unsigned int kWPID   = 24; //Found with TDatabasePDG::Instance()->GetParticle("W+")->PdgCode()
const unsigned int kTauPID = 15; //Found with TDatabasePDG::Instance()->GetParticle("tau-")->PdgCode()


// Prepare analysis Constructor
AnalysisVH::AnalysisVH(TreeManager * data, InputParameters * ip, 
		CutManager * selectioncuts, TTree * tree ) :
	CMSAnalysisSelector(tree),
	_selectioncuts(selectioncuts)//,
//	fPUWeight(0),
//	fMuonSelector(0),
//	fHistos(0),	
//	fIsWH(false)
{
	// FIXME: Check that the data is attached to the selector manager

	// The Inputparameters have to be initialized before, just to complete it
	// introducing the set of datasets: 
	//  --> extract filenames: datafilenames_[index]
	std::string filenames( "datafilenames_" );
	// -- Checking is there
	if( ip == 0 )
	{
		std::cerr << "AnalysisVH::AnalysisVH ERROR: The 'InputParameters' argument "
			<< "cannot be passed as NULL pointer, initialize first!! Exiting... " << std::endl;
		exit(-1);
	}

	if( ip->TheNamedString( filenames+"0" ) == 0 )
	{
		std::cerr << "AnalysisVH::AnalysisVH ERROR: The 'InputParameters' argument "
			<< "must contain a 'datafilenames_0' value!! Exiting..." << std::endl;
		exit(-1);
	}
	std::stringstream istr;
	int id = 0;
	istr << id;
	const char * filename  = 0;
	while( (filename = ip->TheNamedString(filenames+istr.str())) )
	{
		_datafiles.push_back( filename );
		id++;
		istr << id;
	}
	fInputParameters = ip;

	// Including to the list of fInputs the InputParameter
	TList * inputlist = new TList;
	inputlist->Add(fInputParameters);
	this->SetInputList(inputlist);

	/*
	for(unsigned int i = 0; i < _datafiles.size(); ++i)
	{
		std::cout << _datafiles[i] << std::endl;
	}*/
}

void AnalysisVH::InitialiseParameters()
{
	InputParameters * ip = this->GetInputParameters();

	//Cuts
	//----
	//   - Pt of leptons
	std::vector<double> * cuts = new std::vector<double>;
	double dummy = 0;
	// Possiblemente se pueda coger dependiendo
	// que analisis (mu o ele) y extraerlo aqui
	ip->TheNamedDouble("MinMuPt1", dummy);
	cuts->push_back(dummy);
	ip->TheNamedDouble("MinMuPt2", dummy);
	cuts->push_back(dummy);
	ip->TheNamedDouble("MinMuPt3", dummy);
	cuts->push_back(dummy);
	_selectioncuts->SetPtMinCuts(*cuts);
	cuts->clear();

	//   - IP and DeltaZ of track associated with muon w.r.t PV

	ip->TheNamedDouble("MaxMuIP2DInTrack", dummy);
	cuts->push_back(dummy);
	_selectioncuts->SetUndefCuts(*cuts,CutManager::kMaxMuIP2DInTrack);
	cuts->clear();

	ip->TheNamedDouble("MaxDeltaZMu", dummy );
	cuts->push_back(dummy);
	_selectioncuts->SetUndefCuts(*cuts,CutManager::kMaxDeltaZMu);
	cuts->clear();
	
	//   - Isolation: (PTtraks + ETcalo)/PTmuon
	ip->TheNamedDouble("MaxIsoMu", dummy);
	cuts->push_back(dummy);
	_selectioncuts->SetIsoCuts(*cuts);
	cuts->clear();
	
	//   - Max DeltaR between muons
	ip->TheNamedDouble("MaxDeltaRMuMu",dummy);
	cuts->push_back(dummy);
	_selectioncuts->SetUndefCuts(*cuts,CutManager::kMaxDeltaRMuMu);
	cuts->clear();
	
	//   - Z mass window
	double deltazmass=0;
	ip->TheNamedDouble("DeltaZMass", deltazmass);
	cuts->push_back( kZMass - deltazmass );
	_selectioncuts->SetUndefCuts(*cuts,CutManager::kMinZMass);
	cuts->clear();
	cuts->push_back( kZMass + deltazmass );
	_selectioncuts->SetUndefCuts(*cuts,CutManager::kMaxZMass);
	cuts->clear();
	
	//   - Min MET of the event
	ip->TheNamedDouble("MinMET", dummy);
	cuts->push_back(dummy);
	_selectioncuts->SetEventCuts(*cuts);
	cuts->clear();

	delete cuts;
	cuts = 0;
	
	// Variables describing dataset...
	//--------------------------------
	fDataName = ip->TheNamedString("DataName");
	fIsWH = false;
	fIsData = false;
	if(fDataName.Contains("WH"))
	{
		fIsWH = true;
	}
	else if(fDataName.Contains("Data"))
	{
		fIsData = true;
	}
	
	// Luminosity
	//--------------------------------
	ip->TheNamedDouble("Luminosity", fLuminosity);
	
#ifdef DEBUGANALYSIS
	std::cout << "DEBUG: IsWH   = " << fIsWH << endl;
	std::cout << "DEBUG: IsData = " << fIsData << endl;
#endif
}

void AnalysisVH::Initialise()
{
}


void AnalysisVH::InsideLoop()
{
}

void AnalysisVH::Summary()
{
}

AnalysisVH::~AnalysisVH()
{
	if( _data != 0)
	{
		delete _data; //WARNING: Somebody deleting??
	}
	if( fInputParameters != 0)
	{
		delete fInputParameters;
	}
	if( fInput != 0 )
	{
		delete fInput;
	}
}
