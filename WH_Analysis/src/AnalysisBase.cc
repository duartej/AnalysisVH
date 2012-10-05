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
	fIsWZ(false),
	fIsSignal(false),
	fLuminosity(0),
	fRunPeriod(""),
	fFS(999),
	//fLeptonName(0),
	fNGenElectrons(0),
	fNGenMuons(0),
	fNGenLeptons(0),
	fPUWeight(0),
	fFO(0),
	fPO(0),
	fSF(0),
	_cuttree(0),
	_cutweight(1),
	_cutvalue(-1),
	_eventnumber(-1),
	_runnumber(-1),
	_evtlisttree(0),
	_wcharge(0),
	_jetname(""),
	fWasStored(false)
{
	// FIXME: Check that the data is attached to the selector manager
	fLeptonSelection = selectioncuts;
	// Initialize the cuts for the cut manager
	fLeptonSelection->InitialiseCuts(ipmap);
	
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
	
	// Extract the run period
	fRunPeriod = fInputParameters->TheNamedString("RunPeriod");

	// Set jet name
	_jetname = "JetAKPFNoPU";
	if( fRunPeriod.find("2012") != std::string::npos )
	{
		_jetname = "JetAKCHS";
	}
	
	// Initialize the scale factors
	fSF = new WManager( WManager::SF, fRunPeriod );

	// Are in fake sample mode?
	if( fLeptonSelection->IsInFakeableMode() ) 
	{
		// Fake rate Matrix for Z Jets, when proceed
		int iszjetsFRMatrixint = 0;
		fInputParameters->TheNamedInt("FRMatrixZJETS",iszjetsFRMatrixint);
		const bool iszjetsFRMatrix = (bool)iszjetsFRMatrixint;
		fFO = new WManager( WManager::FR, fRunPeriod, iszjetsFRMatrix );
		fPO = new WManager( WManager::PR, fRunPeriod );
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
	_cuttree->Branch("weight",&_cutweight);
	_cuttree->Branch("EventNumber",&_eventnumber);
	_cuttree->Branch("RunNumber",&_runnumber);

	// The tree for Event info
	_evtlisttree = new TTree("evtlist","Event selected info");
	_evtlisttree->Branch("evtinfo",&_evtinfo.run,"run/I:lumi/I:evt/I:channel/I:zmass/D:zlep1pt/D:zlep1eta/D:zlep1phi/D:zlep2pt/D:zlep2eta/D:zlep2phi/D:wmt/D:wleppt/D:wlepeta/D:wlepphi/D:metet/D:metphi/D");
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
	if( _evtlisttree != 0)
	{
		delete _evtlisttree;
		_evtlisttree = 0;
	}

	if( fFO != 0 )
	{
		delete fFO;
		fFO = 0;
	}
	
	if( fPO != 0 )
	{
		delete fPO;
		fPO = 0;
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

		outputfile = "Results/"+std::string(finalname+".root");
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
		// Also the event info tree
		_evtlisttree->Write();

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
	if(fDataName.Contains("WH"))
	{
		fIsWH = true;
	}
	else if(fDataName.Contains("WZ"))
	{
		fIsWZ  = true;
	}
	// FIXED BUG: Using BeginsWith method in order to avoid the MC Fakes
	else if(fDataName.Contains("Data") || fDataName.BeginsWith("Fakes") )
	{
		fIsData = true;
	}
	
	// Luminosity
	//--------------------------------
	ip->TheNamedDouble("Luminosity", fLuminosity);
	
#ifdef DEBUGANALYSIS
	std::cout << "DEBUG: IsWH   = " << fIsWH << std::endl;
	std::cout << "DEBUG: IsWZ   = " << fIsWZ << std::endl;
	std::cout << "DEBUG: IsData = " << fIsData << std::endl;
#endif
	
	// PU Weight
	//----------------------------------------------------------------------------
	EMCDistribution MCdist; 
	if( fRunPeriod == "2011A" )
	{
		MCdist = Summer11;
	}
	else if( fRunPeriod == "2011B" )
	{
		MCdist = Summer11;
	}
	else if( fRunPeriod == "2011" )
	{
		//MCdist = Fall11True;
		MCdist = Fall11;
	}
	else if( fRunPeriod == "2012A" )
	{
		MCdist = Summer12_53X; 
	}
	else if( fRunPeriod == "2012B" )
	{
		MCdist = Summer12_53X;
	}
	else if( fRunPeriod == "2012" )
	{
		MCdist = Summer12_53X;
	}
	else
	{
		std::cerr << "\033[1;31mAnalysisBase::AnalysisBase ERROR:\033[1;m The Run Period '"
			<< fRunPeriod << "' is not recognized" << std::endl;
		exit(-1);
	}
	fPUWeight = new PUWeight(fLuminosity, MCdist, fRunPeriod.c_str());

	// The W-charge if needed (if there are anything is going to return -1 but without
	// touching the initial value 0
	ip->TheNamedInt("WCharge",_wcharge);
}


// Function to extract 4-moments of objects
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
	std::cout << " + Channel evaluated: " << SignatureFS::GetFSID(fFS) << std::endl;
	if( ! fIsData )
	{
		std::cout << " + Scale Factors used: |" << std::endl;
		if( fFS == SignatureFS::_iFSmmm ||
				fFS == SignatureFS::_iFSeem || SignatureFS::_iFSmme )
		{
			std::cout << "                       + MUON: " << fSF->GetFilename(MUON) << std::endl;
		}
		
		if( fFS == SignatureFS::_iFSeee ||
				fFS == SignatureFS::_iFSeem || SignatureFS::_iFSmme )
		{
			std::cout << "                       + ELECTRON: " << fSF->GetFilename(ELECTRON) << std::endl;
		}
	}
	if( fLeptonSelection->IsInFakeableMode() ) 
	{
		std::cout << " + FAKEABLE MODE ENABLED: ";
		int iszjetsFRMatrixint = 0;
		// Fake rate Matrix for Z Jets, when proceed
		fInputParameters->TheNamedInt("FRMatrixZJETS",iszjetsFRMatrixint);
		const bool iszjetsFRMatrix = (bool)iszjetsFRMatrixint;
		if( iszjetsFRMatrix ) 
		{
			std::cout << "Z+Jets Region" << std::endl;
		}
		else
		{
			std::cout << "ttbar Region" << std::endl;
		}
	}
	// FIXME---> SYSTEMATICS MODE ?? -> IsSystematicMode() ??
	std::cout << " ------------------------------------------------- " << std::endl;
	std::cout << std::endl;
  
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
	//return true;
	//Only data by the moment
	if( ! fIsData )
	{
		return true;
	}

	bool passtrigger = false;
	// BUG IN THE LATINO TREE CREATION--- GRIDUI ONLY?? --- Has to be checked
	//if( fRunPeriod.find("2011") != std::string::npos )
	//{
	//       passtrigger = fData->Get<bool>("T_passTriggerDoubleEl");	
	//}
	if( fFS == SignatureFS::_iFSmmm || fFS == SignatureFS::_iFSmme )
	{
		passtrigger = fData->Get<bool>("T_passTriggerDoubleMu");
	}
	else if( fFS == SignatureFS::_iFSeee || fFS == SignatureFS::_iFSeem )
	{
		passtrigger = fData->Get<bool>("T_passTriggerDoubleEl");
	}

	return passtrigger;
}


void AnalysisBase::StoresCut(const unsigned int & cut, const float & weight)
{
	_cutweight = weight;
	_cutvalue = cut;
	_eventnumber = fData->Get<int>("T_Event_EventNumber");
	_runnumber   = fData->Get<int>("T_Event_RunNumber");
	_cuttree->Fill();
	_cutvalue = -1;
	_cutweight = 1;
}

void AnalysisBase::StoresEvtInf(const std::vector<TLorentzVector> & lepton, 
		const int & iZ1, const int & iZ2,
		const int & iW, const double & transversmass, const TLorentzVector & METV)
{
        _evtinfo.run = fData->Get<int>("T_Event_RunNumber");
        _evtinfo.lumi = fData->Get<int>("T_Event_LuminosityBlock");
        _evtinfo.evt  = fData->Get<int>("T_Event_EventNumber");
        _evtinfo.channel = fFS;

        _evtinfo.zmass = (lepton[iZ1]+lepton[iZ2]).M();
        _evtinfo.zlep1pt = lepton[iZ1].Pt();
        _evtinfo.zlep1eta = lepton[iZ1].Eta();
        _evtinfo.zlep1phi = lepton[iZ1].Phi();

        _evtinfo.zlep2pt = lepton[iZ2].Pt();
        _evtinfo.zlep2eta = lepton[iZ2].Eta();
        _evtinfo.zlep2phi = lepton[iZ2].Phi();

        _evtinfo.wmt = transversmass;
        _evtinfo.wleppt = lepton[iW].Pt();
        _evtinfo.wlepeta = lepton[iW].Eta();
        _evtinfo.wlepphi = lepton[iW].Phi();

        _evtinfo.metet = METV.Pt();
        _evtinfo.metphi= METV.Phi();

	_evtlisttree->Fill();
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
	if(fIsSignal && fNGenLeptons == _nLeptons) 
	{
		for(unsigned int i = 0; i < fNGenLeptons; i++) 
		{
			fHGenPtLepton[i][cut]->Fill(fGenLepton[i].Pt(), puw);
			fHGenEtaLepton[i][cut]->Fill(fGenLepton[i].Eta(), puw);
		}
	}
}

// Fake rate estimations
//===============================================================

// Simplistic calculation using full approximations p\sim 1, f->0
double AnalysisBase::GetPPFWeightApprx()
{
	// As we are using the approximation PromptRate=1, then
	// PPF (3,2) = fF0->GetWeight
	// PFF (3,1) = (fFO->GetWeight)^2
	// FFF (3,0) = (fFO->GetWeight)^3
	// Where (N,T) are actually the number of Total leptons and PROMPT leptons. 
	// This equivalence between tight-prompt can be done because of the approximations
	// used. So, each no-tight lepton is weighted in order to get its probability to be
	// fake.
	double puw = 1.0;
	for(unsigned int k = 0; k < fLeptonSelection->GetNAnalysisNoTightLeptons(); ++k)
	{
		const unsigned int i = fLeptonSelection->GetNoTightIndex(k);
		const LeptonTypes ileptontype= fLeptonSelection->GetNoTightLeptonType(k);
		const char * name = 0;
		if( ileptontype == MUON )
		{
			name = "Muon";
		}
		else
		{
			name = "Elec";
		}
		TLorentzVector lvec = this->GetTLorentzVector(name,i);
		const double pt  = lvec.Pt();
		const double eta = lvec.Eta();
		puw *= fFO->GetWeight(ileptontype,pt,eta);
	}

	return puw;
}

// PPF estimation (full calculation)
double AnalysisBase::GetPPFWeight()
{
	// Weighting rules:
	//  PROMPT ESTIMATED:     p(1-f)  for each passing 
	//                        pf      for each failing
	//  FAKE ESTIMATED:       f(1-p)  for each passing
	//                        pf      for each failing
	//  common factor: 1/(p-f)
	const unsigned int ntight = fLeptonSelection->GetNAnalysisTightLeptons();
	const unsigned int nfailing = fLeptonSelection->GetNAnalysisNoTightLeptons();
	std::vector<double> p; // index ordered in tight-noTight
	std::vector<double> f;
	// 1. Tight 
	for(unsigned int k = 0; k < ntight; ++k)
	{
		const unsigned int i = fLeptonSelection->GetTightIndex(k);
		const LeptonTypes ileptontype= fLeptonSelection->GetTightLeptonType(k);
		const char * name = 0;
		if( ileptontype == MUON )
		{
			name = "Muon";
		}
		else
		{
			name = "Elec";
		}
		TLorentzVector lvec = this->GetTLorentzVector(name,i);
		const double pt  = lvec.Pt();
		const double eta = lvec.Eta();
		p.push_back(fPO->GetWeight(ileptontype,pt,eta));
		f.push_back(fFO->GetWeight(ileptontype,pt,eta));
	}
	// 2. NoTight (or failing)
	for(unsigned int k = 0; k < nfailing; ++k)
	{
		const unsigned int i = fLeptonSelection->GetNoTightIndex(k);
		const LeptonTypes ileptontype= fLeptonSelection->GetNoTightLeptonType(k);
		const char * name = 0;
		if( ileptontype == MUON )
		{
			name = "Muon";
		}
		else
		{
			name = "Elec";
		}
		TLorentzVector lvec = this->GetTLorentzVector(name,i);
		const double pt  = lvec.Pt();
		const double eta = lvec.Eta();
		p.push_back( fPO->GetWeight(ileptontype,pt,eta) );
		f.push_back( fFO->GetWeight(ileptontype,pt,eta) );
	}
	
	// Weights applied to each lepton as map-> index: weight
	// Estimated as prompt pass (tight) and fail (no tight)
	std::map<int,double> wPromptPass; 
	std::map<int,double> wPromptFail; 
	// Estimated as fake pass (tight) and fail (no tight)
	std::map<unsigned int,double> wFakePass; 
	std::map<unsigned int,double> wFakeFail; 
	for(unsigned int k = 0; k < ntight; ++k)
	{
		wPromptPass[k] = (1.0/(p[k]-f[k]))*p[k]*(1.0-f[k]);
		wFakePass[k] = (1.0/(p[k]-f[k]))*f[k]*(1.0-p[k]);
	}

	for(unsigned int k = ntight; k < ntight+nfailing; ++k)
	{
		wPromptFail[k] = (1.0/(p[k]-f[k]))*p[k]*f[k];
		wFakeFail[k] = (1.0/(p[k]-f[k]))*f[k]*p[k];
	}

	// The contributions to the PPF estimation: given the observed (tight or no-tight)
	// ordered as 0,1,2,... (see p and f vectors), the three different contributions are
	// calculated assuming the observed i-essim leptons is estimated as P(prompt) or F(fake)
	double PPF = 0;  
	double PFP = 0;
	double FPP = 0;
	switch(ntight)
	{
		case 0: //Nt0
			PPF = wPromptFail[0]*wPromptFail[1]*wFakeFail[2];
			PFP = wPromptFail[0]*wFakeFail[1]*wPromptFail[2];
			FPP = wFakeFail[0]*wPromptFail[1]*wPromptFail[0];
			break;
		case 1: //Nt1
			PPF = wPromptPass[0]*wPromptFail[1]*wFakeFail[2];
			PFP = wPromptPass[0]*wFakeFail[1]*wPromptFail[2];
			FPP = wFakePass[0]*wPromptFail[1]*wPromptFail[2];
			break;
		case 2: //Nt2
			PPF = wPromptPass[0]*wPromptPass[1]*wFakeFail[2];
			PFP = wPromptPass[0]*wFakePass[1]*wPromptFail[2];
			FPP = wFakePass[0]*wPromptPass[1]*wPromptFail[2];
			break;
		case 3: //Nt3
			PPF = wPromptPass[0]*wPromptPass[1]*wFakePass[2];
			PFP = wPromptPass[0]*wFakePass[1]*wPromptPass[2];
			FPP = wFakePass[0]*wPromptPass[1]*wPromptPass[2];
			break;
		default:
			std::cerr << "\033[1;31mAnalysisBase::AnalysisBase::GetPPFWeight ERROR:\033[1;m" 
				<<" The 'Nt=" << ntight << "' contribution is not coded."
				<< " Exiting... " << std::endl;
			exit(-1);
	}
	
	return PPF+PFP+FPP;
}

// PFF estimation (full calculation)
double AnalysisBase::GetPFFWeight()
{
	// Weighting rules:
	//  PROMPT ESTIMATED:     p(1-f)  for each passing 
	//                        pf      for each failing
	//  FAKE ESTIMATED:       f(1-p)  for each passing
	//                        pf      for each failing
	//  common factor: 1/(p-f)
	const unsigned int ntight = fLeptonSelection->GetNAnalysisTightLeptons();
	const unsigned int nfailing = fLeptonSelection->GetNAnalysisNoTightLeptons();
	std::vector<double> p; // index ordered in tight-noTight
	std::vector<double> f;
	// 1. Tight 
	for(unsigned int k = 0; k < ntight; ++k)
	{
		const unsigned int i = fLeptonSelection->GetTightIndex(k);
		const LeptonTypes ileptontype= fLeptonSelection->GetTightLeptonType(k);
		const char * name = 0;
		if( ileptontype == MUON )
		{
			name = "Muon";
		}
		else
		{
			name = "Elec";
		}
		TLorentzVector lvec = this->GetTLorentzVector(name,i);
		const double pt  = lvec.Pt();
		const double eta = lvec.Eta();
		p.push_back(fPO->GetWeight(ileptontype,pt,eta));
		f.push_back(fFO->GetWeight(ileptontype,pt,eta));
	}
	// 2. NoTight (or failing)
	for(unsigned int k = 0; k < nfailing; ++k)
	{
		const unsigned int i = fLeptonSelection->GetNoTightIndex(k);
		const LeptonTypes ileptontype= fLeptonSelection->GetNoTightLeptonType(k);
		const char * name = 0;
		if( ileptontype == MUON )
		{
			name = "Muon";
		}
		else
		{
			name = "Elec";
		}
		TLorentzVector lvec = this->GetTLorentzVector(name,i);
		const double pt  = lvec.Pt();
		const double eta = lvec.Eta();
		p.push_back( fPO->GetWeight(ileptontype,pt,eta) );
		f.push_back( fFO->GetWeight(ileptontype,pt,eta) );
	}
	
	// Weights applied to each lepton as map-> index: weight
	// Estimated as prompt pass (tight) and fail (no tight)
	std::map<int,double> wPromptPass; 
	std::map<int,double> wPromptFail; 
	// Estimated as fake pass (tight) and fail (no tight)
	std::map<unsigned int,double> wFakePass; 
	std::map<unsigned int,double> wFakeFail; 
	for(unsigned int k = 0; k < ntight; ++k)
	{
		wPromptPass[k] = (1.0/(p[k]-f[k]))*p[k]*(1.0-f[k]);
		wFakePass[k] = (1.0/(p[k]-f[k]))*f[k]*(1.0-p[k]);
	}

	for(unsigned int k = ntight; k < ntight+nfailing; ++k)
	{
		wPromptFail[k] = (1.0/(p[k]-f[k]))*p[k]*f[k];
		wFakeFail[k] = (1.0/(p[k]-f[k]))*f[k]*p[k];
	}

	// The contributions to the PPF estimation: given the observed (tight or no-tight)
	// ordered as 0,1,2,... (see p and f vectors), the three different contributions are
	// calculated assuming the observed i-essim leptons is estimated as P(prompt) or F(fake)
	double PFF = 0;  
	double FPF = 0;
	double FFP = 0;
	switch(ntight)
	{
		case 0: //Nt0
			PFF = wPromptFail[0]*wFakeFail[1]*wFakeFail[2];
			FPF = wFakeFail[0]*wPromptFail[1]*wFakeFail[2];
			FFP = wFakeFail[0]*wFakeFail[1]*wPromptFail[0];
			break;
		case 1: //Nt1
			PFF = wPromptPass[0]*wFakeFail[1]*wFakeFail[2];
			FPF = wFakePass[0]*wPromptFail[1]*wFakeFail[2];
			FFP = wFakePass[0]*wFakeFail[1]*wPromptFail[2];
			break;
		case 2: //Nt2
			PFF = wPromptPass[0]*wFakePass[1]*wFakeFail[2];
			FPF = wFakePass[0]*wPromptPass[1]*wFakeFail[2];
			FFP = wFakePass[0]*wFakePass[1]*wPromptFail[2];
			break;
		case 3: //Nt3
			PFF = wPromptPass[0]*wFakePass[1]*wFakePass[2];
			FPF = wFakePass[0]*wPromptPass[1]*wFakePass[2];
			FFP = wFakePass[0]*wFakePass[1]*wPromptPass[2];
			break;
		default:
			std::cerr << "\033[1;31mAnalysisBase::AnalysisBase::GetPPFWeight ERROR:\033[1;m" 
				<<" The 'Nt=" << ntight << "' contribution is not coded."
				<< " Exiting... " << std::endl;
			exit(-1);
	}
	
	return PFF+FPF+FFP;
}

// FFF estimation (full calculation)
double AnalysisBase::GetFFFWeight()
{
	// Weighting rules:
	//  PROMPT ESTIMATED:     p(1-f)  for each passing 
	//                        pf      for each failing
	//  FAKE ESTIMATED:       f(1-p)  for each passing
	//                        pf      for each failing
	//  common factor: 1/(p-f)
	const unsigned int ntight = fLeptonSelection->GetNAnalysisTightLeptons();
	const unsigned int nfailing = fLeptonSelection->GetNAnalysisNoTightLeptons();
	std::vector<double> p; // index ordered in tight-noTight
	std::vector<double> f;
	// 1. Tight 
	for(unsigned int k = 0; k < ntight; ++k)
	{
		const unsigned int i = fLeptonSelection->GetTightIndex(k);
		const LeptonTypes ileptontype= fLeptonSelection->GetTightLeptonType(k);
		const char * name = 0;
		if( ileptontype == MUON )
		{
			name = "Muon";
		}
		else
		{
			name = "Elec";
		}
		TLorentzVector lvec = this->GetTLorentzVector(name,i);
		const double pt  = lvec.Pt();
		const double eta = lvec.Eta();
		p.push_back(fPO->GetWeight(ileptontype,pt,eta));
		f.push_back(fFO->GetWeight(ileptontype,pt,eta));
	}
	// 2. NoTight (or failing)
	for(unsigned int k = 0; k < nfailing; ++k)
	{
		const unsigned int i = fLeptonSelection->GetNoTightIndex(k);
		const LeptonTypes ileptontype= fLeptonSelection->GetNoTightLeptonType(k);
		const char * name = 0;
		if( ileptontype == MUON )
		{
			name = "Muon";
		}
		else
		{
			name = "Elec";
		}
		TLorentzVector lvec = this->GetTLorentzVector(name,i);
		const double pt  = lvec.Pt();
		const double eta = lvec.Eta();
		p.push_back( fPO->GetWeight(ileptontype,pt,eta) );
		f.push_back( fFO->GetWeight(ileptontype,pt,eta) );
	}
	
	// Weights applied to each lepton as map-> index: weight
	// Estimated as prompt pass (tight) and fail (no tight)
	std::map<int,double> wPromptPass; 
	std::map<int,double> wPromptFail; 
	// Estimated as fake pass (tight) and fail (no tight)
	std::map<unsigned int,double> wFakePass; 
	std::map<unsigned int,double> wFakeFail; 
	for(unsigned int k = 0; k < ntight; ++k)
	{
		wPromptPass[k] = (1.0/(p[k]-f[k]))*p[k]*(1.0-f[k]);
		wFakePass[k] = (1.0/(p[k]-f[k]))*f[k]*(1.0-p[k]);
	}

	for(unsigned int k = ntight; k < ntight+nfailing; ++k)
	{
		wPromptFail[k] = (1.0/(p[k]-f[k]))*p[k]*f[k];
		wFakeFail[k] = (1.0/(p[k]-f[k]))*f[k]*p[k];
	}

	// The contributions to the PPF estimation: given the observed (tight or no-tight)
	// ordered as 0,1,2,... (see p and f vectors), the three different contributions are
	// calculated assuming the observed i-essim leptons is estimated as P(prompt) or F(fake)
	double FFF = 0;  
	switch(ntight)
	{
		case 0: //Nt0
			FFF = wFakeFail[0]*wFakeFail[1]*wFakeFail[2];
			break;
		case 1: //Nt1
			FFF = wFakePass[0]*wFakeFail[1]*wFakeFail[2];
			break;
		case 2: //Nt2
			FFF = wFakePass[0]*wFakePass[1]*wFakeFail[2];
			break;
		case 3: //Nt3
			FFF = wFakePass[0]*wFakePass[1]*wFakePass[2];
			break;
		default:
			std::cerr << "\033[1;31mAnalysisBase::AnalysisBase::GetPPFWeight ERROR:\033[1;m" 
				<<" The 'Nt=" << ntight << "' contribution is not coded."
				<< " Exiting... " << std::endl;
			exit(-1);
	}
	
	return FFF;
}

// PPP estimation (full calculation)
double AnalysisBase::GetPPPWeight()
{
	// Weighting rules:
	//  PROMPT ESTIMATED:     p(1-f)  for each passing 
	//                        pf      for each failing
	//  FAKE ESTIMATED:       f(1-p)  for each passing
	//                        pf      for each failing
	//  common factor: 1/(p-f)
	const unsigned int ntight = fLeptonSelection->GetNAnalysisTightLeptons();
	const unsigned int nfailing = fLeptonSelection->GetNAnalysisNoTightLeptons();
	std::vector<double> p; // index ordered in tight-noTight
	std::vector<double> f;
	// 1. Tight 
	for(unsigned int k = 0; k < ntight; ++k)
	{
		const unsigned int i = fLeptonSelection->GetTightIndex(k);
		const LeptonTypes ileptontype= fLeptonSelection->GetTightLeptonType(k);
		const char * name = 0;
		if( ileptontype == MUON )
		{
			name = "Muon";
		}
		else
		{
			name = "Elec";
		}
		TLorentzVector lvec = this->GetTLorentzVector(name,i);
		const double pt  = lvec.Pt();
		const double eta = lvec.Eta();
		p.push_back(fPO->GetWeight(ileptontype,pt,eta));
		f.push_back(fFO->GetWeight(ileptontype,pt,eta));
	}
	// 2. NoTight (or failing)
	for(unsigned int k = 0; k < nfailing; ++k)
	{
		const unsigned int i = fLeptonSelection->GetNoTightIndex(k);
		const LeptonTypes ileptontype= fLeptonSelection->GetNoTightLeptonType(k);
		const char * name = 0;
		if( ileptontype == MUON )
		{
			name = "Muon";
		}
		else
		{
			name = "Elec";
		}
		TLorentzVector lvec = this->GetTLorentzVector(name,i);
		const double pt  = lvec.Pt();
		const double eta = lvec.Eta();
		p.push_back( fPO->GetWeight(ileptontype,pt,eta) );
		f.push_back( fFO->GetWeight(ileptontype,pt,eta) );
	}
	
	// Weights applied to each lepton as map-> index: weight
	// Estimated as prompt pass (tight) and fail (no tight)
	std::map<int,double> wPromptPass; 
	std::map<int,double> wPromptFail; 
	// Estimated as fake pass (tight) and fail (no tight)
	std::map<unsigned int,double> wFakePass; 
	std::map<unsigned int,double> wFakeFail; 
	for(unsigned int k = 0; k < ntight; ++k)
	{
		wPromptPass[k] = (1.0/(p[k]-f[k]))*p[k]*(1.0-f[k]);
		wFakePass[k] = (1.0/(p[k]-f[k]))*f[k]*(1.0-p[k]);
	}

	for(unsigned int k = ntight; k < ntight+nfailing; ++k)
	{
		wPromptFail[k] = (1.0/(p[k]-f[k]))*p[k]*f[k];
		wFakeFail[k] = (1.0/(p[k]-f[k]))*f[k]*p[k];
	}

	// The contributions to the PPF estimation: given the observed (tight or no-tight)
	// ordered as 0,1,2,... (see p and f vectors), the three different contributions are
	// calculated assuming the observed i-essim leptons is estimated as P(prompt) or F(fake)
	double PPP = 0;  
	switch(ntight)
	{
		case 0: //Nt0
			PPP = wPromptFail[0]*wPromptFail[1]*wPromptFail[2];
			break;
		case 1: //Nt1
			PPP = wPromptPass[0]*wPromptFail[1]*wPromptFail[2];
			break;
		case 2: //Nt2
			PPP = wPromptPass[0]*wPromptPass[1]*wPromptFail[2];
			break;
		case 3: //Nt3
			PPP = wPromptPass[0]*wPromptPass[1]*wPromptPass[2];
			break;
		default:
			std::cerr << "\033[1;31mAnalysisBase::AnalysisBase::GetPPFWeight ERROR:\033[1;m" 
				<<" The 'Nt=" << ntight << "' contribution is not coded."
				<< " Exiting... " << std::endl;
			exit(-1);
	}
	
	return PPP;
}
