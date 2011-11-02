
#include "MuonSelection.h"

#include "TLorentzVector.h"

#include<cmath>

bool MuonSelection::PassTriggerCuts()
{
}

bool MuonSelection::PassEventCuts()
{
}


bool MuonSelection::PassTopologicalCuts(const unsigned int & i,const double & pt,
		const double & eta) const
{
	checkercutinit(_ptCuts);
	checkercutinit(_etaCuts);
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



bool MuonSelection::PassIsoCuts(const unsigned int & i)
{
}
bool MuonSelection::PassIdCuts(const unsigned int & i)
{
}
bool MuonSelection::PassQualityCuts(const unsigned int & i)
{
}
bool MuonSelection::PassUndefCuts( const unsigned int & i, const int & cutindex )
{
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
	//First event
	if( _selectedbasicLeptons == 0 )
	{
		_selectedbasicLeptons = new std::vector<int>;
	}

	// Empty the selected muons vector
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
unsigned int MuonSelection::SelectMuonsCloseToPV() 
{
	//First event
	if( _closeToPVLeptons == 0)
	{
		_closeToPVLeptons = new std::vector<int>;
	}

	//Empty the vector of indices
	_closeToPVLeptons->clear();

	// First check is already was run over selected muons
	// if not do it
	if( _selectedbasicLeptons == 0)
	{
		this->SelectBasicLeptons();
	}

	//Loop over selected muons
	for(std::vector<int>::iterator it = _selectedbasicLeptons->begin();
			it != _selectedbasicLeptons->end(); ++it)
	{
		unsigned int i = *it;

		//Build 4 vector for muon
		double ptMu = TLorentzVector(_data->GetMuonPx()->at(i), 
				_data->GetMuonPy()->at(i), 
				_data->GetMuonPz()->at(i), 
				_data->GetMuonEnergy()->at(i)).Pt();

		//[Require muons to be close to PV]
		//-------------------
		double deltaZMu = fData->GetMuonvz()->at(i) - fData->GetVertexz()->at->at(iGoodVertex)
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
		// Apply cut on PV depending on region
		// + R1: PT >= 20
		// + R2: PT <  20
		if (ptMu >= 20 && fabs(IPMu) > fCutMaxMuIP2DInTrackR1) 
			continue;
		else if (ptMu < 20  && fabs(IPMu) > fCutMaxMuIP2DInTrackR2) 
			continue;
		
		if (fabs(deltaZMu) > fCutMaxDeltaZMu) continue;
		
		
		// If we got here it means the muon is good
		fSelectedPVMuons->push_back(i);
	}
	
	return fSelectedPVMuons->size();
}
