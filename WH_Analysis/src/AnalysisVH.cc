#include<sstream>
#include<stdlib.h>
#include<cstring>
#include<iostream>

#include "AnalysisVH.h"
#include "InputParameters.h"
#include "CutManager.h"
#include "PUWeight.h"

#include "TTree.h"
#include "TH1F.h"
#include "TMath.h"

// To be changed: FIXME
const double kZMass = 91.1876;
//const unsigned int kNMuons = 3; 
const unsigned int kNMuons = 2; 
const unsigned int kWPID   = 24; //Found with TDatabasePDG::Instance()->GetParticle("W+")->PdgCode()
const unsigned int kTauPID = 15; //Found with TDatabasePDG::Instance()->GetParticle("tau-")->PdgCode()


// Prepare analysis Constructor
AnalysisVH::AnalysisVH(TreeManager * data, InputParameters * ip, 
		CutManager * selectioncuts, TTree * tree ) : // Tree : TO BE DEPRECATED
	CMSAnalysisSelector(data),
	_nLeptons(3), //FIXME: argumento de entrada
	fIsData(false),
	fIsWH(false),
	fLuminosity(0),
	fPUWeight(0),
	_tree(0)
{
	// FIXME: Check that the data is attached to the selector manager
	fLeptonSelection = selectioncuts;

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

AnalysisVH::~AnalysisVH()
{
	/*if( fData != 0)
	{
		delete fData; //WARNING: Somebody deleting-- > YES!!
	}*/
	if( fInputParameters != 0)
	{
		delete fInputParameters;
	}
	if( fInput != 0 )
	{
		delete fInput;  // new TList deleted
	}
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
	fLeptonSelection->SetPtMinCuts(*cuts);
	cuts->clear();
	
	ip->TheNamedDouble("MaxAbsEta", dummy);
	cuts->push_back(dummy);
	fLeptonSelection->SetABSEtaMaxCuts(*cuts);
	cuts->clear();

	//   - IP and DeltaZ of track associated with muon w.r.t PV
	ip->TheNamedDouble("MaxMuIP2DInTrackR1", dummy);
	cuts->push_back(dummy);
	fLeptonSelection->SetUndefCuts(*cuts,CutManager::kMaxMuIP2DInTrackR1);
	cuts->clear();

	ip->TheNamedDouble("MaxMuIP2DInTrackR2", dummy);
	cuts->push_back(dummy);
	fLeptonSelection->SetUndefCuts(*cuts,CutManager::kMaxMuIP2DInTrackR2);
	cuts->clear();

	ip->TheNamedDouble("MaxDeltaZMu", dummy );
	cuts->push_back(dummy);
	fLeptonSelection->SetUndefCuts(*cuts,CutManager::kMaxDeltaZMu);
	cuts->clear();
	
	//   - Isolation: (PTtraks + ETcalo)/PTmuon
	ip->TheNamedDouble("MaxPTIsolationR1",dummy);
	cuts->push_back(dummy);
	ip->TheNamedDouble("MaxPTIsolationR2",dummy);
	cuts->push_back(dummy);
	ip->TheNamedDouble("MaxPTIsolationR3",dummy);
	cuts->push_back(dummy);
	ip->TheNamedDouble("MaxPTIsolationR4",dummy);
	cuts->push_back(dummy);
	ip->TheNamedDouble("MaxIsoMu", dummy);  // OBSOLETE--> Now in regions
	cuts->push_back(dummy);
	fLeptonSelection->SetIsoCuts(*cuts);
	cuts->clear();
	
	//   - Max DeltaR between muons
	ip->TheNamedDouble("MaxDeltaRMuMu",dummy);
	cuts->push_back(dummy);
	fLeptonSelection->SetUndefCuts(*cuts,CutManager::kMaxDeltaRMuMu);
	cuts->clear();
	
	//   - Z mass window
	double deltazmass=0;
	ip->TheNamedDouble("DeltaZMass", deltazmass);
	cuts->push_back( kZMass - deltazmass );
	fLeptonSelection->SetUndefCuts(*cuts,CutManager::kMinZMass);
	cuts->clear();
	cuts->push_back( kZMass + deltazmass );
	fLeptonSelection->SetUndefCuts(*cuts,CutManager::kMaxZMass);
	cuts->clear();
	
	//   - Min MET of the event
	ip->TheNamedDouble("MinMET", dummy);
	cuts->push_back(dummy);
	fLeptonSelection->SetEventCuts(*cuts);
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
	std::cout << "DEBUG: IsWH   = " << fIsWH << std::endl;
	std::cout << "DEBUG: IsData = " << fIsData << std::endl;
#endif
}

void AnalysisVH::Initialise()
{
	// PU Weight
	//----------------------------------------------------------------------------
	fPUWeight = new PUWeight(fLuminosity, Summer11InTime); //EMCDistribution enum

	// Selection cuts
  	//----------------------------------------------------------------------------
  
	// Histograms
	//----------------------------------------------------------------------------
	// Process ID
	_histos[fHProcess] = CreateH1D("fHProcess", "Proccess ID", _iNProcesses, 0, _iNProcesses);
	for(unsigned int i = 0; i < _iNProcesses; i++)
	{
		_histos[fHProcess]->GetXaxis()->SetBinLabel(i+1,kProcesses[i]);
	}
	
	// Final state from generation (incl. taus)
	_histos[fHGenFinalState] = CreateH1D("fHGenFinalState", "Final State (incl. #tau)", 
			_iFStotal, 0, _iFStotal);
	for(unsigned int i = 0; i < _iFStotal; i++)
	{
		_histos[fHGenFinalState]->GetXaxis()->SetBinLabel(i+1,kFinalStates[i]);
	}
	_histos[fHGenFinalState]->Sumw2();

	// Final state from generation (no taus)
	_histos[fHGenFinalStateNoTaus] = CreateH1D("fHGenFinalStateNoTaus", "Final State (no #tau)", 
			_iFStotal, 0, _iFStotal);
	for(unsigned int i = 0; i < _iFStotal; i++)
	{
		_histos[fHGenFinalStateNoTaus]->GetXaxis()->SetBinLabel(i+1,kFinalStates[i]);
	}
	_histos[fHGenFinalStateNoTaus]->Sumw2();

	// Generated muons coming from a W
	_histos[fHNGenWMuons] = CreateH1D("fHNGenWMuons", "N Gen #mu from W", 5, -0.5, 4.5);
	
	// PT and Eta of most energetic gen muon from W or tau
	for(unsigned int i = 0; i < _nLeptons; i++) 
	{
		for (unsigned int j = 0; j < _iNCuts; j++) 
		{
			TString ptname  = Form("fHGenPtMu%i_%i", i+1,j);
			TString pttitle = Form("P_{T} #mu_{gen}^{%i} from W (#tau)", i+1);
			TString etaname  = Form("fHGenEtaMu%i_%i", i+1,j);
			TString etatitle = Form("#eta #mu_{gen}^{%i} from W (#tau)", i+1);
			fHGenPtMu[i][j]  = CreateH1D(ptname,  pttitle, 150, 0, 150);
			fHGenEtaMu[i][j] = CreateH1D(etaname, etatitle, 100, -5, 5);
		}
	}


	// Events passing every cut
	_histos[fHEventsPerCut] = CreateH1D("fHEventsPerCut", "Events passing each cut", 
			_iNCuts, 0, _iNCuts);
	for (unsigned int i = 0; i < _iNCuts; i++)
	{
		_histos[fHEventsPerCut]->GetXaxis()->SetBinLabel(i+1,kCutNames[i]);
	}
  
	// Events passing every cut that are 3 mu from gen
	_histos[fHEventsPerCut3Mu] = CreateH1D("fHEventsPerCut3Mu", "Events passing each cut that are 3 mu from gen", 
			_iNCuts, 0, _iNCuts);
	for(unsigned int i = 0; i < _iNCuts; i++)
	{
		_histos[fHEventsPerCut3Mu]->GetXaxis()->SetBinLabel(i+1,kCutNames[i]);
	}

	// Reconstructed muons in the event
	_histos[fHNRecoMuons] = CreateH1D("fHNRecoMuons", "Reconstructed #mu", 
			10, -0.5, 9.5);
	
	// Muons passing the basic selection
	_histos[fHNSelectedMuons] = CreateH1D("fHNSelectedMuons", "Selected #mu", 
			       10, -0.5, 9.5);

	// Selected Muons close to the PV
	_histos[fHNSelectedPVMuons] = CreateH1D("fHNSelectedPVMuons", 
			"Selected #mu close to PV", 10, -0.5, 9.5);
	// Selected Isolated Muons
	_histos[fHNSelectedIsoMuons] = CreateH1D("fHNSelectedIsoMuons", 
			"Selected Isolated #mu", 10, -0.5, 9.5);
	// Selected Isolated Good Muons
	_histos[fHNSelectedIsoGoodMuons] = CreateH1D("fHNSelectedIsoGoodMuons", 
			"Selected good Isolated #mu", 10, -0.5, 9.5);
	
	// Pt and eta of first/second/third good isolated muon
	for (unsigned int i = 0; i < 3; i++) 
	{
		TString ptname  = Form("fHPtMu%i", i+1);
		TString pttitle = Form("P_{T}^{#mu_{%i}}", i+1);
		TString etaname  = Form("fHEtaMu%i", i+1);
		TString etatitle = Form("#eta^{#mu_{%i}}", i+1);
		TString drname  = Form("fHDeltaRGenRecoMu%i", i+1);
		TString drtitle = Form("#Delta R for #mu %i", i+1);
		fHPtMu[i]  = CreateH1D(ptname,  pttitle, 150, 0, 150);
		fHEtaMu[i] = CreateH1D(etaname, etatitle, 100, -5, 5);
		fHDeltaRGenRecoMu[i] = CreateH1D(drname, drtitle, 150, 0, 5);
	}
	
	//Smallest DeltaR between 2 opp. sign leptons
	fHMinDeltaRMuMu = CreateH1D("fHMinDeltaRMuMu", "Smallest #Delta R_{#mu#mu}",
			125, 0, 5);
	//Smallest DeltaR between 2 opp. sign leptons
	fHMaxDeltaRMuMu = CreateH1D("fHMaxDeltaRMuMu", "Largest #Delta R_{#mu#mu}",
			125, 0, 5);
	
	//Smallest DeltaPhi between 2 opp. sign leptons
	fHMinDeltaPhiMuMu = CreateH1D("fHMinDeltaPhiMuMu", "Smallest #Delta #phi_{#mu#mu}",
			120, 0, TMath::Pi());
	//Largest DeltaPhi between 2 opp. sign leptons
	fHMaxDeltaPhiMuMu = CreateH1D("fHMaxDeltaPhiMuMu", "Largest #Delta #phi_{#mu#mu}",
			120, 0, TMath::Pi());
	
	// Selected Isolated Good Muons
	fHMuonCharge = CreateH1D("fHMuonCharge", "#Sum q_{#mu}", 7, -3.5, 3.5);
		
	// Invariant mass of leptons supposedly from H  
	fHHInvMass = CreateH1D("fHHInvMass", "M^{inv.}_{#mu#mu}", 150, 0, 150);
	
	// Invariant mass of leptons in/out of Z peak
	fHZInvMass = CreateH1D("fHZInvMass", "M^{inv.}_{#mu#mu}",150, 0, 150);
	
	// Missing ET after inv mass cut
	fHMET = CreateH1D("fHMET", "MET",160, 0, 160);
	
}


void AnalysisVH::InsideLoop()
{
#ifdef DEBUGANALYSIS
	std::cout << "========================================================" << std::endl;
	std::cout << "New event" << std::endl;
#endif
	// Get PU Weight
	//----------------------------------------------------------------------
	double puw(1);
	if(!fIsData)
	{
		puw = fPUWeight->GetWeight(fData->GetEventnPU());
	}

	// Generation studies
	//----------------------------------------------------------------------
	unsigned int fsTaus = _iFSunknown;
	unsigned int fsNTau = _iFSunknown;
	fNGenElectrons = 0; //Number of generated electrons from W or tau
	fNGenMuons = 0;     //Number of generated muons from W or tau
	if(fIsWH) 
	{
		// + Classify by leptonic final state (taus undecayed)
		unsigned int nelecsfromW = fData->GetGenElecSt3PID()->size();
		unsigned int nmusfromW = fData->GetGenMuonSt3PID()->size();
		unsigned int ntausfromW = fData->GetGenTauSt3PID()->size();
		
		_histos[fHNGenWMuons]->Fill(nmusfromW,puw); 
		fsTaus = GetFSID(nelecsfromW, nmusfromW, ntausfromW);
		_histos[fHGenFinalState]->Fill(fsTaus, puw);
		
#ifdef DEBUGANALYSIS
		std::cout << "DEBUG: W->e/mu/tau " << nelecsfromW 
			<< "/" << nmusfromW << "/" << ntausfromW << std::endl;
		std::cout << "DEBUG: fsTaus --> " << fsTaus << std::endl;
#endif
		// + Classify by leptonic final state (no taus)
		for(unsigned int i = 0; i < fData->GetGenElecMPID()->size(); i++) 
		{
#ifdef DEBUGANALYSIS
			std::cout << "DEBUG: Elec [" << i << "/" << fData->GetGenElecMPID()->size() << "]" 
				<< " Status: " << fData->GetGenElecMSt()->at(i)
				<< " Mother: " << fData->GetGenElecMPID()->at(i)
				<< " E: " << fData->GetGenElecEnergy()->at(i)
				<< " Px: " << fData->GetGenElecPx()->at(i)
				<< std::endl;
#endif
			//I have seen that electrons from taus have status 2 and are not replicated
			unsigned int mpidel  = TMath::Abs(fData->GetGenElecMPID()->at(i));
			if ( mpidel == kWPID || mpidel == kTauPID) 
			{
				fNGenElectrons++;
#ifdef DEBUGANALYSIS
				std::cout << "DEBUG:     Good!" << std::endl;
#endif
			}
		}
		int igenmuons[3] = {-1, -1, -1}; //Index of generated muons from W or tau
		for (unsigned int i = 0; i < fData->GetGenMuonMPID()->size(); ++i) 
		{
#ifdef DEBUGANALYSIS
			std::cout << "DEBUG: Muon [" << i << "/" << fData->GetGenMuonMPID()->size() << "]" 
				<< " Status: " << fData->GetGenMuonMSt()->at(i)
				<< " Mother: " << fData->GetGenMuonMPID()->at(i)
				<< " E: " << fData->GetGenMuonEnergy()->at(i)
				<< " Px: " << fData->GetGenMuonPx()->at(i)
				<< std::endl;
#endif
			//I have seen that muons from taus have status 2 and are not replicated
			unsigned int mpid  = TMath::Abs(fData->GetGenMuonMPID()->at(i));
			if ( mpid == kWPID || mpid == kTauPID) 
			{
				igenmuons[fNGenMuons] = i;
				fNGenMuons++;
#ifdef DEBUGANALYSIS
				std::cout << "DEBUG:     Good! " << fNGenMuons << std::endl;
#endif
			}
		}
		fsNTau = GetFSID(fNGenElectrons, fNGenMuons, 3-fNGenMuons-fNGenElectrons);
		_histos[fHGenFinalStateNoTaus]->Fill(fsNTau, puw);
		
#ifdef DEBUGANALYSIS
		std::cout << "DEBUG: W->e/mu/tau " <<  fNGenElectrons
			<< "/" << fNGenMuons << "/" << (3-fNGenMuons-fNGenElectrons) << std::endl;
		std::cout << "DEBUG: fsNTau --> " << fsNTau << std::endl;
#endif
		//   Sort by energy
		if(fNGenMuons == 3)  // FIXME:: PORQUE SOLO 3??
		{
			std::map<float,TLorentzVector> vOrder;
			std::vector<TLorentzVector> * vGenMuon = new std::vector<TLorentzVector>;
			for(unsigned int i = 0; i < fNGenMuons; i++) 
			{
				vGenMuon->push_back( TLorentzVector(fData->GetGenMuonPx()->at(igenmuons[i]), 
							fData->GetGenMuonPy()->at(igenmuons[i]), 
							fData->GetGenMuonPz()->at(igenmuons[i]),
							fData->GetGenMuonEnergy()->at(igenmuons[i])) );
				vOrder[vGenMuon->back().Pt()] = vGenMuon->back();
			}

			fGenMuon = new std::vector<TLorentzVector>;
			for(std::vector<TLorentzVector>::reverse_iterator it = vGenMuon->rbegin(); 
					it != vGenMuon->rend(); ++it)
			{
				fGenMuon->push_back( *it );
			}
			delete vGenMuon;
			vGenMuon = 0;
			
#ifdef DEBUGANALYSIS
			for (unsigned int i = 0; i < fNGenMuons; ++i) 
			{
				std::cout << "[" << i << "] <- [" << igenmuons[i] 
					<< "] PT = " << (*fGenMuon)[i].Pt() 
					<< " - ET = " << fData->GetGenMuonEnergy()->at(igenmuons[i]) 
					<< std::endl;
			}      
#endif
			FillGenPlots(_iAllEvents,puw);
			
			// Freeing memory
			delete fGenMuon;
			fGenMuon = 0;
		}
	}
	
	// All events
	//------------------------------------------------------------------
	FillHistoPerCut(_iAllEvents, puw, fsNTau);
  
	// Proccess ID
	//------------------------------------------------------------------
	int procn = _iOther;
	const int processID = fData->GetEventprocessID();
	if(processID >= 0 && processID <= 4)  //ZJets
	{
		procn = _iVarious;
	}
	else if(processID == 22)  //ZZ
	{
		procn = _iZZ;
	}
	else if (processID == 23)  //WZ
	{
		procn = _iWZ;
	}
	else if (processID == 24)  //ZH
	{   
		procn = _iZH; 
	}
	else if (processID == 25)  //WW
	{
		procn = _iWW; 
	}
	else if (processID == 26)  //WH
	{
		procn = _iWH;
	}
	else if (processID >= 120 && processID <= 123 ) //ttH
	{
		procn = _ittH;
	}
	_histos[fHProcess]->Fill(procn);

	if(fIsWH && (procn != _iWH || fsNTau != _iFSmmm))
	{
		return;
	}
	
	FillHistoPerCut(_iIsWH, puw, fsNTau);

	// HLT: TBD...
	//------------------------------------------------------------------
	FillHistoPerCut(_iHLT, puw, fsNTau);

	// Vertex cut (Event stuff)-- OBSOLETE (implemented per default) SURE?
	//int iGoodVertex = GoodVertex();
	int iGoodVertex = 0; // First one
	//fLeptonSelection->PassEventsCuts();
	//if( iGoodVertex < 0)
	//{
	//	return;
	//}
	FillHistoPerCut(_iGoodVertex, puw, fsNTau);
	
	// Muon selection
	//------------------------------------------------------------------
	//
	//this->SetGoodVertexIndex(iGoodVertex);
	
	// Store the number of reconstructed muons without any filter
	_histos[fHNRecoMuons]->Fill(fData->GetMuonPx()->size());
	
	// (1) Basic selection
	//--------------------
	unsigned int nSelectedMuons = fLeptonSelection->GetNBasicLeptons();
	_histos[fHNSelectedMuons]->Fill(nSelectedMuons,puw);

	if(nSelectedMuons < _nLeptons)
	{
		return;
	}
	
	FillHistoPerCut(_iHas2Leptons, puw, fsNTau);
	
	// (2) PV selection
	//--------------------
	unsigned int nSelectedPVMuons = fLeptonSelection->GetNLeptonsCloseToPV();
	_histos[fHNSelectedPVMuons]->Fill(nSelectedPVMuons,puw); 
	
	if(nSelectedPVMuons < _nLeptons)
	{
		return;
	}

	FillHistoPerCut(_iHas2PVLeptons, puw, fsNTau);


	// (3) Isolated muons
	//--------------------
	unsigned int nSelectedIsoMuons = fLeptonSelection->GetNIsoLeptons();
	_histos[fHNSelectedIsoMuons]->Fill(nSelectedIsoMuons,puw);  
	
	if(nSelectedIsoMuons < _nLeptons)
	{
		return;
	}
	
	FillHistoPerCut(_iHas2IsoLeptons, puw, fsNTau);
	
	// (4) Isolated good muons --> Identification !!!
	//--------------------
	unsigned int nSelectedIsoGoodMuons = fLeptonSelection->GetNIsoGoodLeptons();
	fHNSelectedIsoGoodMuons->Fill(nSelectedIsoGoodMuons,puw);
	
	if(nSelectedIsoGoodMuons < _nLeptons)
	{
		return;
	}
	FillHistoPerCut(_iHas2IsoGoodLeptons, puw, fsNTau);
	
}

void AnalysisVH::Summary()
{
	std::cout << std::endl << "[ AnalisysVH::Sumary ]" << std::endl << std::endl;
  
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
	for(unsigned int i = 0; i < _iNCuts; i++)
	{
		std::cout << _histos[fHEventsPerCut]->GetBinContent(i+1) << " ["
			<< "%] selected events (" << kCutNames[i] << ")" << std::endl;
	}
	std::cout << std::endl << std::endl;
}




//---------------------------------------------------------------------
// Other helper methods
//---------------------------------------------------------------------
//

void AnalysisVH::FillHistoPerCut(const ECutLevel & cut,const double & puw, const unsigned int & fs) 
{
	_histos[fHEventsPerCut]->Fill(cut, puw);
	if (fs == _iFSmmm)
	{
		_histos[fHEventsPerCut3Mu]->Fill(cut, puw);
	}
}

void AnalysisVH::FillGenPlots(ECutLevel cut, double puw) 
{
	if (fIsWH && fNGenMuons == 3) 
	{
		for (unsigned int i = 0; i < fNGenMuons; i++) 
		{
			fHGenPtMu[i][cut]->Fill((*fGenMuon)[i].Pt(), puw);
			fHGenEtaMu[i][cut]->Fill((*fGenMuon)[i].Eta(), puw);
		}
	}
}

// Get Final state
unsigned int AnalysisVH::GetFSID(const unsigned int & nel, 
		const unsigned int & nmu, const unsigned int & ntau) const 
{
	unsigned int fs = _iFSunknown;
        if (nel == 3)
        {
		fs = _iFSeee;
	}
        else if (nel == 2) 
        {
		if (nmu == 1)
		{
			fs = _iFSeem;
		}
		else if (ntau == 1)
		{
			fs = _iFSeet;
		}
	}
	else if (nel == 1) 
	{
		if (nmu == 2)
		{
			fs = _iFSmme;
		}
		else if(ntau == 2)
		{
			fs = _iFStte;
		}
		else if (nmu == 1 && ntau == 1)
		{
			fs = _iFSemt;
		}
	}
	else if(nel == 0) 
	{
		if (nmu == 3)
		{
			fs = _iFSmmm;
		}
		else if (ntau == 3)
		{
			fs = _iFSttt;
		}
		else if (nmu == 2 && ntau == 1)
		{
			fs = _iFSmmt;
		}
		else if (nmu == 1 && ntau == 2)
		{
			fs = _iFSttm;
		}
	}
        
/*	unsigned int fs = _iFSunknown;
	const unsigned int fsencoded = nel*1000+nmu*100+ntau*10;

	return fsencoded;
}*/
        return fs;
}
