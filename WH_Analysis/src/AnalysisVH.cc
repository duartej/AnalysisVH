#include<sstream>
#include<stdlib.h>
#include<cstring>
#include<iostream>

#include "AnalysisVH.h"
#include "InputParameters.h"
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


// Prepare analysis Constructor
AnalysisVH::AnalysisVH(TreeManager * data, InputParameters * ip, 
		CutManager * selectioncuts, const unsigned int & finalstate ) :
	CMSAnalysisSelector(data),
	_nLeptons(3), //FIXME: argumento de entrada ? --> No dependera de la SignatureFS
	fIsData(false),
	fIsWH(false),
	fLuminosity(0),
	fFS(999),
	fLeptonName(0),
	fNGenElectrons(0),
	fNGenMuons(0),
	fNGenLeptons(0),
	fPUWeight(0)
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

	// The signature of the analysis
	fFS = finalstate;
	// And the link to the Gen we want to use
	if( fFS == SignatureFS::_iFSmmm )
	{
		fLeptonType = MUON;
		fLeptonName = "Muon";
		fNGenLeptons = &fNGenMuons;
	}
	else if( fFS == SignatureFS::_iFSeee )
	{
		fLeptonType = ELECTRON;
		fLeptonName = "Elec";
		fNGenLeptons = &fNGenElectrons;
	}
	// else ???? Ya deberia estar controlado antes
	// anyway FIXME: meter aqui un exit--->>
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

	// FIXME: I need a method to checked that the cut
	// is really in the configuration file
	//Cuts
	//---- FIXME: Recupera las explicaciones
	std::vector<std::string> cuts;
	//   - Pt and Eta of muons
	cuts.push_back("MinMuPt1");
	cuts.push_back("MinMuPt2");
	cuts.push_back("MinMuPt3");
	cuts.push_back("MaxAbsEta");
	//   - IP and DeltaZ of track associated with muon w.r.t PV
	cuts.push_back("MaxMuIP2DInTrackR1");
	cuts.push_back("MaxMuIP2DInTrackR2");
	cuts.push_back("MaxDeltaZMu") ;
  	//   - Isolation: (PTtraks + ETcalo)/PTmuon: different regions
	cuts.push_back("MaxPTIsolationR1");
	cuts.push_back("MaxPTIsolationR2");
	cuts.push_back("MaxPTIsolationR3");
	cuts.push_back("MaxPTIsolationR4");
	cuts.push_back("MaxIsoMu");  // OBSOLETE--> Now in regions
	//   - Quality and Identification
	cuts.push_back("MinNValidHitsSATrk");
	cuts.push_back("MaxNormChi2GTrk");
	cuts.push_back("MinNumOfMatches");
	cuts.push_back("MinNValidPixelHitsInTrk");
	cuts.push_back("MinNValidHitsInTrk");
	cuts.push_back("MaxDeltaPtMuOverPtMu");
	//   - Max DeltaR between muons
	cuts.push_back("MaxDeltaRMuMu");
  	//   - Min MET of the event
	cuts.push_back("MinMET");

	// Now including all the cuts to the manager
	double dummy = 0;
	for(std::vector<std::string>::iterator it = cuts.begin();
			it != cuts.end(); ++it)
	{
		ip->TheNamedDouble(it->c_str(), dummy);
		fLeptonSelection->SetCut(it->c_str(),dummy);
		dummy = 0;
	}
	
	//   - Z mass window
	double deltazmass=0;
	ip->TheNamedDouble("DeltaZMass", deltazmass);
	fLeptonSelection->SetCut("MaxZMass",kZMass+deltazmass);
	fLeptonSelection->SetCut("MinZMass",kZMass-deltazmass);


	// All the cuts introduced, locking up to fix them
	fLeptonSelection->LockCuts();
	
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
	std::cout << *fLeptonSelection << std::endl;
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

	// Generated muons coming from a W
	_histos[fHNGenWLeptons] = CreateH1D("fHNGenWLeptons", "N Gen #mu from W", 5, -0.5, 4.5);
	
	// PT and Eta of most energetic gen muon from W or tau
	for(unsigned int i = 0; i < _nLeptons; i++) 
	{
		for (unsigned int j = 0; j < _iNCuts; j++) 
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
			_iNCuts, 0, _iNCuts);
	for (unsigned int i = 0; i < _iNCuts; i++)
	{
		_histos[fHEventsPerCut]->GetXaxis()->SetBinLabel(i+1,kCutNames[i]);
	}
  
	// Events passing every cut that are 3 mu from gen
	_histos[fHEventsPerCut3Lepton] = CreateH1D("fHEventsPerCut3Lepton", "Events passing each cut that are 3 mu from gen", 
			_iNCuts, 0, _iNCuts);
	for(unsigned int i = 0; i < _iNCuts; i++)
	{
		_histos[fHEventsPerCut3Lepton]->GetXaxis()->SetBinLabel(i+1,kCutNames[i]);
	}

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
	
	// Selected Isolated Good Muons
	_histos[fHLeptonCharge] = CreateH1D("fHLeptonCharge", "#Sum q_{#mu}", 7, -3.5, 3.5);
		
	// Invariant mass of leptons supposedly from H  
	_histos[fHHInvMass] = CreateH1D("fHHInvMass", "M^{inv.}_{#mu#mu}", 150, 0, 150);
	
	// Invariant mass of leptons in/out of Z peak
	_histos[fHZInvMass] = CreateH1D("fHZInvMass", "M^{inv.}_{#mu#mu}",150, 0, 150);
	
	// Missing ET after inv mass cut
	_histos[fHMET] = CreateH1D("fHMET", "MET",160, 0, 160);
	
}

const TLorentzVector AnalysisVH::GetTLorentzVector( const int & index) const
{
	const std::string px("T_"+std::string(fLeptonName)+"_Px");
	const std::string py("T_"+std::string(fLeptonName)+"_Py");
	const std::string pz("T_"+std::string(fLeptonName)+"_Pz");
	const std::string energy("T_"+std::string(fLeptonName)+"_Energy");

	return TLorentzVector( fData->Get<float>(px.c_str(),index),
			fData->Get<float>(py.c_str(),index),
			fData->Get<float>(pz.c_str(),index),
			fData->Get<float>(energy.c_str(),index)
			);
}

// Overloaded function to extract 4-moments of other than leptons
const TLorentzVector AnalysisVH::GetTLorentzVector( const char * name, const int & index) const
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


//---------------------------------------------------------------------
// InsideLoop
//---------------------------------------------------------------------
void AnalysisVH::InsideLoop()
{
#ifdef DEBUGANALYSIS
	std::cout << "========================================================" << std::endl;
	std::cout << "New event: " << fData->Get<int>("T_Event_EventNumber") << std::endl;
#endif
	// Get PU Weight
	//----------------------------------------------------------------------
	double puw(1);
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
	if(fIsWH) 
	{
		// + Classify by leptonic final state (taus undecayed)
		unsigned int nelecsfromW = fData->GetSize<int>("T_Gen_ElecSt3_PID");
		unsigned int nmusfromW = fData->GetSize<int>("T_Gen_MuonSt3_PID");
		unsigned int ntausfromW = fData->GetSize<int>("T_Gen_TauSt3_PID");
		
		_histos[fHNGenWLeptons]->Fill(nmusfromW,puw); 
		fsTaus = SignatureFS::GetFSID(nelecsfromW, nmusfromW, ntausfromW);
		_histos[fHGenFinalState]->Fill(fsTaus, puw);
		
#ifdef DEBUGANALYSIS
		std::cout << "DEBUG: W->e/mu/tau " << nelecsfromW 
			<< "/" << nmusfromW << "/" << ntausfromW << std::endl;
		std::cout << "DEBUG: fsTaus --> " << fsTaus << std::endl;
#endif
		int igen[3] = {-1, -1, -1 };
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
			unsigned int mpidel  = TMath::Abs(fData->Get<int>("T_Gen_Elec_MPID",i));
			if ( mpidel == kWPID || mpidel == kTauPID) 
			{
				if( fLeptonType == ELECTRON )
				{
					igen[fNGenElectrons] = i;
				}
				fNGenElectrons++;
#ifdef DEBUGANALYSIS
				std::cout << "DEBUG:     Good!" << std::endl;
#endif
			}
		}
		
		for (unsigned int i = 0; i < fData->GetSize<int>("T_Gen_Muon_MPID"); ++i) 
		{
#ifdef DEBUGANALYSIS
			std::cout << "DEBUG: Muon [" << i << "/" << fData->GetSize<int>("T_Gen_Muon_MPID") << "]" 
				<< " Status: " << fData->Get<int>("T_Gen_Muon_MSt",i)
				<< " Mother: " << fData->Get<int>("T_Gen_Muon_MPID",i)
				<< " E: " << fData->Get<int>("T_Gen_Muon_Energy",i)
				<< " Px: " << fData->Get<int>("T_Gen_Muon_Px",i)
				<< std::endl;
			b
#endif
			//I have seen that muons from taus have status 2 and are not replicated
			unsigned int mpid  = TMath::Abs(fData->Get<int>("T_Gen_Muon_MPID",i));
			if ( mpid == kWPID || mpid == kTauPID) 
			{
				if( fLeptonType == MUON )
				{
					igen[fNGenMuons] = i;
				}
				fNGenMuons++;
#ifdef DEBUGANALYSIS
				std::cout << "DEBUG:     Good! " << fNGenMuons << std::endl;
#endif
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
		//   Sort by energy
		if(*fNGenLeptons == 3)  // FIXME:: PORQUE SOLO 3??
		{
			std::map<double,TLorentzVector> vOrder;
			std::vector<TLorentzVector> * vGenMuon = new std::vector<TLorentzVector>;
			std::string genname = std::string("Gen_"+std::string(fLeptonName));
			for(unsigned int i = 0; i < *fNGenLeptons; i++) 
			{
				vGenMuon->push_back( this->GetTLorentzVector( genname.c_str(),igen[i]) );
				vOrder[vGenMuon->back().Pt()] = vGenMuon->back();
			}
			for(std::map<double,TLorentzVector>::reverse_iterator it = vOrder.rbegin(); 
					it != vOrder.rend(); ++it)
			{
				fGenLepton.push_back( it->second );
			}
			delete vGenMuon;
			vGenMuon = 0;
			
#ifdef DEBUGANALYSIS
			for (unsigned int i = 0; i < *fNGenLeptons; ++i) 
			{
				std::cout << "Lepton:" << genname << " [" << i << "] <- [" << igen[i] 
					<< "] PT = " << fGenLepton[i].Pt() 
					<< " - ET = " << fGenLepton[i].Energy() 
					<< std::endl;
			}      
#endif
			FillGenPlots(_iAllEvents,puw);
		}
	}
	
	// All events
	//------------------------------------------------------------------
	FillHistoPerCut(_iAllEvents, puw, fsNTau);
  
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
		return;
	}
	
	FillHistoPerCut(_iIsWH, puw, fsNTau);

	// HLT: TBD...
	//------------------------------------------------------------------
	FillHistoPerCut(_iHLT, puw, fsNTau);

	// Vertex cut (Event stuff)-- OBSOLETE (implemented per default) SURE?
	//int iGoodVertex = GoodVertex();
	//int iGoodVertex = 0; // First one
	//fLeptonSelection->PassEventsCuts();
	//if( iGoodVertex < 0)
	//{
	//	return;
	//}
	FillHistoPerCut(_iGoodVertex, puw, fsNTau);
	

	// First looking at events with 2 leptons
	//const int kNMuons = 2; --> Defined in the preamble
	// Muon selection
	//------------------------------------------------------------------
	//
	//this->SetGoodVertexIndex(iGoodVertex);
	
	// Store the number of reconstructed muons without any filter
	std::string leptonpx = std::string("T_"+std::string(fLeptonName)+"_Px");
	_histos[fHNRecoLeptons]->Fill(fData->GetSize<float>(leptonpx.c_str()));//GetElecPx()->size());
	
	// (1) Basic selection
	//--------------------
	unsigned int nSelectedMuons = fLeptonSelection->GetNBasicLeptons();
	_histos[fHNSelectedLeptons]->Fill(nSelectedMuons,puw);

	if(nSelectedMuons < kNMuons)
	{
		return;
	}
	
	FillHistoPerCut(_iHas2Leptons, puw, fsNTau);
	
	// (2) PV selection
	//--------------------
	unsigned int nSelectedPVMuons = fLeptonSelection->GetNLeptonsCloseToPV();
	_histos[fHNSelectedPVLeptons]->Fill(nSelectedPVMuons,puw); 
	
	if(nSelectedPVMuons < kNMuons)
	{
		return;
	}

	FillHistoPerCut(_iHas2PVLeptons, puw, fsNTau);


	// (3) Isolated muons
	//--------------------
	unsigned int nSelectedIsoMuons = fLeptonSelection->GetNIsoLeptons();
	_histos[fHNSelectedIsoLeptons]->Fill(nSelectedIsoMuons,puw);  
	
	if(nSelectedIsoMuons < kNMuons)
	{
		return;
	}
	
	FillHistoPerCut(_iHas2IsoLeptons, puw, fsNTau);
	
	// (4) Isolated good muons: Identification
	//--------------------
	unsigned int nSelectedIsoGoodMuons = fLeptonSelection->GetNGoodIdLeptons();
	_histos[fHNSelectedIsoGoodLeptons]->Fill(nSelectedIsoGoodMuons,puw);
	
	if(nSelectedIsoGoodMuons < kNMuons)
	{
		return;
	}
	
	FillHistoPerCut(_iHas2IsoGoodLeptons, puw, fsNTau);	
      
	// The leading lepton has to have PT > fCutMinMuPt (20 most probably)
	// Applying the pt-cuts
	//-------------------------------------------------------------------
	
	if( ! fLeptonSelection->IsPass("PtMuonsCuts") )
	{
		return;
	}
	FillHistoPerCut(_iMuPTPattern, puw, fsNTau);
	
	// Keep events with just 3 leptons and store momentum and charge
	//---------------------------------------------------------------------------
	if(nSelectedIsoGoodMuons != _nLeptons)
	{
		return;
	}
	// Indexs of good leptons
	std::vector<int> * theLeptons = fLeptonSelection->GetGoodLeptons(); 
	// + Fill histograms with Pt and Eta
	int k = 0;  // Note that k is the index of the vectors, not the TTree
#ifdef DEBUGANALYSIS
	std::cout << "Numbre of " << fLeptonName << ": " << fData->GetSize<float>(leptonpx.c_str()) << std::endl; //GetElecPx()->size() << std::endl;
#endif 
	for(std::vector<int>::iterator it = theLeptons->begin(); it != theLeptons->end(); ++it) 
	{
		unsigned int i = *it;
		const TLorentzVector vL = this->GetTLorentzVector(i);
		fHPtLepton[k]->Fill(vL.Pt(), puw);
		fHEtaLepton[k]->Fill(vL.Eta(), puw);
		if( fGenLepton.size() != 0 )
		{
			fHDeltaRGenRecoLepton[k]->Fill(vL.DeltaR(fGenLepton.at(k)), puw);
		}
		k++;
	}
	FillGenPlots(_iHasExactly3Leptons,puw);
	
	FillHistoPerCut(_iHasExactly3Leptons, puw, fsNTau);
	// + Store Momentum and charge for the muons : FIXME: This loop inside the
	// last one
	std::vector<TLorentzVector> lepton;
	std::vector<int> leptonCharge;
	const std::string chargedm("T_"+std::string(fLeptonName)+"_Charge");
	for(std::vector<int>::iterator it = theLeptons->begin(); it != theLeptons->end(); ++it) 
	{
		const unsigned int i = *it;
		lepton.push_back( this->GetTLorentzVector(i) );
		leptonCharge.push_back( fData->Get<int>(chargedm.c_str(),i) );
	}
	
	// Discard extra electrons and extra muons
	// ------------------------------------------------------------------
	// 
	// Not yet implemented...
  
	// Discard 3 muons with the same charge
	// ------------------------------------------------------------------
	// 
	// + Add up charges. If the abs value of the total number is equal to N then 
	//   all have the same sign
	int charge = 0;
	for(std::vector<int>::iterator it = leptonCharge.begin(); it != leptonCharge.end(); ++it)
	{
		charge += *it;
	}
	// Fill muon charge before rejecting or accepting  
	_histos[fHLeptonCharge]->Fill(charge, puw);
	
	if( (unsigned int)TMath::Abs(charge) == theLeptons->size() )
	{
		return;
	}
	
	FillHistoPerCut(_iOppositeCharge, puw, fsNTau);
	
	// Find muons with opposite charges and calculate DeltaR, invMass...
	// Keep the pair with DeltaR minimum
	// ------------------------------------------------------------------
	// 
	// + Store the real vector index of the opposite charge muons in pairs
	//   (for the lepton and leptonCharge vectors: 0,...,nLeptons)
	std::vector<std::pair<int,int> > leptonPair;
	for(unsigned int kbegin = 0; kbegin < leptonCharge.size(); ++kbegin)
	{
		for(unsigned int kfromend = leptonCharge.size()-1; kfromend > kbegin; --kfromend) 
		{
			if( leptonCharge[kbegin]*leptonCharge[kfromend] < 0 )
			{
				leptonPair.push_back( std::pair<int,int>(kbegin,kfromend) );
			}
		}
	}
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
        
        // + Calculate inv mass of closest pair in R plane
	// Remember map<double,pair>  (second is the pair)
	const unsigned int i1 = ((deltaRMuMu.begin())->second).first;
	const unsigned int i2 = ((deltaRMuMu.begin())->second).second;
	const double invMassMuMuH = (lepton[i1] + lepton[i2]).M();

	// + Fill histograms
	//   - Smallest and Largest DeltaR between 2 opposite charged muons
	_histos[fHMinDeltaRLp1Lp2]->Fill(minDeltaRMuMu,puw);
	_histos[fHMaxDeltaRLp1Lp2]->Fill(maxDeltaRMuMu,puw);
	//   - Smallest and Largest Delta Phi between 2 opposite charged muons
	_histos[fHMinDeltaPhiLp1Lp2]->Fill(minDeltaPhiMuMu,puw);
	_histos[fHMaxDeltaPhiLp1Lp2]->Fill(maxDeltaPhiMuMu,puw);
	//   - Invariant mass of leptons supposedly from H
	_histos[fHHInvMass]->Fill(invMassMuMuH,puw);
  
	
	// Jet Veto:
	//------------------------------------------------------------------
	unsigned int nJets = 0;
	//for(unsigned int k = 0; k < fData->GetJetAKPFNoPUEnergy()->size(); ++k) 
	for(unsigned int k = 0; k < fData->GetSize<float>("T_JetAKPFNoPU_Energy"); ++k) 
	{
		TLorentzVector Jet = this->GetTLorentzVector("JetAKPFNoPU",k);
		//FIXME: Add the cuts to the config file
		if(Jet.Pt() > 30 && fabs(Jet.Eta()) < 5  && fabs(Jet.DeltaR(lepton[0])) > 0.3 
				&& fabs(Jet.DeltaR(lepton[1])) > 0.3 && 
				fabs(Jet.DeltaR(lepton[2])) > 0.3) 
		{
			nJets++;
		}
	}
	if(nJets > 0)
	{
		return;
	}
	FillHistoPerCut(_iJetVeto, puw, fsNTau);
  	
	// Cut in DeltaR
	std::vector<double> * auxVar= new std::vector<double>;
	auxVar->push_back(minDeltaRMuMu);
	//------------------------------------------------------------------
	if( ! fLeptonSelection->IsPass("DeltaRMuMuCut", auxVar) )
	{
		return;
	}
	
	FillHistoPerCut(_iDeltaR, puw, fsNTau);
  	
	// Check invariant mass of muons with opposite charge (outside Z)
	//------------------------------------------------------------------
	(*auxVar)[0] = invMassMuMuH;
	if( ! fLeptonSelection->IsPass("ZMassWindow", auxVar) )
	{
		return;
	}
	
	_histos[fHZInvMass]->Fill(invMassMuMuH,puw);
	FillHistoPerCut(_iZMuMuInvMass, puw, fsNTau);
	
	// MET
	//------------------------------------------------------------------
	const double met = fData->Get<float>("T_METPF_ET");
	(*auxVar)[0] = met;
	if( ! fLeptonSelection->IsPass("MinMET", auxVar) ) 
	{
		return;
	}
	delete auxVar;
	auxVar=0;
	
	_histos[fHMET]->Fill(met,puw);
	FillHistoPerCut(_iMET, puw, fsNTau);
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
			<< 100.0*_histos[fHEventsPerCut]->GetBinContent(i+1)/_histos[fHEventsPerCut]->GetBinContent(1)
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
	if(fs == fFS)
	{
		_histos[fHEventsPerCut3Lepton]->Fill(cut, puw);
	}
}

void AnalysisVH::FillGenPlots(ECutLevel cut, double puw) 
{
	if (fIsWH && (*fNGenLeptons) == _nLeptons) 
	{
		for(unsigned int i = 0; i < (*fNGenLeptons); i++) 
		{
			fHGenPtLepton[i][cut]->Fill(fGenLepton[i].Pt(), puw);
			fHGenEtaLepton[i][cut]->Fill(fGenLepton[i].Eta(), puw);
		}
	}
}

