#include<sstream>
#include<stdlib.h>
#include<cstring>
#include<iostream>
#include<functional>
#include<algorithm>

#include "AnalysisWZ.h"
#include "InputParameters.h"
#include "CutLevels.h"
#include "CutManager.h"
#include "PUWeight.h"

#include "TTree.h"
#include "TH1D.h"
#include "TMath.h"

// To be changed: FIXME
const double kZMass = 91.1876;
//const unsigned int kNMuons = 3; 
const unsigned int kNMuons = 2; 
const unsigned int kWPID   = 24; //Found with TDatabasePDG::Instance()->GetParticle("W+")->PdgCode()
const unsigned int kTauPID = 15; //Found with TDatabasePDG::Instance()->GetParticle("tau-")->PdgCode()
		

AnalysisWZ::AnalysisWZ( TreeManager * data, std::map<LeptonTypes,InputParameters*> ipmap, 
				CutManager * selectorcuts, const unsigned int & finalstate ) :
	AnalysisBase(data,ipmap,selectorcuts,finalstate ) 
{ 
	// Number of cuts
	fNCuts = WZCuts::_iNCuts;
	for(unsigned int i = 0; i < fNCuts; ++i)
	{
		fCutNames.push_back(WZCuts::kCutNames[i]);
	}
} 


void AnalysisWZ::Initialise()
{
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
			SignatureFS::_iFStotal, 0, SignatureFS::_iFStotal);
	for(unsigned int i = 0; i < SignatureFS::_iFStotal; i++)
	{
		_histos[fHGenFinalState]->GetXaxis()->SetBinLabel(i+1,SignatureFS::kFinalStates[i]);
	}
	_histos[fHGenFinalState]->Sumw2();

	// Final state from generation (no taus)
	_histos[fHGenFinalStateNoTaus] = CreateH1D("fHGenFinalStateNoTaus", "Final State (no #tau)", 
			SignatureFS::_iFStotal, 0, SignatureFS::_iFStotal);
	for(unsigned int i = 0; i < SignatureFS::_iFStotal; i++)
	{
		_histos[fHGenFinalStateNoTaus]->GetXaxis()->SetBinLabel(i+1,SignatureFS::kFinalStates[i]);
	}
	_histos[fHGenFinalStateNoTaus]->Sumw2();

	// Generated electrons coming from a W
	_histos[fHNGenWElectrons] = CreateH1D("fHNGenWElectrons", "N Gen e from W", 5, -0.5, 4.5);
	// Generated muons coming from a W
	_histos[fHNGenWMuons] = CreateH1D("fHNGenWMuonss", "N Gen #mu from W", 5, -0.5, 4.5);
	// Generated leptons coming from a W
	_histos[fHNGenWLeptons] = CreateH1D("fHNGenWLeptons", "N Gen leptons from W", 5, -0.5, 4.5);
	
	// PT and Eta of most energetic gen muon from W or tau
	// Creating the maps
	for(unsigned int i = 0; i < _nLeptons; i++) 
	{
		fHGenPtLepton[i].resize(WZCuts::_iNCuts);
		fHGenEtaLepton[i].resize(WZCuts::_iNCuts);
		for (unsigned int j = 0; j < WZCuts::_iNCuts; j++) 
		{
			TString ptname  = Form("fHGenPtLepton%i_%i", i+1,j);
			TString pttitle = Form("P_{T} #mu_{gen}^{%i} from W (#tau)", i+1);
			TString etaname  = Form("fHGenEtaLepton%i_%i", i+1,j);
			TString etatitle = Form("#eta #mu_{gen}^{%i} from W (#tau)", i+1);
			fHGenPtLepton[i][j]  = CreateH1D(ptname,  pttitle, 150, 0, 150);
			fHGenEtaLepton[i][j] = CreateH1D(etaname, etatitle, 100, -5, 5);
		}
	}


	// Events passing every cut
	_histos[fHEventsPerCut] = CreateH1D("fHEventsPerCut", "Events passing each cut", 
			WZCuts::_iNCuts, 0, WZCuts::_iNCuts);
	for (unsigned int i = 0; i < WZCuts::_iNCuts; i++)
	{
		_histos[fHEventsPerCut]->GetXaxis()->SetBinLabel(i+1,WZCuts::kCutNames[i]);
	}
  
	// Events passing every cut that are 3 mu from gen
	_histos[fHEventsPerCut3Lepton] = CreateH1D("fHEventsPerCut3Lepton", "Events passing each cut that are 3 mu from gen", 
			WZCuts::_iNCuts, 0, WZCuts::_iNCuts);
	for(unsigned int i = 0; i < WZCuts::_iNCuts; i++)
	{
		_histos[fHEventsPerCut3Lepton]->GetXaxis()->SetBinLabel(i+1,WZCuts::kCutNames[i]);
	}

	// Number of Primary Vertices in the event
	_histos[fHNPrimaryVertices] = CreateH1D("fHNPrimaryVertices", "Number of Primary Vertices", 31, 0, 30);

	// Reconstructed muons in the event
	_histos[fHNRecoLeptons] = CreateH1D("fHNRecoLeptons", "Reconstructed #mu", 
			10, -0.5, 9.5);
	
	// Muons passing the basic selection
	_histos[fHNSelectedLeptons] = CreateH1D("fHNSelectedLeptons", "Selected #mu", 
			       10, -0.5, 9.5);

	// Selected Muons close to the PV
	_histos[fHNSelectedPVLeptons] = CreateH1D("fHNSelectedPVLeptons", 
			"Selected #mu close to PV", 10, -0.5, 9.5);
	// Selected Isolated Muons
	_histos[fHNSelectedIsoLeptons] = CreateH1D("fHNSelectedIsoLeptons", 
			"Selected Isolated #mu", 10, -0.5, 9.5);
	// Selected Isolated Good Muons
	_histos[fHNSelectedIsoGoodLeptons] = CreateH1D("fHNSelectedIsoGoodLeptons", 
			"Selected good Isolated #mu", 10, -0.5, 9.5);
	
	// Pt and eta of first/second/third good isolated muon
	for(unsigned int i = 0; i < _nLeptons; i++) 
	{
		TString ptname  = Form("fHPtLepton%i", i+1);
		TString pttitle = Form("P_{T}^{#mu_{%i}}", i+1);
		TString etaname  = Form("fHEtaLepton%i", i+1);
		TString etatitle = Form("#eta^{#mu_{%i}}", i+1);
		TString drname  = Form("fHDeltaRGenRecoLepton%i", i+1);
		TString drtitle = Form("#Delta R for #mu %i", i+1);
		fHPtLepton[i]  = CreateH1D(ptname,  pttitle, 150, 0, 150);
		fHEtaLepton[i] = CreateH1D(etaname, etatitle, 100, -5, 5);
		fHDeltaRGenRecoLepton[i] = CreateH1D(drname, drtitle, 150, 0, 5);
	}
	
	
	// Selected Isolated Good Muons
	_histos[fHLeptonCharge] = CreateH1D("fHLeptonCharge", "#Sum q_{#mu}", 7, -3.5, 3.5);
		
	// Invariant mass of leptons in/out of Z peak after all cuts
	_histos[fHZInvMass] = CreateH1D("fHZInvMass", "M^{inv.}_{#mu#mu}",120, 60, 120);
	_histos[fHZInvMassAfterZCand] = CreateH1D("fHZInvMassAfterZCand", "M^{inv.}_{#mu#mu}",120, 60, 120);
	_histos[fHZInvMassAfterWCand] = CreateH1D("fHZInvMassAfterWCand", "M^{inv.}_{#mu#mu}",120, 60, 120);
	
	// Missing ET after inv mass cut
	_histos[fHMET] = CreateH1D("fHMET", "MET",120, 0, 300);
	_histos[fHMETAfterZCand] = CreateH1D("fHMETAfterZCand", "MET",120, 0, 300);
	_histos[fHMETAfterWCand] = CreateH1D("fHMETAfterWCand", "MET",120, 0, 300);

	// Number of Jets after all cuts
	_histos[fHNJets] = CreateH1D("fHNJets", "NJets",21, 0, 20);
	
	// W candidate transvers mass
	_histos[fHTransversMass] = CreateH1D("fHTransversMass","m_{T}",100,0,100);
	
	// dR between leading lepton for the Z candidate and the W candidate lepton
	_histos[fHdRl1Wcand] = CreateH1D("fHdRl1Wcand","dR Z-cand. lepton leading w.r.t. W-cand. lepton",100,0,5);
	// dR between leading lepton for the Z candidate and the W candidate lepton
	_histos[fHdRl2Wcand] = CreateH1D("fHdRl2Wcand","dR Z-cand. lepton trailing w.r.t. W-cand. lepton",100,0,5);
}

//---------------------------------------------------------------------
// InsideLoop
//---------------------------------------------------------------------
unsigned int AnalysisWZ::InsideLoop()
{
#ifdef DEBUGANALYSIS
	std::cout << "========================================================" << std::endl;
	std::cout << "New event: " << fData->Get<int>("T_Event_EventNumber") << std::endl;
#endif
	// Get PU Weight
	//----------------------------------------------------------------------
	double puw(1);
	const int nPV = fData->Get<int>("T_Event_nPU");
	if(!fIsData)
	{
		puw = fPUWeight->GetWeight(nPV);
	}

	// Generation studies
	//----------------------------------------------------------------------
	unsigned int fsTaus = SignatureFS::_iFSunknown;
	unsigned int fsNTau = SignatureFS::_iFSunknown;
	fNGenElectrons = 0; //Number of generated electrons from W or tau
	fNGenMuons = 0;     //Number of generated muons from W or tau
	fNGenLeptons = 0;   //Number of generated leptons from W or tau
	if(fIsWH) 
	{
		// + Classify by leptonic final state (taus undecayed)
		unsigned int nelecsfromW = fData->GetSize<int>("T_Gen_ElecSt3_PID");
		unsigned int nmusfromW = fData->GetSize<int>("T_Gen_MuonSt3_PID");
		unsigned int ntausfromW = fData->GetSize<int>("T_Gen_TauSt3_PID");
		
		// Later it will be usefull knowing in what lepton decay the W->tau
		std::vector<TLorentzVector> elecsfromtaus;
		std::vector<TLorentzVector> muonsfromtaus;
		for(unsigned int i = 0; i < ntausfromW; ++i)
		{
			if( abs(fData->Get<int>("T_Gen_TauSt3_LepDec_PID",i)) == 11 )
			{
				elecsfromtaus.push_back(
                                     TLorentzVector( fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i),
					     fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i),
					     fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i),
					     fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i) ) );
#ifdef DEBUGANALYSIS
				std::cout << "DEBUG: Elec from tau (from W) "
					<< " E: " << fData->Get<float>("T_Gen_TauSt3_LepDec_Energy",i)
					<< " Px: " << fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i)
					<< std::endl;
#endif
			}
			else if( abs(fData->Get<int>("T_Gen_TauSt3_LepDec_PID",i)) == 13 )
			{
				muonsfromtaus.push_back(
                                     TLorentzVector( fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i),
					     fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i),
					     fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i),
					     fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i) ) );
#ifdef DEBUGANALYSIS
				std::cout << "DEBUG: Muon from tau (from W) "
					<< " E: " << fData->Get<float>("T_Gen_TauSt3_LepDec_Energy",i)
					<< " Px: " << fData->Get<float>("T_Gen_TauSt3_LepDec_Px",i)
					<< std::endl;
#endif
			}
		}


		unsigned int ngenfromW;
		if( fFS  == SignatureFS::_iFSmmm )
		{
			ngenfromW = nmusfromW;
		}
		else if( fFS == SignatureFS::_iFSeee ) 
		{
			ngenfromW = nelecsfromW;
		}
		else if( fFS == SignatureFS::_iFSmme || fFS == SignatureFS::_iFSeem )
		{
			ngenfromW = nelecsfromW+nmusfromW;
		}
		
		_histos[fHNGenWElectrons]->Fill(nelecsfromW,puw);
		_histos[fHNGenWMuons]->Fill(nmusfromW,puw);
		_histos[fHNGenWLeptons]->Fill(ngenfromW,puw);
		fsTaus = SignatureFS::GetFSID(nelecsfromW, nmusfromW, ntausfromW);
		_histos[fHGenFinalState]->Fill(fsTaus, puw);
		
#ifdef DEBUGANALYSIS
		std::cout << "DEBUG: W->e/mu/tau " << nelecsfromW 
			<< "/" << nmusfromW << "/" << ntausfromW << std::endl;
		std::cout << "DEBUG: fsTaus --> " << fsTaus << std::endl;
#endif
		int igen[3] = {-1, -1, -1};
		std::string iname[3] = {"", "", ""}; 
		// + Classify by leptonic final state (no taus)
		for(unsigned int i = 0; i < fData->GetSize<int>("T_Gen_Elec_MPID"); i++) 
		{
#ifdef DEBUGANALYSIS
			std::cout << "DEBUG: Elec [" << i << "/" << fData->GetSize<int>("T_Gen_Elec_MPID") << "]" 
				<< " Status: " << fData->Get<int>("T_Gen_Elec_MSt",i)
				<< " Mother: " << fData->Get<int>("T_Gen_Elec_MPID",i)
				<< " E: " << fData->Get<float>("T_Gen_Elec_Energy",i)
				<< " Px: " << fData->Get<float>("T_Gen_Elec_Px",i)
				<< std::endl;
#endif
			//I have seen that electrons from taus have status 2 and are not replicated
			// also I have to be sure the mother tau has decayed from the W
			unsigned int mpidel  = TMath::Abs(fData->Get<int>("T_Gen_Elec_MPID",i));
			if( mpidel == kWPID || ( mpidel == kTauPID && elecsfromtaus.size() != 0) ) 
			{
				// Checking that the electron really decay from the W->tau (st3)
				if( mpidel == kTauPID )
				{
					TLorentzVector elec( fData->Get<float>("T_Gen_Elec_Px",i),
							fData->Get<float>("T_Gen_Elec_Py",i),
							fData->Get<float>("T_Gen_Elec_Pz",i),
							fData->Get<float>("T_Gen_Elec_Energy",i) );
					int neq = std::count_if(elecsfromtaus.begin(), 	elecsfromtaus.end(), 
							std::bind1st(std::equal_to<TLorentzVector>(),elec) );
					if( neq == 0 )
					{
						continue;
					}
				}
				if( fLeptonType == ELECTRON 
	        			|| fLeptonType == MIX2MU1ELE || fLeptonType == MIX2ELE1MU )
				{
					igen[fNGenLeptons] = i;
					iname[fNGenLeptons] = "Elec";
					++fNGenLeptons;
				}
				++fNGenElectrons;
			}
		}
		
		for (unsigned int i = 0; i < fData->GetSize<int>("T_Gen_Muon_MPID"); ++i) 
		{
#ifdef DEBUGANALYSIS
			std::cout << "DEBUG: Muon [" << i+1 << "/" << fData->GetSize<int>("T_Gen_Muon_MPID") << "]" 
				<< " Status: " << fData->Get<int>("T_Gen_Muon_MSt",i)
				<< " Mother: " << fData->Get<int>("T_Gen_Muon_MPID",i)
				<< " E: " << fData->Get<float>("T_Gen_Muon_Energy",i)
				<< " Px: " << fData->Get<float>("T_Gen_Muon_Px",i)
				<< std::endl;
#endif
			//I have seen that muons from taus have status 2 and are not replicated
			// also I have to be sure the mother tau has decayed from the W
			unsigned int mpid  = TMath::Abs(fData->Get<int>("T_Gen_Muon_MPID",i));
			if ( mpid == kWPID || (mpid == kTauPID && muonsfromtaus.size() != 0) ) 
			{
				// Checking that the muon really decay from the W->tau (st3)
				if( mpid == kTauPID )
				{
					TLorentzVector muon( fData->Get<float>("T_Gen_Muon_Px",i),
							fData->Get<float>("T_Gen_Muon_Py",i),
							fData->Get<float>("T_Gen_Muon_Pz",i),
							fData->Get<float>("T_Gen_Muon_Energy",i) );
					int neq = std::count_if(elecsfromtaus.begin(), 	elecsfromtaus.end(), 
							std::bind1st(std::equal_to<TLorentzVector>(),muon) );
					if( neq == 0 )
					{
						continue;
					}
				}
				if( fLeptonType == MUON 
	        			|| fLeptonType == MIX2MU1ELE || fLeptonType == MIX2ELE1MU )
				{
					igen[fNGenLeptons] = i;
					iname[fNGenLeptons] = "Muon";
					++fNGenLeptons;
				}
				++fNGenMuons;
			}
		}
		fsNTau = SignatureFS::GetFSID(fNGenElectrons, fNGenMuons, 3-fNGenMuons-fNGenElectrons);
		_histos[fHGenFinalStateNoTaus]->Fill(fsNTau, puw);
		
#ifdef DEBUGANALYSIS
		std::cout << "DEBUG: W->e/mu/tau " <<  fNGenElectrons
			<< "/" << fNGenMuons << "/" << (3-fNGenMuons-fNGenElectrons) << std::endl;
		std::cout << "DEBUG: fsNTau --> " << fsNTau << std::endl;
#endif
		// Initialize generation vector
		fGenLepton.clear();
		fGenLeptonName.clear();
		//   Sort by energy (? or by pt?)
		if(fNGenLeptons == _nLeptons)  
		{
			std::map<double,std::pair<TLorentzVector,std::string> > vOrder;
			std::vector<TLorentzVector> * vGenMuon = new std::vector<TLorentzVector>;
			for(unsigned int i = 0; i < fNGenLeptons; i++) 
			{
				// Extract the name
				std::string genname = std::string("Gen_"+iname[i]);
				vGenMuon->push_back( this->GetTLorentzVector( genname.c_str(),igen[i]) ); 
				vOrder[vGenMuon->back().Pt()] = 
					std::pair<TLorentzVector,std::string>(vGenMuon->back(),iname[i]);
			}
			for(std::map<double,std::pair<TLorentzVector,std::string> >::reverse_iterator it = vOrder.rbegin(); 
					it != vOrder.rend(); ++it)
			{
				fGenLepton.push_back( it->second.first );
				fGenLeptonName.push_back( it->second.second );
			}
			delete vGenMuon;
			vGenMuon = 0;
			
#ifdef DEBUGANALYSIS
			for (unsigned int i = 0; i < fNGenLeptons; ++i) 
			{
				std::cout << "DEBUG: Lepton:" << fGenLeptonName[i] << " [" << i << "] <- [" << igen[i] 
					<< "] PT = " << fGenLepton[i].Pt() 
					<< " - ET = " << fGenLepton[i].Energy() 
					<< std::endl;
			}      
#endif
			FillGenPlots(WZCuts::_iAllEvents,puw);
		}
	}
	
	// All events
	//------------------------------------------------------------------
	FillHistoPerCut(WZCuts::_iAllEvents, puw, fsNTau);
  
	// Proccess ID
	//------------------------------------------------------------------
	int procn = _iOther;
	const int processID = fData->Get<int>("T_Event_processID");//GetEventprocessID();
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

	/*if(fIsWH && (procn != _iWH || fsNTau != fFS))
	{
		return WZCuts::_iIsWH;
	}*/
	
	FillHistoPerCut(WZCuts::_iIsWH, puw, fsNTau);

	// HLT: TBD...
	//------------------------------------------------------------------
	if( ! IspassHLT() )
	{
		return WZCuts::_iHLT;
	}
	FillHistoPerCut(WZCuts::_iHLT, puw, fsNTau);

	// Store the number of reconstructed leptons without any filter
	int nLeptonsbfCuts = 0;
	for(unsigned int i = 0; i < fLeptonName.size(); ++i)
	{
		std::string leptonpx = std::string("T_"+fLeptonName[i]+"_Px");
		nLeptonsbfCuts += fData->GetSize<float>(leptonpx.c_str());
	}
	_histos[fHNRecoLeptons]->Fill(nLeptonsbfCuts,puw);
	
	// (1) Basic selection
	//--------------------
	unsigned int nSelectedMuons = fLeptonSelection->GetNBasicLeptons();
	_histos[fHNSelectedLeptons]->Fill(nSelectedMuons,puw);

	if( ! fLeptonSelection->IspassAtLeastN(kNMuons,nSelectedMuons) )
	{
		return WZCuts::_iHas2Leptons;
	}
	
	FillHistoPerCut(WZCuts::_iHas2Leptons, puw, fsNTau);
	
	// (2) PV selection
	//--------------------
	unsigned int nSelectedPVMuons = fLeptonSelection->GetNLeptonsCloseToPV();
	_histos[fHNSelectedPVLeptons]->Fill(nSelectedPVMuons,puw); 
	
	if( ! fLeptonSelection->IspassAtLeastN(kNMuons,nSelectedPVMuons) )
	{
		return WZCuts::_iHas2PVLeptons;
	}

	FillHistoPerCut(WZCuts::_iHas2PVLeptons, puw, fsNTau);


	// (3) Isolated muons
	//--------------------
	unsigned int nSelectedIsoMuons = fLeptonSelection->GetNIsoLeptons();
	_histos[fHNSelectedIsoLeptons]->Fill(nSelectedIsoMuons,puw);  
	
	if( ! fLeptonSelection->IspassAtLeastN(kNMuons,nSelectedIsoMuons) )
	{
		return WZCuts::_iHas2IsoLeptons;
	}
	
	FillHistoPerCut(WZCuts::_iHas2IsoLeptons, puw, fsNTau);
	
	// (4) Isolated good muons: Identification (tight + notight)
	//--------------------
	unsigned int nSelectedIsoGoodMuons = fLeptonSelection->GetNGoodIdLeptons();
	_histos[fHNSelectedIsoGoodLeptons]->Fill(nSelectedIsoGoodMuons,puw);
	
	// Note that nSelectedIsoGoodMuons are tight+noTights leptons (when proceed, so
	// I cannot use the CutManager::IspassAtLeastN(howmany,nTights) because the 
	// second argument are the number of  tight leptons
	if( kNMuons > nSelectedIsoGoodMuons )
	{
		return WZCuts::_iHas2IsoGoodLeptons;
	}
	
	FillHistoPerCut(WZCuts::_iHas2IsoGoodLeptons, puw, fsNTau);	
	//FillGenPlots(_iHas2IsoGoodLeptons,puw);
	// Keep events at least 3 leptons and store momentum and charge
	//---------------------------------------------------------------------------
	if( ! fLeptonSelection->IspassAtLeastN() )
	{
		return WZCuts::_iHasAtLeast3Leptons;
	}
	// Using the fake rate if we are in fake mode
	if( fFO != 0 )
	{
		//FIXME: Note that by the moment we are assuming that the only fakeable sample
		// available is TTnT, so there are just 1 no tight lepton (and must be at least 1)
		const unsigned int i = fLeptonSelection->GetNoTightIndex(0);
		const LeptonTypes ileptontype= fLeptonSelection->GetNoTightLeptonType(0);
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
	FillHistoPerCut(WZCuts::_iHasAtLeast3Leptons, puw, fsNTau);
	FillGenPlots(WZCuts::_iHasAtLeast3Leptons,puw);

	// N-primary vertices
	_histos[fHNPrimaryVertices]->Fill(nPV,puw);

	// Indexs of good leptons (noTight+Tights if proceed)
	std::vector<int> * theLeptons = fLeptonSelection->GetGoodLeptons(); 

	// + Fill histograms with Pt and Eta
	int k = 0;  // Note that k is the index of the vectors, not the TTree
	// + Store Momentum and charge for the muons
	std::vector<TLorentzVector> lepton;
	std::vector<int> leptonCharge;
	std::vector<LeptonTypes> leptontypes;
	for(std::vector<int>::iterator it = theLeptons->begin(); it != theLeptons->end(); ++it) 
	{
		unsigned int i = *it;
		const char * name = 0;
		LeptonTypes ileptontype = fLeptonSelection->GetLeptonType(k);
		std::string chargedm;
		if( ileptontype == MUON )
		{
			name = "Muon";
			chargedm = "T_Muon_Charge";
		}
		else
		{
			name = "Elec";
			chargedm = "T_Elec_Charge";
		}
		// Fill histos
		const TLorentzVector vL = this->GetTLorentzVector(name,i);
		fHPtLepton[k]->Fill(vL.Pt(), puw);
		fHEtaLepton[k]->Fill(vL.Eta(), puw);
		if( fGenLepton.size() != 0 )
		{
			fHDeltaRGenRecoLepton[k]->Fill(vL.DeltaR(fGenLepton.at(k)), puw);
		}
		// charge and leptons
		lepton.push_back( this->GetTLorentzVector(name,i) );
		leptonCharge.push_back( fData->Get<int>(chargedm.c_str(),i) );
		leptontypes.push_back( ileptontype );
		++k;
	}
	
	
	// Storing the charge
	int charge = 0;
	for(unsigned int k = 0; k < leptonCharge.size(); ++k)
	{
		charge += leptonCharge[k];
	}
	// Fill charge before rejecting or accepting  
	_histos[fHLeptonCharge]->Fill(charge, puw);
	
	// Build Z candidates: same flavour opposite charge
	// ------------------------------------------------------------------
	LeptonTypes zcandflavour;
	std::vector<double> ptcutv;
	if( fFS == SignatureFS::_iFSmmm || fFS == SignatureFS::_iFSmme )
	{
		zcandflavour = MUON;
		//ptcutv.push_back(15.0);
		//ptcutv.push_back(15.0);
		ptcutv.push_back(20.0);
		ptcutv.push_back(10.0);
	}
	else if( fFS == SignatureFS::_iFSeee || fFS == SignatureFS::_iFSeem )
	{
		zcandflavour = ELECTRON;
		//ptcutv.push_back(20.0);
		//ptcutv.push_back(10.0);
		ptcutv.push_back(20.0);
		ptcutv.push_back(10.0);
	}

	// Find muons with opposite charges and the ptcuts fulfill
	// Note that we are not using anymore the index of the Tree, instead
	// are using the index of lepton, leptonCharge and leptontype vectors
	std::vector<std::pair<int,int> > leptonPair;
	for(unsigned int kbegin = 0; kbegin < leptonCharge.size(); ++kbegin)
	{
		for(unsigned int kfromend = leptonCharge.size()-1; kfromend > kbegin; --kfromend) 
		{
			// Same flavour (and specifically the flavour of the channel
			if( leptontypes[kbegin] != zcandflavour 
				 || ( leptontypes[kbegin] != leptontypes[kfromend] ) )
			{
				continue;
			}
			// opposite charge
			if( leptonCharge[kbegin]*leptonCharge[kfromend] >= 0 )
			{
				continue;
			}
			// Check pass the pt (recall PT[kbegin] > PT[kfromend])
			if( ptcutv[0] > lepton[kbegin].Pt() || ptcutv[1] > lepton[kfromend].Pt() )
			{
				continue;
			}
			leptonPair.push_back( std::pair<int,int>(kbegin,kfromend) );
		}
	}
	// rejecting events with no opposite charge leptons: Better put it before
	// when already get the total charge of the selected leptons
	if( leptonPair.size() == 0 )
	{
		return WZCuts::_iOppositeCharge;
	}	
	// Accepted events with two opposite charge leptons
	FillHistoPerCut(WZCuts::_iOppositeCharge, puw, fsNTau);
	
	// Calculate invMass...
	// Keep the pair with Mass closer to Z nominal
	// ------------------------------------------------------------------	
	std::map<double,std::pair<int,int> > candidatesZMass;
	for(std::vector<std::pair<int,int> >::iterator it = leptonPair.begin(); 
			it != leptonPair.end(); ++it)
	{
		unsigned int i1 = it->first;
		unsigned int i2 = it->second;
		const double invMass= (lepton[i1]+lepton[i2]).M();
		if( invMass > 120.0 || invMass < 60.0 )
		{
			continue;
		}
		const double deltaZMass = fabs(kZMass-invMass);
		candidatesZMass[deltaZMass] = std::pair<int,int>(i1,i2);
	}


	if( candidatesZMass.size() == 0 )
	{
		return WZCuts::_iHasZCandidate;
	}
	FillHistoPerCut(WZCuts::_iHasZCandidate, puw, fsNTau);
        
	// + Getting the nearest pair to nominal ZMass
	// Remember map<double,pair>  (second is the pair)
	const unsigned int i1Z = ((candidatesZMass.begin())->second).first;
	const unsigned int i2Z = ((candidatesZMass.begin())->second).second;
	// Also check that there are not another non-overlapping Z: ZZ rejection
	if( candidatesZMass.size() > 1 )
	{
		for(std::map<double,std::pair<int,int> >::iterator it = ++candidatesZMass.begin(); 
				it != candidatesZMass.end(); ++it)
		{
			const unsigned int index1 = (it->second).first;
			const unsigned int index2 = (it->second).second;
			if( index1 != i1Z && index1 != i2Z && index2 != i1Z && index2 != i2Z)
			{
				// Found another Z non-overlapping with the other already found
				return WZCuts::_iHasZOverlapping;
			}
		}
	}
	FillHistoPerCut(WZCuts::_iHasZOverlapping, puw, fsNTau);

	const double invMassLL = (lepton[i1Z] + lepton[i2Z]).M();

	// + Fill histograms
	//   - Invariant mass of leptons supposedly from Z
	_histos[fHZInvMassAfterZCand]->Fill(invMassLL,puw);
	// Extract MET to fill histograms
	const double met = fData->Get<float>("T_METPF_ET");
	_histos[fHMETAfterZCand]->Fill(met,puw);
	
	// W selection
	//------------------------------------------------------------------
	// Looking the not used leptons
	LeptonTypes wcandflavour;
	if( fFS == SignatureFS::_iFSmmm || fFS == SignatureFS::_iFSeem )
	{
		wcandflavour = MUON;
	}
	else if( fFS == SignatureFS::_iFSeee || fFS == SignatureFS::_iFSmme )
	{
		wcandflavour = ELECTRON;
	}

	// Some MET stuff we're going to need
	const double phi = fData->Get<float>("T_METPF_Phi");
	const double px = met*cos(phi);
	const double py = met*sin(phi);
	TLorentzVector METV(px,py,0.0,0.0); 

	std::map<int,double> transverseMassW;
	std::map<double,int> wcandidate;
	for(unsigned int i=0; i < lepton.size(); ++i) 
	{
		// lepton used in the Z candidate
		if( i == i1Z || i == i2Z )
		{
			continue;
		}
		// Must be the correct flavour
		if( leptontypes[i] != wcandflavour )
		{
			continue;
		}
		// Pt cut and isolation // FIXME just for electrons WP80
		const double pt = lepton[i].Pt();
		if( pt < 20.0 )
		{
			continue;
		}
		// NEW (and important for 2mu1e channel): Rejecting the fake electron aligned with 
		// the muon due to internal bremsstrahlung in W an Z decays
		const double dRl1 = lepton[i].DeltaR( lepton[i1Z] );
		const double dRl2 = lepton[i].DeltaR( lepton[i2Z] );
		_histos[fHdRl1Wcand]->Fill( dRl1, puw );
		_histos[fHdRl2Wcand]->Fill( dRl2, puw );
		if( dRl1 < 0.1 || dRl2 < 0.1 )
		{
			continue;
		}
		// Stores transverse mass (W cand., MET)
		const double lWEt = lepton[i].Et();
		const double tMassW = sqrt( lWEt*lWEt + met*met - 2.0*lWEt*met*cos(lepton[i].DeltaPhi(METV)));
		wcandidate[pt] = i;
		transverseMassW[i] = tMassW;
	}
	// No W candidate
	if( wcandidate.size() == 0 )
	{
		return WZCuts::_iHasWCandidate; 
	}
	FillHistoPerCut(WZCuts::_iHasWCandidate, puw, fsNTau);
	// Fill histos
	_histos[fHZInvMassAfterWCand]->Fill(invMassLL,puw);
	_histos[fHMETAfterWCand]->Fill(met,puw);
	// Getting the highest pt lepton to W-candidate (if there are more than one)
	const int iWcand = wcandidate.rbegin()->second;	
	_histos[fHTransversMass]->Fill(transverseMassW[iWcand],puw);
	
	// Jet Veto:
	//------------------------------------------------------------------
	unsigned int nJets = 0;
	//for(unsigned int k = 0; k < fData->GetJetAKPFNoPUEnergy()->size(); ++k) 
	for(unsigned int k = 0; k < fData->GetSize<float>("T_JetAKPFNoPU_Energy"); ++k) 
	{
		TLorentzVector Jet = this->GetTLorentzVector("JetAKPFNoPU",k);
		//FIXME: Add the pt,eta and deltaR cuts in the config file
		if( Jet.Pt() <= 30 || fabs(Jet.Eta()) >= 5 )
		{
			continue;
		}
		// Leptons not inside the Jets
		bool leptoninsideJet = false;
		for(unsigned int j = 0; j < lepton.size(); ++j)
		{
			if( fabs(Jet.DeltaR(lepton[j])) <= 0.3 )
			{
				leptoninsideJet = true;
				break;
			}
		}
		if( leptoninsideJet )
		{
			continue;
		}
		nJets++;
	}
	// Storing before the veto
	_histos[fHNJets]->Fill(nJets,puw);
  	
	
	// MET
	//------------------------------------------------------------------
	std::vector<double> * auxVar = new std::vector<double>;
	auxVar->push_back( met );
	if( ! fLeptonSelection->IsPass("MinMET", auxVar) ) 
	{
		return WZCuts::_iMET;
	}
	FillHistoPerCut(WZCuts::_iMET, puw, fsNTau);

	delete auxVar;
	auxVar=0;

	// Filling histos -------------------------------------
	_histos[fHZInvMass]->Fill(invMassLL,puw);
	_histos[fHMET]->Fill(met,puw);

	return WZCuts::_iNCuts;
	
}
