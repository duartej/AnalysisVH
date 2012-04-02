#include<sstream>
#include<stdlib.h>
#include<cstring>
#include<iostream>
#include<functional>
#include<algorithm>

#include "AnalysisBase.h"
#include "InputParameters.h"
#include "CutManager.h"
#include "PUWeight.h"
#include "FOManager.h"
#include "WManager.h"

#include "TTree.h"
#include "TH1D.h"
#include "TMath.h"

// To be changed: FIXME
const double kZMass = 91.1876;
//const unsigned int kNMuons = 3; 
const unsigned int kNMuons = 2; 
const unsigned int kWPID   = 24; //Found with TDatabasePDG::Instance()->GetParticle("W+")->PdgCode()
const unsigned int kTauPID = 15; //Found with TDatabasePDG::Instance()->GetParticle("tau-")->PdgCode()



// Prepare analysis Constructor
AnalysisBase::AnalysisBase(TreeManager * data, std::map<LeptonTypes,InputParameters*> ipmap, 
		CutManager * selectioncuts, const unsigned int & finalstate ) :
	CMSAnalysisSelector(data),
	_nLeptons(3), //FIXME: argumento de entrada ? --> No dependera de la SignatureFS
	fIsData(false),
	fIsWH(false),
	fLuminosity(0),
	fFS(999),
	//fLeptonName(0),
	fNGenElectrons(0),
	fNGenMuons(0),
	fNGenLeptons(0),
	fPUWeight(0),
	fFO(0),
	fSF(0),
	_cuttree(0),
	_cutvalue(-1),
	_eventnumber(-1),
	fWasStored(false)
{
	// FIXME: Check that the data is attached to the selector manager
	fLeptonSelection = selectioncuts;
	// Initialize the cuts for the cut manager
	fLeptonSelection->InitialiseCuts(ipmap);

	// Initialize the scale factors // FIXME: Para el InitializeParameters
	fSF = new WManager( WManager::SF );

	// Are in fake sample mode?  // FIXME: Para el InitializeParameters
	if( fLeptonSelection->IsInFakeableMode() ) 
	{
		fFO = new FOManager;
		//fFO = new WManager( WManager::FR );
	}

	
	// Just to use the general values fLuminosity, cross section and so on....
	// it doesn't matters which one pick up
	// The others already can be deleted as they have already used to input the cuts
	int ksize = ipmap.size();
	for(std::map<LeptonTypes,InputParameters*>::iterator ipI = ipmap.begin();
			ipI != ipmap.end(); ++ipI)
	{
		if( ksize == 1 )
		{
			fInputParameters = ipI->second;
			break;
		}

		if( ipI->second != 0 )
		{
			delete ipI->second;
			ipI->second = 0;
		}
		--ksize;
	}

	// The Inputparameters have to be initialized before, just to complete it
	// introducing the set of datasets: 
	//  --> extract filenames: datafilenames_[index]
	std::string filenames( "datafilenames_" );
	// -- Checking is there
	if( fInputParameters == 0 )
	{
		std::cerr << "\033[1;31mAnalysisBase::AnalysisBase ERROR:\033[1;m The 'InputParameters' argument "
			<< "cannot be passed as NULL pointer, initialize first!! Exiting... " << std::endl;
		exit(-1);
	}

	if( fInputParameters->TheNamedString( filenames+"0" ) == 0 )
	{
		std::cerr << "\033[1;31mAnalysisBase::AnalysisBase ERROR:\033[1;m The 'InputParameters' argument "
			<< "must contain a 'datafilenames_0' value!! Exiting..." << std::endl;
		exit(-1);
	}
	std::stringstream istr;
	int id = 0;
	istr << id;
	const char * filename  = 0;
	while( (filename = fInputParameters->TheNamedString(filenames+istr.str())) )
	{
		_datafiles.push_back( filename );
		id++;
		istr << id;
	}

	// Including to the list of fInputs the InputParameter
	TList * inputlist = new TList;
	inputlist->Add(fInputParameters);
	this->SetInputList(inputlist);

	// The signature of the analysis
	fFS = finalstate;
	// And the link to the Gen we want to use
	if( fFS == SignatureFS::_iFSmmm )
	{
		fLeptonType = MUON;
		fLeptonName.push_back("Muon");
	}
	else if( fFS == SignatureFS::_iFSeee )
	{
		fLeptonType = ELECTRON;
		fLeptonName.push_back("Elec");
	}
	else if( fFS == SignatureFS::_iFSmme )
	{
		fLeptonType = MIX2MU1ELE;
		fLeptonName.push_back("Muon");
		fLeptonName.push_back("Elec");
	}
	else if( fFS == SignatureFS::_iFSeem )
	{
		fLeptonType = MIX2ELE1MU;
		fLeptonName.push_back("Muon");
		fLeptonName.push_back("Elec");
	}

	// else if fFS == SignatrueFS::iFSmee --> fGenLeptonIndex = vector(MUON ELECTRON ELECTRON)
	// else ???? Ya deberia estar controlado antes
	// anyway FIXME: meter aqui un exit--->>

	// The tree for cuts
	_cuttree = new TTree("cuts","Last cut used");
	_cuttree->Branch("cuts",&_cutvalue);
	_cuttree->Branch("EventNumber",&_eventnumber);
}

AnalysisBase::~AnalysisBase()
{
	// Checking if already was stored the results,
	// otherwise do it
	if( ! fWasStored )
	{
		this->SaveOutput();
	}

	/*if( fData != 0)
	{
		delete fData; //WARNING: Somebody deleting-- > YES!!
	}*/
	if( fInputParameters != 0)
	{
		delete fInputParameters; 
		fInputParameters=0;
	}
	if( fInput != 0 )
	{
		delete fInput;  // new TList deleted
		fInput = 0;
	}
	if( _cuttree != 0)
	{
		delete _cuttree;
		_cuttree = 0;
	}

	if( fFO != 0 )
	{
		delete fFO;
		fFO = 0;
	}
	
	if( fSF != 0 )
	{
		delete fSF;
		fSF = 0;
	}

}

void AnalysisBase::SaveOutput( const char * outputname )
{
	// Create the output file and fill it
	// Putting the outputfile name per default if not defined
	std::string outputfile;

	if( outputname == 0 )
	{
		// Extract the name of the file and get the last 
		std::string filename( fInputParameters->TheNamedString("datafilenames_0") );
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

		outputfile = "Results/"+std::string(//fInputParameters->TheNamedString("MyAnalysis"))+"_"
			//+
			finalname+".root");
	}
	else
	{
		outputfile = outputname;
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
		li = this->GetInputList();
		lo = this->GetOutputList();
		li->Write();
		lo->Write();
		// And the tree for cuts
		_cuttree->Write();

		histoAnalysis.Close();
	}

	this->fWasStored = true;
}

void AnalysisBase::InitialiseParameters()
{
	InputParameters * ip = this->GetInputParameters();

	// FIXME: I need a method to checked that the cut
	// is really in the configuration file
	//Cuts ---> Reorganized!! See CutManager src
	
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
	else if(fDataName.Contains("Fake"))
	{
		fIsData = true;
	}
	
	// Luminosity
	//--------------------------------
	ip->TheNamedDouble("Luminosity", fLuminosity);
	
#ifdef DEBUGANALYSIS
	std::cout << "DEBUG: IsWH   = " << fIsWH << std::endl;
	std::cout << "DEBUG: IsData = " << fIsData << std::endl;
#endif
	
	// PU Weight
	//----------------------------------------------------------------------------
	//fPUWeight = new PUWeight(fLuminosity, Summer11InTime); //EMCDistribution enum
	// --> FIXME: PONER EN el Input parameters una nueva variable str con el anyo??
	std::string year = ip->TheNamedString("RunPeriod"); //FIXME, Check it is there
	fPUWeight = new PUWeight(fLuminosity, Fall11True, year.c_str());
}


/*const TLorentzVector AnalysisBase::GetTLorentzVector( const int & index) const
{
	std::string name;
	// FIXME: WARNING, Not implemented
	if( fLeptonSelection->GetLeptonType(index) == MUON )
	{
		name = "Muon";
	}
	else
	{
		name = "Elec";
	}

	const std::string px("T_"+name+"_Px");
	const std::string py("T_"+name+"_Py");
	const std::string pz("T_"+name+"_Pz");
	const std::string energy("T_"+name+"_Energy");
std::cout << "ADIOS CARACOLA" << std::endl;

	return TLorentzVector( fData->Get<float>(px.c_str(),index),
			fData->Get<float>(py.c_str(),index),
			fData->Get<float>(pz.c_str(),index),
			fData->Get<float>(energy.c_str(),index)
			);
}*/

// Overloaded function to extract 4-moments of other than leptons
const TLorentzVector AnalysisBase::GetTLorentzVector( const char * name, const int & index) const
{
	std::string namestr = name;
	const std::string px("T_"+namestr+"_Px");
	const std::string py("T_"+namestr+"_Py");
	const std::string pz("T_"+namestr+"_Pz");
	const std::string energy("T_"+namestr+"_Energy");

	return TLorentzVector( fData->Get<float>(px.c_str(),index),
			fData->Get<float>(py.c_str(),index),
			fData->Get<float>(pz.c_str(),index),
			fData->Get<float>(energy.c_str(),index)
			);
}

void AnalysisBase::Summary()
{
	std::cout << std::endl << "[ Analisys::Sumary ]" << std::endl << std::endl;
  
	std::cout << "N. events by process ID:" << std::endl;
	std::cout << "------------------------" << std::endl;
	for(unsigned int i = 0; i < _iNProcesses; i++)
	{
		std::cout << kProcesses[i] << ":\t" << _histos[fHProcess]->GetBinContent(i+1) << " events ("
			<< 100.0*_histos[fHProcess]->GetBinContent(i+1)/_histos[fHProcess]->GetEntries() << "%)" << std::endl;
	}
	std::cout << std::endl << std::endl;
	std::cout << "N. events selected at each stage:" << std::endl;
	std::cout << "---------------------------------" << std::endl;
	for(unsigned int i = 0; i < fNCuts; i++)
	{
		std::cout << _histos[fHEventsPerCut]->GetBinContent(i+1) << " ["
			<< 100.0*_histos[fHEventsPerCut]->GetBinContent(i+1)/_histos[fHEventsPerCut]->GetBinContent(1)
			<< "%] selected events (" << fCutNames[i] << ")" << std::endl;
	}
	std::cout << std::endl << std::endl;
}




//---------------------------------------------------------------------
// Other helper methods
//---------------------------------------------------------------------
//

bool AnalysisBase::IspassHLT() const
{
	//FIXME
	return  true;
	/*// Only data by the moment
	if( ! fIsData )
	{
		return true;
	}

	bool passtrigger = false;
	if( fFS == SignatureFS::_iFSmmm )
	{
		passtrigger = fData->Get<bool>("T_passTriggerDoubleMu");
	}
	else if( fFS == SignatureFS::_iFSeee )
	{
		passtrigger = fData->Get<bool>("T_passTriggerDoubleEl");
	}
	else if( fFS == SignatureFS::_iFSmme || fFS == SignatureFS::_iFSeem )
	{
		passtrigger = fData->Get<bool>("T_passTriggerElMu");
	}
	else if( fFS == SignatureFS::_iFSlll )
	{
		// Note that here we are going to mix DoubleElectron samples with 
		// DoubleMuon Samples: so to avoid double counting:
		// -- 
		passtrigger = fData->Get<bool>("T_passTriggerElMu");
		if( ! passtrigger )
		{
			passtrigger = pass->Get<bool>("T_passTriggerMu");
		}
		if( ! passtrigger )
		{
			passtrigger = pass->Get<bool>("T_passTriggerEl");
		}
	}

	return passtrigger;*/
}


void AnalysisBase::StoresCut(const unsigned int & cut)
{
	_cutvalue = cut;
	_eventnumber = fData->Get<int>("T_Event_EventNumber");
	_cuttree->Fill();
	_cutvalue = -1;
}


void AnalysisBase::FillHistoPerCut(const unsigned int & cut,const double & puw, const unsigned int & fs) 
{
	_histos[fHEventsPerCut]->Fill(cut, puw);
	if(fs == fFS)
	{
		_histos[fHEventsPerCut3Lepton]->Fill(cut, puw);
	}
}

void AnalysisBase::FillGenPlots(const unsigned int & cut, double puw) 
{
	if(fIsWH && fNGenLeptons == _nLeptons) 
	{
		for(unsigned int i = 0; i < fNGenLeptons; i++) 
		{
			fHGenPtLepton[i][cut]->Fill(fGenLepton[i].Pt(), puw);
			fHGenEtaLepton[i][cut]->Fill(fGenLepton[i].Eta(), puw);
		}
	}
}

