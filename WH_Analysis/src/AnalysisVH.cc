#include<sstream>
#include<stdlib.h>
#include<cstring>
#include<iostream>
#include<functional>
#include<algorithm>

#include "AnalysisVH.h"
#include "InputParameters.h"
#include "CutManager.h"
#include "PUWeight.h"
#include "CutLevels.h"

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
AnalysisVH::AnalysisVH(TreeManager * data, std::map<LeptonTypes,InputParameters*> ipmap, 
		CutManager * selectioncuts, const unsigned int & finalstate ) :
	AnalysisBase(data,ipmap,selectioncuts,finalstate )
{
	// Number of cuts
	fNCuts = WHCuts::_iNCuts;
	for( unsigned int i = 0 ; i < fNCuts; ++i)
	{
		fCutNames.push_back(WHCuts::kCutNames[i]);
	}
}

AnalysisVH::~AnalysisVH()
{
}

void AnalysisVH::Initialise()
{
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
	
	// PT and Eta of most energetic gen lepton from W or tau
	// Creating the maps
	for(unsigned int i = 0; i < _nLeptons; i++) 
	{
		fHGenPtLepton[i].resize(WHCuts::_iNCuts);
		fHGenEtaLepton[i].resize(WHCuts::_iNCuts);
		for (unsigned int j = 0; j < WHCuts::_iNCuts; j++) 
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
			WHCuts::_iNCuts, 0, WHCuts::_iNCuts);
	for (unsigned int i = 0; i < WHCuts::_iNCuts; i++)
	{
		_histos[fHEventsPerCut]->GetXaxis()->SetBinLabel(i+1,WHCuts::kCutNames[i]);
	}
  
	// Events passing every cut that are 3 mu from gen
	_histos[fHEventsPerCut3Lepton] = CreateH1D("fHEventsPerCut3Lepton", "Events passing each cut that are 3 mu from gen", 
			WHCuts::_iNCuts, 0, WHCuts::_iNCuts);
	for(unsigned int i = 0; i < WHCuts::_iNCuts; i++)
	{
		_histos[fHEventsPerCut3Lepton]->GetXaxis()->SetBinLabel(i+1,WHCuts::kCutNames[i]);
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
	
	//Smallest DeltaR between 2 opp. sign leptons
	_histos[fHMinDeltaRLp1Lp2] = CreateH1D("fHMinDeltaRLp1Lp2", "Smallest #Delta R_{#mu#mu}",
			125, 0, 5);
	//Smallest DeltaR between 2 opp. sign leptons
	_histos[fHMaxDeltaRLp1Lp2] = CreateH1D("fHMaxDeltaRLp1Lp2", "Largest #Delta R_{#mu#mu}",
			125, 0, 5);
	
	//Smallest DeltaPhi between 2 opp. sign leptons
	_histos[fHMinDeltaPhiLp1Lp2] = CreateH1D("fHMinDeltaPhiLp1Lp2", "Smallest #Delta #phi_{#mu#mu}",
			120, 0, TMath::Pi());
	//Largest DeltaPhi between 2 opp. sign leptons
	_histos[fHMaxDeltaPhiLp1Lp2] = CreateH1D("fHMaxDeltaPhiLp1Lp2", "Largest #Delta #phi_{#mu#mu}",
			120, 0, TMath::Pi());
	
	// Sum of charge of selected Isolated Good Leptons after opp. sign cut
	_histos[fHLeptonCharge] = CreateH1D("fHLeptonCharge", "#Sum q_{#mu}", 7, -3.5, 3.5);
		
	// Invariant mass of leptons supposedly from H  after all cuts
	_histos[fHHInvMass] = CreateH1D("fHHInvMass", "M^{inv.}_{#mu#mu}", 150, 0, 150);
	// Invariant mass of leptons supposedly from H  after Jet Veto
	_histos[fHHInvMassAfterJetVeto] = CreateH1D("fHHInvMassAfterJetVeto", "M^{inv.}_{#mu#mu}", 150, 0, 150);
	
	// Missing ET after inv mass cut
	_histos[fHMET] = CreateH1D("fHMET", "MET",160, 0, 160);
	// Missing ET after inv mass cut
	_histos[fHMETAfterWCand] = CreateH1D("fHMETAfterWCand", "MET",160, 0, 160);

	// Number of jets before jet veto cut
	_histos[fHNJets] = CreateH1D("fHNJets", "Number of Jets", 10, 0, 10);

	// Transverse Mass of all leptons and MET after all cuts
	_histos[fHTransversMass] = CreateH1D("fHTransversMass", "M_{t}^{#ell#ell#ell}",400, 0, 400);
	// Transverse Mass of all leptons and MET after het veto cut
	_histos[fHTransversMassAfterWCand] = CreateH1D("fHTransversMassAfterWCand", "M_{t}^{#ell#ell#ell}",400, 0, 400);

	// Invariant Mass of the 3 leptons after all cuts
	_histos[fHTrileptonMass] = CreateH1D("fHTrileptonMass", "M_{#ell#ell#ell}",400, 0, 400);
	// Invariant Mass of the 3 leptons after Jet veto cut
	_histos[fHTrileptonMassAfterWCand] = CreateH1D("fHTrileptonMassAfterWCand", "M_{#ell#ell#ell}",400, 0, 400);
	
	// Ht: Sum of all transverse energy of the event (Jets+leptons+MET) after all cuts
	_histos[fHHT] = CreateH1D("fHHT", "H_{T}",400, 0, 400);
	// Ht: Sum of all transverse energy of the event (Jets+leptons+MET) after Jet Veto cut
	_histos[fHHTAfterWCand] = CreateH1D("fHHTAfterWCand", "H_{T}",400, 0, 400);
	
}

//---------------------------------------------------------------------
// InsideLoop
//---------------------------------------------------------------------
std::pair<unsigned int,float> AnalysisVH::InsideLoop()
{
#ifdef DEBUGANALYSIS
	std::cout << "========================================================" << std::endl;
	std::cout << "New event: " << fData->Get<int>("T_Event_EventNumber") << std::endl;
#endif
	// Get PU Weight
	//----------------------------------------------------------------------
	double puw(1);
	const int nPV = fData->GetSize<int>("T_Vertex_z");
	if(!fIsData)
	{
		puw = fPUWeight->GetWeight(fData->Get<int>("T_Event_nPU"));
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
		if(fNGenLeptons == 3)  // FIXME:: PORQUE SOLO 3?? --> cAMBIAR POR nLeptons
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
			FillGenPlots(WHCuts::_iAllEvents,puw);
		}
	}
	
	// All events
	//------------------------------------------------------------------
	FillHistoPerCut(WHCuts::_iAllEvents, puw, fsNTau);
  
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

	if(fIsWH && (procn != _iWH || fsNTau != fFS))
	{
		return std::pair<unsigned int,float>(WHCuts::_iIsWH,puw);
	}
	
	FillHistoPerCut(WHCuts::_iIsWH, puw, fsNTau);

	// HLT: TBD...
	//------------------------------------------------------------------
	if( ! IspassHLT() )
	{
		return std::pair<unsigned int,float>(WHCuts::_iHLT,puw);
	}
	FillHistoPerCut(WHCuts::_iHLT, puw, fsNTau);

	// Vertex cut (Event stuff)-- OBSOLETE (implemented per default) SURE?
	//int iGoodVertex = GoodVertex();
	//int iGoodVertex = 0; // First one
	//fLeptonSelection->PassEventsCuts();
	//if( iGoodVertex < 0)
	//{
	//	return std::pair<unsigned int,float>(WHCuts::_iGoodVertex;
	//}
	//FillHistoPerCut(WHCuts::_iGoodVertex, puw, fsNTau);
	

	// First looking at events with 2 leptons
	//const int kNMuons = 2; --> Defined in the preamble
	// Muon selection
	//------------------------------------------------------------------
	//
	//this->SetGoodVertexIndex(iGoodVertex);
	
	// Store the number of reconstructed leptons without any filter
	int nLeptonsbfCuts = 0;
	for(unsigned int i = 0; i < fLeptonName.size(); ++i)
	{
		std::string leptonpx = std::string("T_"+fLeptonName[i]+"_Px");
		nLeptonsbfCuts += fData->GetSize<float>(leptonpx.c_str());
	}
	_histos[fHNRecoLeptons]->Fill(nLeptonsbfCuts);
	
	// (1) Basic selection
	//--------------------
	unsigned int nSelectedMuons = fLeptonSelection->GetNBasicLeptons();
	_histos[fHNSelectedLeptons]->Fill(nSelectedMuons,puw);

	if(nSelectedMuons < kNMuons)
	{
		return std::pair<unsigned int,float>(WHCuts::_iHas2Leptons,puw);
	}
	
	FillHistoPerCut(WHCuts::_iHas2Leptons, puw, fsNTau);
	
	// (2) PV selection
	//--------------------
	unsigned int nSelectedPVMuons = fLeptonSelection->GetNLeptonsCloseToPV();
	_histos[fHNSelectedPVLeptons]->Fill(nSelectedPVMuons,puw); 
	
	if(nSelectedPVMuons < kNMuons)
	{
		return std::pair<unsigned int,float>(WHCuts::_iHas2PVLeptons,puw);
	}

	FillHistoPerCut(WHCuts::_iHas2PVLeptons, puw, fsNTau);


	// (3) Isolated muons
	//--------------------
	unsigned int nSelectedIsoMuons = fLeptonSelection->GetNIsoLeptons();
	_histos[fHNSelectedIsoLeptons]->Fill(nSelectedIsoMuons,puw);  
	
	if(nSelectedIsoMuons < kNMuons)
	{
		return std::pair<unsigned int,float>(WHCuts::_iHas2IsoLeptons,puw);
	}
	
	FillHistoPerCut(WHCuts::_iHas2IsoLeptons, puw, fsNTau);
	
	// (4) Isolated good muons: Identification
	//--------------------
	unsigned int nSelectedIsoGoodMuons = fLeptonSelection->GetNGoodIdLeptons();
	_histos[fHNSelectedIsoGoodLeptons]->Fill(nSelectedIsoGoodMuons,puw);
	
	if(nSelectedIsoGoodMuons < kNMuons)
	{
		return std::pair<unsigned int,float>(WHCuts::_iHas2IsoGoodLeptons,puw);
	}
	
	FillHistoPerCut(WHCuts::_iHas2IsoGoodLeptons, puw, fsNTau);	
      
	// The leading lepton has to have PT > fCutMinMuPt (20 most probably)
	// Applying the pt-cuts
	//-------------------------------------------------------------------
	std::vector<double> * nLeptons= new std::vector<double>;
	// Describe the number of leptons we want to cut in order: 
	// --- nLeptons[0] = mmuons
	// --- nLeptons[1] = electrons 
	// Just needed for the mixing (and total?) case
	// FIXME: I don't like this patch but... 
	if( fFS == SignatureFS::_iFSmme )
	{
		nLeptons->push_back(2);
		nLeptons->push_back(1);
	}
	else if( fFS == SignatureFS::_iFSeem )
	{
		nLeptons->push_back(1);
		nLeptons->push_back(2);
	}

	if( ! fLeptonSelection->IsPass("PtMuonsCuts",nLeptons) )
	{
		return std::pair<unsigned int,float>(WHCuts::_iMuPTPattern,puw);
	}
	FillHistoPerCut(WHCuts::_iMuPTPattern, puw, fsNTau);
	delete nLeptons;
	nLeptons = 0;
	
	// Keep events with just 3 leptons and store momentum and charge
	//---------------------------------------------------------------------------
	if(nSelectedIsoGoodMuons != _nLeptons)
	{
		return std::pair<unsigned int,float>(WHCuts::_iHasExactly3Leptons,puw);
	}
	// Indexs of good leptons
	std::vector<int> * theLeptons = fLeptonSelection->GetGoodLeptons(); 
	// + Fill histograms with Pt and Eta
	int k = 0;  // Note that k is the index of the vectors, not the TTree
	// + Store momentum and charge from the muons
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
	FillGenPlots(WHCuts::_iHasExactly3Leptons,puw);
	
	FillHistoPerCut(WHCuts::_iHasExactly3Leptons, puw, fsNTau);
	
	// N-primary vertices
	_histos[fHNPrimaryVertices]->Fill(nPV,puw);

	// CAVEAT =================================================
	// Note that from here on, the indices used are the ones of
	// lepton, leptonCharge and leptontypes vectors
	// ========================================================
  
	// Discard 3 leptons with the same charge
	// ------------------------------------------------------------------
	// 
	// + Add up charges. If the abs value of the total number is equal to N then 
	//   all have the same sign
	int charge = 0;
	for(unsigned int k = 0; k < leptonCharge.size(); ++k)
	{
		charge += leptonCharge[k];
	}
	// Fill muon charge before rejecting or accepting  
	_histos[fHLeptonCharge]->Fill(charge, puw);
	
	if( (unsigned int)TMath::Abs(charge) == theLeptons->size() )
	{
		return std::pair<unsigned int,float>(WHCuts::_iNotSameSign,puw);
	}
	
	FillHistoPerCut(WHCuts::_iNotSameSign, puw, fsNTau);
	
	// Jet Veto:
	//------------------------------------------------------------------
	unsigned int nJets = 0;
	double sumEtJets = 0;
	//for(unsigned int k = 0; k < fData->GetJetAKPFNoPUEnergy()->size(); ++k) 
	for(unsigned int k = 0; k < fData->GetSize<float>("T_JetAKPFNoPU_Energy"); ++k) 
	{
		TLorentzVector Jet = this->GetTLorentzVector("JetAKPFNoPU",k);
		//FIXME: Add the cuts to the config file
		// Kinematics
		if(Jet.Pt() <= 30 || fabs(Jet.Eta()) >= 5)
		{
			continue;
		}
		// Leptons not inside the Jets
		bool leptoninsideJet = false;
		for(unsigned int j = 0; j < lepton.size(); ++j)
		{
			if( Jet.DeltaR(lepton[j]) <= 0.3 )
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
		sumEtJets += Jet.Et();
	}
	// - Number of Jets before the cut
	_histos[fHNJets]->Fill(nJets,puw);

	//if(nJets > 0)
	//{
	//	return std::pair<unsigned int,float>(WHCuts::_iJetVeto;
	//}
	//FillHistoPerCut(WHCuts::_iJetVeto, puw, fsNTau);

	
	// Find muons with opposite charges and calculate DeltaR, invMass...
	// Keep the pair with DeltaR minimum
	// ------------------------------------------------------------------
	// 
	// + Store the real vector index of the opposite charge muons in pairs
	//   (for the lepton and leptonCharge vectors: 0,...,nLeptons)
	std::vector<std::pair<int,int> > leptonPair;
	// + Also storing the potential Z candidates: opp. sign and same flavour
	LeptonTypes zcandflavour;
	std::vector<int> zcandindxpair;  // Index of the leptonPair vector candidate
	                                 // with lepton Z candidate
	if( fFS == SignatureFS::_iFSmmm || fFS == SignatureFS::_iFSmme )
	{
		zcandflavour = MUON;
	}
	else if( fFS == SignatureFS::_iFSeee || fFS == SignatureFS::_iFSeem )
	{
		zcandflavour = ELECTRON;
	}

	int indexPair = -1;  
	bool wasstoredpair = false;
	for(unsigned int kbegin = 0; kbegin < leptonCharge.size(); ++kbegin)
	{
		for(unsigned int kfromend = leptonCharge.size()-1; kfromend > kbegin; --kfromend) 
		{
			// Opposite charge
			if( leptonCharge[kbegin]*leptonCharge[kfromend] < 0 )
			{
				leptonPair.push_back( std::pair<int,int>(kbegin,kfromend) );
				++indexPair;
				wasstoredpair = true;
			}
			// Checking if the pair stored could be a Z candidate
			if( wasstoredpair && 
					leptontypes[kbegin] == zcandflavour &&
					( leptontypes[kbegin] == leptontypes[kfromend] ) )
			{
				zcandindxpair.push_back(indexPair);
				// Resetting
				wasstoredpair = false;
			}
		}
	}

	// At least a pair with opposite charge (Note: this cut is almost superfluous, it
	// is defined for the same sign control region... For the other regions there will
	// be no difference w.r.t. the last cut)
	if( leptonPair.size() < 1 )
	{
		return std::pair<unsigned int,float>(WHCuts::_iOppositeCharge,puw);
	} 
	FillHistoPerCut(WHCuts::_iOppositeCharge, puw, fsNTau);

	// + Find Min/Max DeltaR and DeltaPhi
	// Using ordering from maps (from lower to higher)
	// Again uses the index of the lepton and leptonCharge vectors
	std::map<double,std::pair<int,int> > deltaRMuMu;
	std::map<double,std::pair<int,int> > deltaPhiMuMu;
	for(std::vector<std::pair<int,int> >::iterator it = leptonPair.begin(); 
			it != leptonPair.end(); ++it)
	{
		unsigned int i1 = it->first;
		unsigned int i2 = it->second;
		const double deltaR = lepton[i1].DeltaR(lepton[i2]);
		deltaRMuMu[deltaR] = *it;
		const double deltaPhi = TMath::Abs(lepton[i1].DeltaPhi(lepton[i2]));
		deltaPhiMuMu[deltaPhi] = *it;
	}
	
	double minDeltaRMuMu   = (deltaRMuMu.begin())->first;
	double maxDeltaRMuMu   = (deltaRMuMu.rbegin())->first;
	double minDeltaPhiMuMu = (deltaPhiMuMu.begin())->first;
	double maxDeltaPhiMuMu = (deltaPhiMuMu.rbegin())->first;
	
	// + Fill histograms
	//   - Smallest and Largest DeltaR between 2 opposite charged muons
	_histos[fHMinDeltaRLp1Lp2]->Fill(minDeltaRMuMu,puw);
	_histos[fHMaxDeltaRLp1Lp2]->Fill(maxDeltaRMuMu,puw);
	//   - Smallest and Largest Delta Phi between 2 opposite charged muons
	_histos[fHMinDeltaPhiLp1Lp2]->Fill(minDeltaPhiMuMu,puw);
	_histos[fHMaxDeltaPhiLp1Lp2]->Fill(maxDeltaPhiMuMu,puw); 
        
        
	// + Calculate inv mass of closest pair in R plane
	// Remember map<double,pair>  (second is the pair)
	const unsigned int i1 = ((deltaRMuMu.begin())->second).first;
	const unsigned int i2 = ((deltaRMuMu.begin())->second).second;
	const double invMassMuMuH = (lepton[i1] + lepton[i2]).M();
	
	//   - Invariant mass of leptons supposedly from H
	_histos[fHHInvMassAfterJetVeto]->Fill(invMassMuMuH,puw);
	
	// Z Veto
	//-----------------------------------------------------------------
	// + Evaluate if there are some Z candidate within the mass window
	//   In that case applies the Z mass veto (NEW: now not only with
	//   the H candidate)
	std::vector<double> * auxVar= new std::vector<double>;  // Auxiliar variable to be 
	                                                        // used in the cuts
	auxVar->push_back(0.0);
	for(unsigned int i = 0; i < zcandindxpair.size(); ++i)
	{
		const unsigned iZ1 = leptonPair[zcandindxpair[i]].first;
		const unsigned iZ2 = leptonPair[zcandindxpair[i]].second;
		const double invMass = (lepton[iZ1] + lepton[iZ2]).M();
		// Check invariant mass of muons with opposite charge (outside Z)
		//------------------------------------------------------------------
		(*auxVar)[0] = invMass;
		if( (! fLeptonSelection->IsPass("ZMassWindow", auxVar)) )
		{
			return std::pair<unsigned int,float>(WHCuts::_iZMuMuInvMass,puw);
		}
	}
	FillHistoPerCut(WHCuts::_iZMuMuInvMass, puw, fsNTau);	
	
	// A W candidate not coming from the Higgs
	//-----------------------------------------------------------
	// + Extract the index of the not used lepton (a W, then)
	// We are in the exactly 3--> FIXME: it'd be possible to loose
	// that requirement, say at least 3 leptons and then we can 
	// choose if there are Z candidate, W candidates and so on?
	int iWcand = -1;
	for(unsigned int i = 0; i < lepton.size(); ++i)
	{
		if( i == i1 || i == i2 )
		{
			continue;
		}
		// NEW: Rejecting the fake electron aligned with 
		// the muon due to internal bremsstrahlung in W an Z decays
		const double dRl1 = lepton[i].DeltaR( lepton[i1] );
		const double dRl2 = lepton[i].DeltaR( lepton[i2] );
		if( dRl1 < 0.1 || dRl2 < 0.1 )
		{
			continue;
		}
		iWcand = i;
	}

	if( iWcand < 0 )
	{
		return std::pair<unsigned int,float>(WHCuts::_iWCandidate,puw);
	}
	FillHistoPerCut(WHCuts::_iWCandidate, puw, fsNTau);

	//+ Transverse mass all leptons: FIXME: has no sense: must include Jets!!
	const double met = fData->Get<float>("T_METPF_ET");
	const double phiMET = fData->Get<float>("T_METPF_Phi");
	const double pxMET = met*cos(phiMET);
	const double pyMET = met*sin(phiMET);
	TLorentzVector METV(pxMET,pyMET,0.0,met);
	TLorentzVector allleptons = (lepton[i1]+lepton[i2]+lepton[iWcand]);
	const double lEt = allleptons.Et();
	const double tMass = sqrt( lEt*lEt + met*met - 2.0*lEt*met*cos(allleptons.DeltaPhi(METV)));
	//+ Build the invariant mass of the 3leptons
	const double invMass3leptons = allleptons.M();

	//+ Ht: Sum of transverse energy of all the 3 leptons, Jets and MET
	const double Ht = lEt + met + sumEtJets;

	// + And fill the histo all histos after jet veto cut
	_histos[fHTrileptonMassAfterWCand]->Fill(invMass3leptons,puw);
	_histos[fHTransversMassAfterWCand]->Fill(tMass,puw);
	_histos[fHMETAfterWCand]->Fill(met,puw);
	_histos[fHHTAfterWCand]->Fill(Ht,puw);
	

	// Cut in DeltaR
	(*auxVar)[0] = minDeltaRMuMu;
	//------------------------------------------------------------------
	if( ! fLeptonSelection->IsPass("DeltaRMuMuCut", auxVar) )
	{
		return std::pair<unsigned int,float>(WHCuts::_iDeltaR,puw);
	}
	FillHistoPerCut(WHCuts::_iDeltaR, puw, fsNTau); 
	
	
	// MET
	//------------------------------------------------------------------
	//TMath::Min( fData->Get<float>("T_METPF_ET"), fData->Get<float>("T_METtc_ET") );  // NISHU WAY : FIXME CHANGED
	(*auxVar)[0] = met;

	if( ! fLeptonSelection->IsPass("MinMET", auxVar) ) 
	{
		return std::pair<unsigned int,float>(WHCuts::_iMET,puw);
	}
	delete auxVar;
	auxVar=0;
	
	// + Fill histos after all cuts
	_histos[fHMET]->Fill(met,puw);
	_histos[fHTrileptonMass]->Fill(invMass3leptons,puw);
	_histos[fHTransversMass]->Fill(tMass,puw);
	_histos[fHHInvMass]->Fill(invMassMuMuH,puw);
	_histos[fHHT]->Fill(Ht,puw);
	FillHistoPerCut(WHCuts::_iMET, puw, fsNTau);

	return std::pair<unsigned int,float>(WHCuts::_iNCuts,puw);
}

