
#include "AnalysisWHeee.h"

#include "PUWeight.h"

#include "TH1D.h"

// To be changed: FIXME
const double kZMass = 91.1876;
//const unsigned int kNMuons = 3; 
const unsigned int kNMuons = 2; 
const unsigned int kWPID   = 24; //Found with TDatabasePDG::Instance()->GetParticle("W+")->PdgCode()
const unsigned int kTauPID = 15; //Found with TDatabasePDG::Instance()->GetParticle("tau-")->PdgCode()

void AnalysisWHeee::FillHistoPerCut(const ECutLevel & cut,const double & puw, const unsigned int & fs) 
{
	_histos[fHEventsPerCut]->Fill(cut, puw);
	if(fs == _iFSeee)
	{
		_histos[fHEventsPerCut3Lepton]->Fill(cut, puw);
	}
}

void AnalysisWHeee::FillGenPlots(ECutLevel cut, double puw) 
{
	if (fIsWH && fNGenElectrons == _nLeptons) 
	{
		for (unsigned int i = 0; i < fNGenElectrons; i++) 
		{
			//fHGenPtLepton[i][cut]->Fill((*fGenLepton)[i].Pt(), puw);
			//fHGenEtaLepton[i][cut]->Fill((*fGenLepton)[i].Eta(), puw);
			fHGenPtLepton[i][cut]->Fill(fGenLepton[i].Pt(), puw);
			fHGenEtaLepton[i][cut]->Fill(fGenLepton[i].Eta(), puw);
		}
	}
}

//---------------------------------------------------------------------
// InsideLoop
//---------------------------------------------------------------------
void AnalysisWHeee::InsideLoop()
{
#ifdef DEBUGANALYSIS
	std::cout << "========================================================" << std::endl;
	std::cout << "New event: " << fData->GetEventEventNumber() << std::endl;
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
		
		_histos[fHNGenWLeptons]->Fill(nmusfromW,puw); 
		fsTaus = GetFSID(nelecsfromW, nmusfromW, ntausfromW);
		_histos[fHGenFinalState]->Fill(fsTaus, puw);
		
#ifdef DEBUGANALYSIS
		std::cout << "DEBUG: W->e/mu/tau " << nelecsfromW 
			<< "/" << nmusfromW << "/" << ntausfromW << std::endl;
		std::cout << "DEBUG: fsTaus --> " << fsTaus << std::endl;
#endif
		int igenelecs[3] = {-1, -1, -1 };
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
				igenelecs[fNGenElectrons] = i;
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
			b
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
		// Initialize generation vector
		fGenLepton.clear();
		//   Sort by energy
		if(fNGenElectrons == 3)  // FIXME:: PORQUE SOLO 3??
		{
			std::map<double,TLorentzVector> vOrder;
			std::vector<TLorentzVector> * vGenMuon = new std::vector<TLorentzVector>;
			for(unsigned int i = 0; i < fNGenElectrons; i++) 
			{
				vGenMuon->push_back( TLorentzVector(fData->GetGenElecPx()->at(igenelecs[i]), 
							fData->GetGenElecPy()->at(igenelecs[i]), 
							fData->GetGenElecPz()->at(igenelecs[i]),
							fData->GetGenElecEnergy()->at(igenelecs[i])) );
				vOrder[vGenMuon->back().Pt()] = vGenMuon->back();
			}
			//fGenLepton = new std::vector<TLorentzVector>;
			for(std::map<double,TLorentzVector>::reverse_iterator it = vOrder.rbegin(); 
					it != vOrder.rend(); ++it)
			{
				fGenLepton.push_back( it->second );
			}
			delete vGenMuon;
			vGenMuon = 0;
			
#ifdef DEBUGANALYSIS
			for (unsigned int i = 0; i < fNGenElectrons; ++i) 
			{
				std::cout << "[" << i << "] <- [" << igenelecs[i] 
					//<< "] PT = " << (*fGenMuon)[i].Pt() 
					<< "] PT = " << fGenElec[i].Pt() 
					<< " - ET = " << fData->GetGenElecEnergy()->at(igenelecs[i]) 
					<< std::endl;
			}      
#endif
			FillGenPlots(_iAllEvents,puw);
			
			// Freeing memory
			//delete fGenLepton;
			//fGenLepton = 0;
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

	if(fIsWH && (procn != _iWH || fsNTau != _iFSeee))
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
	_histos[fHNRecoLeptons]->Fill(fData->GetElecPx()->size());
	
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
	std::cout << "Numbre of Electrons: " << fData->GetElecPx()->size() << std::endl;
#endif 
	for(std::vector<int>::iterator it = theLeptons->begin(); it != theLeptons->end(); ++it) 
	{
		unsigned int i = *it;
		/*TLorentzVector vL(fData->GetElecPx()->at(i), 
				fData->GetElecPy()->at(i),
				fData->GetElecPz()->at(i), 
				fData->GetElecEnergy()->at(i)); */
		const TLorentzVector vL = this->GetTLorentzVector(ELECTRON,i);
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
	for(std::vector<int>::iterator it = theLeptons->begin(); it != theLeptons->end(); ++it) 
	{
		const unsigned int i = *it;
		/*lepton.push_back( TLorentzVector(fData->GetElecPx()->at(i), 
					fData->GetElecPy()->at(i), 
					fData->GetElecPz()->at(i), 
					fData->GetElecEnergy()->at(i))
				); */
		lepton.push_back( this->GetTLorentzVector(ELECTRON,i) );
		const char * charge = "T_Elec_Charge";
		leptonCharge.push_back( fData->Get<int>(charge,i) );
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
	/*for(std::vector<int>::iterator it = theLeptons->begin(); it != theLeptons->end(); ++it) 
	{*/
	for(std::vector<int>::iterator it = leptonCharge.begin(); it != leptonCharge.end(); ++it)
	{
		//unsigned int i = *it;
		//charge += fData->GetElecCharge()->at(i);
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
	//unsigned int ipair = 0;
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
	/*	TLorentzVector Jet(fData->GetJetAKPFNoPUPx()->at(k), 
				fData->GetJetAKPFNoPUPy()->at(k), 
				fData->GetJetAKPFNoPUPz()->at(k), 
				fData->GetJetAKPFNoPUEnergy()->at(k));*/
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
	const double met = fData->GetMETPFET();
	_histos[fHMET]->Fill(met,puw);
	(*auxVar)[0] = met;
	if( ! fLeptonSelection->IsPass("MinMET", auxVar) ) 
	{
		return;
	}
	delete auxVar;
	auxVar=0;
	
	FillHistoPerCut(_iMET, puw, fsNTau);
}

