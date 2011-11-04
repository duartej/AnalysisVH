
#include "MuonSelection.h"

#include "TLorentzVector.h"

#include<cmath>

bool MuonSelection::PassTriggerCuts()
{
	//Not implemented
	return true;
}

bool MuonSelection::PassEventCuts(const double & MET) const
{
	this->checkercutinit(_eventCuts);
	// Extract the index (possibly only one, all have the
	// same cut)
	double CutMinMET = (*_cuts)[_eventCuts]->at(0);
	
	return ( MET > CutMinMET );
}


bool MuonSelection::PassTopologicalCuts(const unsigned int & i,const double & pt,
		const double & eta) const
{
	this->checkercutinit(_ptCuts);
	this->checkercutinit(_etaCuts);
	// Extract the index (possibly only one, all have the
	// same cut)
	double etacut = (*_cuts)[_etaCuts]->at(0);
	// 
	if( i < (*_cuts)[_etaCuts]->size() )
	{
		etacut = (*(*_cuts)[_etaCuts])[i];
	}
	
	if( fabs(eta) >= etacut )
	{

		return false;
	}

	// Use the lowest value of the pt
	double ptcut = (*(*_cuts)[_ptCuts])[(*_cuts)[_ptCuts]->size()-1];
	if( pt <= ptcut )
	{
		return false;
	}

	return true;
}

// Specific muon pt-cuts (for the good identified-isolated muons)
bool MuonSelection::PassPtCuts(const unsigned int & nLeptons) const
{
	this->checkercutinit(_ptCuts);
	// 
	if( nLeptons != (*_cuts)[_ptCuts]->size() )
	{
		std::cerr << "MuonSelection::PassPtCuts ERROR: Have " << nLeptons 
			<< " muons to analyze but the config file only contain " 
			<< (*_cuts)[_ptCuts]->size() << " pt-cuts. Modify in consonance."
			<< "\nExiting..." << std::endl;
		exit(-1);
	}
	
	// Use the lowest value of the pt-- Ordered
        for(std::vector<int>::iterator it = _selectedGoodIdLeptons->begin(); 
			it != _selectedGoodIdLeptons->end() ; ++it)
	{
		const unsigned int i = *it;
		const double ptcut = (*(*_cuts)[_ptCuts])[i];
		if( _data->GetMuonPt()->at(i) < ptcut )
		{
			return false;
		}
	}
	// allright, all muons passed their cuts
	return true;
}



bool MuonSelection::PassIsoCuts(const double & isolation, const double & mupt,
		const double & mueta) const
{
	// Note that this cut do not depend of the muon,
	// the index of the cut is region-dependent

	// Check the cut has been configured
	this->checkercutinit(_IsoCuts);
	if( (*_cuts)[_IsoCuts]->size() < 4 )
	{
		std::cerr << "MuonSelection::PassIsoCuts ERROR: The configuration"
			<< " file does not contain the needed isolation cuts: "
			<< "MaxPTIsolationR1, MaxPTIsolationR2, MaxPTIsolationR3,"
			<< " MaxPTIsolationR4. Exiting..."  << std::endl;
		exit(-1);
	}
	const double CutMaxPTIsolationR1 = (*(*_cuts)[_IsoCuts])[0];
	const double CutMaxPTIsolationR2 = (*(*_cuts)[_IsoCuts])[1];
	const double CutMaxPTIsolationR3 = (*(*_cuts)[_IsoCuts])[2];
	const double CutMaxPTIsolationR4 = (*(*_cuts)[_IsoCuts])[3];
	//WARNING: HARDCODED limit of the eta regions and Pt
	//The eta/pt plane is divided in 4 regions and the cut on isolation
	//is different in each region
	//
	// PT ^
	//   /|\ |
	//    |  |
	//    |R1|R2
	// 20-+--+---
	//    |R3|R4
	//    +--+---> eta
	//       |
	//      1.479 
	const double etaLimit = 1.479;
	const double ptLimit  = 20.0;

	double IsoCut = -1;
	// High Pt Region:
	if( mupt > ptLimit )
	{
		// Low eta region: R1
		if( fabs(mueta) < etaLimit ) 
		{
			IsoCut = CutMaxPTIsolationR1;
		}
		// High eta region: R2
		else  
		{
			IsoCut = CutMaxPTIsolationR2;
		}
	}
	// Low Pt Region:
	else
	{
		// Low eta region: R3
		if( fabs(mueta) < etaLimit )
		{
			IsoCut = CutMaxPTIsolationR3;
		}
		// High eta region: R4
		else
		{
			IsoCut = CutMaxPTIsolationR4;
		}
	}

	return ( isolation < IsoCut );
}


bool MuonSelection::PassIdCuts(const unsigned int & i, const double & ptResolution) const
{
	// Note that this cut do not depend of the muon,
	// the index of the cut is region-dependent

	// Check the cut has been configured
	this->checkercutinit(_IdCuts);
	if( (*_cuts)[_IdCuts]->size() < 6 )
	{
		std::cerr << "MuonSelection::PassIsoCuts ERROR: The configuration"
			<< " file does not contain the needed isolation cuts:\n "
			<< "MinNValidHitsSATrk, MaxNormChi2GTrk, MinNumOfMatches,"
			<< " MinNValidPixelHitsInTrk, MinNValidHitsInTrk,"
			<< " MaxDeltaPtMuOverPtMu (in this order). Exiting..."  
			<< std::endl;
		exit(-1);
	}
	// FIXME: This has to be improved to avoid errors when introducing
	//        the cuts

	const int CutMinNValidHitsSATrk     = (*(*_cuts)[_IdCuts])[0];
	const double CutMaxNormChi2GTrk     = (*(*_cuts)[_IdCuts])[1];
	const int CutMinNumOfMatches        = (*(*_cuts)[_IdCuts])[2];
	const int CutMinNValidPixelHitsInTrk= (*(*_cuts)[_IdCuts])[3];
	const int CutMinNValidHitsInTrk     = (*(*_cuts)[_IdCuts])[4];
	const double CutMaxDeltaPtMuOverPtMu= (*(*_cuts)[_IdCuts])[5];

	bool passcutsforGlb = false;
	// If is global Muon using its ID cuts
	if( _data->IsGlobalMuon()->at(i) )
	{
		passcutsforGlb = _data->GetMuonNValidHitsSATrk()->at(i) > CutMinNValidHitsSATrk
		    && _data->GetMuonNormChi2GTrk()->at(i) < CutMaxNormChi2GTrk 
		    && _data->GetMuonNumOfMatches()->at(i) > CutMinNumOfMatches;
	}
	
	bool passcutsforSA = false;
	if( _data->IsAllTrackerMuons()->at(i) ) // Tracker muons
	{
		passcutsforSA = _data->IsTMLastStationTight()->at(i);
	}

	const bool passSpecific = passcutsforGlb || passcutsforSA;

	// Already we can go off
	if( ! passSpecific )
	{
		return false;
	}

	bool Idcuts = _data->GetMuonNValidPixelHitsInTrk()->at(i) > CutMinNValidPixelHitsInTrk 
//#ifdef MINITREES
	    && _data->GetMuonNValidHitsInTrk()->at(i) > CutMinNValidHitsInTrk 
//#endif
//#ifdef LATINOTREES
//          && _data->GetMuonInnerTrackFound()->at(i) > CutMinNValidHitsInTrk 
//#endif
           && fabs(ptResolution) < CutMaxDeltaPtMuOverPtMu;

	//Fill Histograms
	//if (fFillHistos) 
	//{
	//	fHMuonSelectionDeltaPTOverPT->Fill(ptResolution);
	//}
	
	// Remember, if you are here, passSpecific=true
	return Idcuts;
}

bool MuonSelection::PassQualityCuts(const unsigned int & i)
{
	//Not implemented 
	return true;
}
bool MuonSelection::PassUndefCuts( const unsigned int & i, const int & cutindex )
{
	//Not implemented
	return true;
}

bool MuonSelection::PassDeltaRCut( const double & minDeltaRMuMu ) const
{
	this->checkerundefcutinit(CutManager::kMaxDeltaRMuMu);

	const double CutMaxDeltaRMuMu = (*(*_undefcuts)[CutManager::kMaxDeltaRMuMu])[0];	

	return ( minDeltaRMuMu <= CutMaxDeltaRMuMu );
}

// Return true if it is inside the Z window
bool MuonSelection::PassZWindow( const double & invariantMass ) const
{
	this->checkerundefcutinit(CutManager::kMaxZMass);
	this->checkerundefcutinit(CutManager::kMinZMass);

	const double MaxZMass = (*(*_undefcuts)[CutManager::kMaxZMass])[0];	
	const double MinZMass = (*(*_undefcuts)[CutManager::kMinZMass])[0];

	return ( MaxZMass > invariantMass && invariantMass > MinZMass);
}

//FIXME: Asumo que estan ordenados por Pt!!! Commprobar
//---------------------------------------------
// Select muons
// - Return the size of the vector with the index of the muons 
//   passing our cuts (Kinematical cut -- pt and eta -- and must
//   be not standalone muon
// - Depends on fCutMinMuPt, fCutMaxMuEta 
//---------------------------------------------
unsigned int MuonSelection::SelectBasicLeptons() 
{
	//FIXME: It has to be always-- this function is called only 
	//      once -- to be checked
	if( _selectedbasicLeptons == 0 )
	{
		_selectedbasicLeptons = new std::vector<int>;
	}

	// Empty the selected muons vector --> Redundant to be removed
	_selectedbasicLeptons->clear();
	
	// Loop over muons
	for(unsigned int i=0; i < _data->GetMuonPx()->size(); ++i) 
	{
		//Build 4 vector for muon
		TLorentzVector Mu(_data->GetMuonPx()->at(i), 
				_data->GetMuonPy()->at(i), 
				_data->GetMuonPz()->at(i), 
				_data->GetMuonEnergy()->at(i));
		//Fill Histograms
		/*if(fFillHistos) 
		{
			fHMuonSelectionPT->Fill(Mu.Pt());
			fHMuonSelectionEta->Fill(Mu.Eta());
		}*/
		
		//[Cut in Eta and Pt]
		//-------------------
		//if(fabs(Mu.Eta()) >= fCutMaxMuEta) continue;
		//if (Mu.Pt()        <= fCutMinMuPt) continue;
		if( ! this->PassTopologicalCuts(i,Mu.Pt(),Mu.Eta()) )
		{
			continue;
		}
		//[If the muon is standalone, and it is neither Tracker 
		//nor Global then get rid of it]
		//-------------------
		if(  _data->IsAllStandAloneMuons()->at(i) 
				&& !_data->IsGlobalMuon()->at(i) 
				&& !_data->IsAllTrackerMuons()->at(i) )
		{
			continue; // muStaOnly 
		}
		
		// If we got here it means the muon is good
		_selectedbasicLeptons->push_back(i);
	}
	
	return _selectedbasicLeptons->size();
}
//---------------------------------------------
// Select closest muons to pv
// - Return the size of the vector with the index of the muons 
//   passing our cuts (Kinematical cut -- pt and eta -- and must
//   be not standalone muon
// - Depends on fCutMinMuPt, fCutMaxMuEta 
//---------------------------------------------
unsigned int MuonSelection::SelectLeptonsCloseToPV() 
{
	//FIXME: It has to be always-- this function is called only 
	//      once -- to be checked
	if( _closeToPVLeptons == 0)
	{
		_closeToPVLeptons = new std::vector<int>;
	}

	//Empty the vector of indices --> Redundant
	_closeToPVLeptons->clear();

	// First check is already was run over selected muons
	// if not do it
	if( _selectedbasicLeptons == 0)
	{
		this->SelectBasicLeptons();
	}

	// Assuming all the vertices are good, getting the first one:
	// the one which points more tracks to him (nombre de variable buscar)
	unsigned int iGoodVertex = 0;

	//Loop over selected muons
	for(std::vector<int>::iterator it = _selectedbasicLeptons->begin();
			it != _selectedbasicLeptons->end(); ++it)
	{
		unsigned int i = *it;

		//Build 4 vector for muon (por que no utilizar directamente Pt
		double ptMu = TLorentzVector(_data->GetMuonPx()->at(i), 
				_data->GetMuonPy()->at(i), 
				_data->GetMuonPz()->at(i), 
				_data->GetMuonEnergy()->at(i)).Pt();

		//[Require muons to be close to PV] --> FIXME: MiniTRees, buscar forma de cambiarlo...
		//-------------------
		double deltaZMu = _data->GetMuonvz()->at(i) - _data->GetVertexz()->at(iGoodVertex);
		double IPMu     = _data->GetMuonIP2DInTrack()->at(i);
/*#ifdef MINITREES
		// Next two lines for pure minitrees
		double deltaZMu = fSelector->T_Muon_vz->at(i) - fSelector->T_Vertex_z->at(iGoodVertex);
		double IPMu     = fSelector->T_Muon_IP2DInTrack->at(i);
#endif
#ifdef LATINOTREES
		// Next two lines for latinos
		double deltaZMu = 0;
		double IPMu = 0;
		// + Lara
		if (fUseBiased) {
			deltaZMu = fSelector->T_Muon_dzPVBiasedPV->at(i);
			IPMu     = fSelector->T_Muon_IP2DBiasedPV->at(i);
		}
		// + Jonatan
		else {
			deltaZMu = fSelector->T_Muon_dzPVUnBiasedPV->at(i);
			IPMu     = fSelector->T_Muon_IP2DUnBiasedPV->at(i);
		}
#endif*/
		// Checking the cuts are defined
		this->checkerundefcutinit(kMaxMuIP2DInTrackR1);
		this->checkerundefcutinit(kMaxMuIP2DInTrackR2);
		this->checkerundefcutinit(kMaxDeltaZMu);
		// Apply cut on PV depending on region
		// + R1: PT >= 20
		// + R2: PT <  20
		if(ptMu >= 20.0 && fabs(IPMu) > (*(*_undefcuts)[kMaxMuIP2DInTrackR1])[0] ) 
		{
			continue;
		}
		else if(ptMu < 20.0  && fabs(IPMu) > (*(*_undefcuts)[kMaxMuIP2DInTrackR2])[0] ) 
		{
			continue;
		}
		
		if(fabs(deltaZMu) > (*(*_undefcuts)[kMaxDeltaZMu])[0]) 
		{
			continue;
		}
		
		// If we got here it means the muon is good
		_closeToPVLeptons->push_back(i);
	}
	
	return _closeToPVLeptons->size();
}

//---------------------------------------------
// Select isolated muons
// - Returns the number of selected isolated muons
// - Depends on MaxIsoMu cut
//---------------------------------------------
unsigned int MuonSelection::SelectIsoLeptons() 
{
	//FIXME: It has to be always-- this function is called only 
	//      once -- to be checked
	if( _selectedIsoLeptons == 0)
	{
		_selectedIsoLeptons = new std::vector<int>;
	}

	//Empty the vector of indices --> Redundant
	_selectedIsoLeptons->clear();

	// First check is already was run over close to PV muons
	// if not do it
	if( _closeToPVLeptons == 0)
	{
		this->SelectLeptonsCloseToPV();
	}
	
	//Loop over selected muons
	for(std::vector<int>::iterator it = _closeToPVLeptons->begin();
			it != _closeToPVLeptons->end(); ++it)
	{
		unsigned int i = *it;
		
		//Build 4 vector for muon
		TLorentzVector Mu(_data->GetMuonPx()->at(i), 
				_data->GetMuonPy()->at(i),
				_data->GetMuonPz()->at(i), 
				_data->GetMuonEnergy()->at(i));
		
		//[Require muons to be isolated]
		//-------------------
//#ifdef MINITREES
		double isolation = (_data->GetMuonSumIsoTrack()->at(i) + 
				_data->GetMuonSumIsoCalo()->at(i)) / Mu.Pt();
//#endif
//#ifdef LATINOTREES
//		double isolation =(fSelector->T_Muon_muSmurfPF->at(i) )/ Mu.Pt();
//#endif
		
		bool isolatedMuon = this->PassIsoCuts(isolation,Mu.Pt(),Mu.Eta());
		
		if( !isolatedMuon )
		{
			continue;
		}
		
		// If we got here it means the muon is good
		_selectedIsoLeptons->push_back(i);
	}
	
	return _selectedIsoLeptons->size();
}

//---------------------------------------------
// Select isolated good muons
// - Returns the number of selected isolated good muons
// - No dependencies: FIX CUTS!!!
//---------------------------------------------
unsigned int MuonSelection::SelectGoodIdLeptons()
{
	//FIXME: It has to be always-- this function is called only 
	//      once -- to be checked
	if( _selectedGoodIdLeptons == 0)
	{
		_selectedGoodIdLeptons = new std::vector<int>;
	}

	//Empty the vector of indices --> Redundant
	_selectedGoodIdLeptons->clear();

	// First check is already was run over close Iso muons
	// if not do it
	if( _selectedIsoLeptons == 0)
	{
		this->SelectIsoLeptons();
	}
	
	//Loop over selected muons
	for(std::vector<int>::iterator it = _selectedIsoLeptons->begin();
			it != _selectedIsoLeptons->end(); ++it)
	{
		const unsigned int i = *it;
	
		double ptResolution = _data->GetMuondeltaPt()->at(i)/
			_data->GetMuonPt()->at(i);
	        //Lepton ID
		if( ! this->PassIdCuts(i,ptResolution) )
		{
			continue;
		}
		// If we got here it means the muon is good
		_selectedGoodIdLeptons->push_back(i);
  }

  return _selectedGoodIdLeptons->size();
}

