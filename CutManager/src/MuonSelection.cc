
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
	bool hols=true;
	if( fabs(eta) >= etacut )
	{

		hols= false;
	}

	// Use the lowest value of the pt
	double ptcut = (*(*_cuts)[_ptCuts])[(*_cuts)[_ptCuts]->size()-1];
	if( pt <= ptcut )
	{
		hols= false;
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
unsigned int MuonSelection::SelectLeptonsCloseToPV() 
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
