
#include "ElecSelection.h"

#include "TLorentzVector.h"

#include<cmath>


//Constructor
ElecSelection::ElecSelection( TreeManager * data, const int & nLeptons) : 
	CutManager(data,nLeptons),
	kMinMuPt1(-1),
	kMinMuPt2(-1),      
	kMinMuPt3(-1),          
	kMaxAbsEta(-1),         		
	kMaxMuIP2DInTrackR1(-1),
	kMaxMuIP2DInTrackR2(-1), 		
	kMaxDeltaZMu(-1), 
	kMaxDeltaRMuMu(-1),
	kMinMET(-1),
	kDeltaZMass(-1),
	kMaxZMass(-1),
	kMinZMass(-1),
	kMaxPTIsolationR1(-1),
	kMaxPTIsolationR2(-1),
	kMaxPTIsolationR3(-1),
	kMaxPTIsolationR4(-1),
	kMinNValidHitsSATrk(-1),
	kMaxNormChi2GTrk(-1),
	kMinNumOfMatches(-1),
	kMinNValidPixelHitsInTrk(-1),
	kMinNValidHitsInTrk(-1),
	kMaxDeltaPtMuOverPtMu(-1)  
{ 
	// Initialize the selection codenames
	_codenames.insert("PtMuonsCuts");
	_codenames.insert("DeltaRMuMuCut");
	_codenames.insert("ZMassWindow");
	_codenames.insert("MinMET");
}


void ElecSelection::LockCuts(){
	// initializing the cuts: Note that before use some cut
	// it has to be checked if it was initialized
	for(std::map<std::string,double>::iterator cut = _cuts->begin(); 
			cut != _cuts->end();++cut)
	{
		if( cut->first == "MinMuPt1" )
		{
			kMinMuPt1 = cut->second;
		}
		else if( cut->first == "MinMuPt2" )
		{
			kMinMuPt2 = cut->second;
		}
		else if( cut->first == "MinMuPt3" )
		{
			kMinMuPt3 = cut->second;
		}
		else if( cut->first == "MaxAbsEta" )
		{
			kMaxAbsEta = cut->second;
		}
		else if( cut->first == "MaxMuIP2DInTrackR1" )
		{
			kMaxMuIP2DInTrackR1 = cut->second;
		}
		else if( cut->first == "MaxMuIP2DInTrackR2" )
		{
			kMaxMuIP2DInTrackR2 = cut->second;
		}
		else if( cut->first == "MaxDeltaZMu" )
		{
			kMaxDeltaZMu = cut->second;
		}
		else if( cut->first == "DeltaZMass" )
		{
			kMaxDeltaZMu = cut->second;
		}
		else if( cut->first == "MaxDeltaRMuMu" )
		{
			kMaxDeltaRMuMu = cut->second;
		}
		else if( cut->first == "MinMET" )
		{
			kMinMET = cut->second;
		}
		else if( cut->first == "MaxPTIsolationR1" )
		{
			kMaxPTIsolationR1 = cut->second;
		}
		else if( cut->first == "MaxPTIsolationR2" )
		{
			kMaxPTIsolationR2 = cut->second;
		}
		else if( cut->first == "MaxPTIsolationR3" )
		{
			kMaxPTIsolationR3 = cut->second;
		}
		else if( cut->first == "MaxPTIsolationR4" )
		{
			kMaxPTIsolationR4 = cut->second;
		}
		else if( cut->first == "MinNValidHitsSATrk" )
		{
			kMinNValidHitsSATrk = (int)cut->second;
		}
		else if( cut->first == "MaxNormChi2GTrk" )
		{
			kMaxNormChi2GTrk = cut->second;
		}
		else if( cut->first == "MinNumOfMatches" )
		{
			kMinNumOfMatches = (int)cut->second;
		}
		else if( cut->first == "MinNValidPixelHitsInTrk" )
		{
			kMinNValidPixelHitsInTrk = (int)cut->second;
		}
		else if( cut->first == "MinNValidHitsInTrk" )
		{
			kMinNValidHitsInTrk = (int)cut->second;
		}
		else if( cut->first == "MaxDeltaPtMuOverPtMu" )
		{
			kMaxDeltaPtMuOverPtMu = cut->second;
		}
		else if( cut->first == "MaxZMass" )
		{
			kMaxZMass = cut->second;
		}
		else if( cut->first == "MinZMass" )
		{
			kMinZMass = cut->second;
		}
		/*else --> Noooo, pues esta funcion se utiliza tambien
		           para recibir otros cortes genericos
		{
			std::cerr << "ElecSelection::LockCuts ERROR:"
				<< " The cut named '" << cut->second << "' is not"
				<< " implemented. It has to be included in this method"
				<< " if you want to use it. Exiting..."
				<< std::endl;
			exit(-1);
		}*/
	}
}

// Helper function
std::vector<std::string> ElecSelection::GetCodenames() const
{
	return std::vector<std::string>(_codenames.begin(),_codenames.end());
}

bool ElecSelection::IsPass(const std::string & codename, const std::vector<double> * varAux ) const
{
	// Checking
	if( _codenames.count( codename ) != 1 )
	{
		std::cerr << "ElecSelection::IsPass ERROR: "
			<< "There is no selection chain called '" << codename
			<< "' implemented. The available selection names are:"
			<< std::endl;
		std::vector<std::string> v = this->GetCodenames();
		for(unsigned int i = 0; i < v.size();++i)
		{
			std::cout << " +" << v[i] << std::endl;
		}
		exit(-1);
	}

	bool ispass = false;
	if( codename == "PtMuonsCuts" )
	{
		ispass = this->IsPassPtCuts();
	}
	else if( codename == "DeltaRMuMuCut" )
	{
		if( varAux == 0 )
		{
			std::cerr << "ElecSelection::IsPass ERROR: "
				<< "Don't pass as second argument a vector<double> "
				<< "which contains the DeltaR between the muons. Exiting!!"
				<< std::endl;
			exit(-1);
		}
		// We need the arguments index of the vector, pt and eta
		if( varAux->size() != 1 )
		{
			std::cerr << "ElecSelection::IsPass ERROR: "
				<< "Don't pass as second argument a vector<double> "
				<< "which contains the DeltaR between the muons. Exiting!!"
				<< std::endl;
			exit(-1);
		}

		ispass = this->IsPassDeltaRCut((*varAux)[0]);
	}
	else if( codename == "ZMassWindow" )
	{
		// We need the invariant mass of the muon system as second argument
		if( varAux == 0 )
		{
			std::cerr << "ElecSelection::IsPass ERROR: "
				<< "Don't pass as second argument a vector<double> "
				<< "which contains the invariant mass of the muon system."
			        << " Exiting!!"
				<< std::endl;
			exit(-1);
		}
		if( varAux->size() != 1 )
		{
			std::cerr << "ElecSelection::IsPass ERROR: "
				<< "Don't pass as second argument a vector<double> "
				<< "which contains the invariant mass of the muon system."
			        << " Exiting!!"
				<< std::endl;
			exit(-1);
		}

		ispass = (! this->IsInsideZWindow((*varAux)[0]));
	}
	else if( codename == "MinMET" )
	{
		if( varAux == 0 )
		{
			std::cerr << "ElecSelection::IsPass ERROR: "
				<< "Don't pass as second argument a vector<double> "
				<< "which contains the MET. Exiting!!"
				<< std::endl;
			exit(-1);
		}
		// We need the arguments MET of the event
		if( varAux->size() != 1 )
		{
			std::cerr << "ElecSelection::IsPass ERROR: "
				<< "Don't pass as second argument a vector<double> "
				<< "which contains the MET. Exiting!!"
				<< std::endl;
			exit(-1);
		}

		ispass = this->IsPassMETCut((*varAux)[0]);
	}
	else
	{
			std::cerr << "ElecSelection::IsPass NOT IMPLEMENTED ERROR\n"
				<< "The codename '" << codename << "' is "
				<< "not implemented as a cut, you should update this"
				<< " function. Exiting!!"
				<< std::endl;
			exit(-1);
	}

	return ispass;
}


bool ElecSelection::IsPassMETCut(const double & MET) const
{	
	return ( MET > kMinMET );
}


// Specific muon pt-cuts (for the good identified-isolated muons)
bool ElecSelection::IsPassPtCuts() const
{
	std::vector<double> vptcut;
	vptcut.push_back(kMinMuPt1);
	vptcut.push_back(kMinMuPt2);
	vptcut.push_back(kMinMuPt3);
	int k = 2;
	// Ordered from higher to lower pt: begin from the lowest in order
	// to accomplish the old cut pt1 = 20 pt2 = 10 when you are dealing
	// with two leptons
        for(std::vector<int>::reverse_iterator it = _selectedGoodIdLeptons->rbegin(); 
			it != _selectedGoodIdLeptons->rend() ; ++it)
	{
		const unsigned int i = *it;
		const double ptcut = vptcut[k];
		if( _data->GetElecPt()->at(i) < ptcut )
		{
			return false;
		}
		k--;
	}
	// allright, all muons passed their cuts
	return true;
}


bool ElecSelection::IsPassDeltaRCut( const double & minDeltaRMuMu ) const
{
	return ( minDeltaRMuMu <= kMaxDeltaRMuMu );
}

// Return true if it is inside the Z window
bool ElecSelection::IsInsideZWindow( const double & invariantMass ) const
{
	return ( kMaxZMass > invariantMass && invariantMass > kMinZMass);
}

//FIXME: Asumo que estan ordenados por Pt!!! Commprobar
//---------------------------------------------
// Select muons
// - Return the size of the vector with the index of the muons 
//   passing our cuts (Kinematical cut -- pt and eta -- and must
//   be not standalone muon
// - Depends on fCutMinMuPt, fCutMaxMuEta 
//---------------------------------------------
unsigned int ElecSelection::SelectBasicLeptons() 
{
	//FIXME: It has to be always-- this function is called only 
	//      once -- to be checked
	if( _selectedbasicLeptons == 0 )
	{
		_selectedbasicLeptons = new std::vector<int>;
	}

	// Empty the selected muons vector --> Redundant to be removed
	_selectedbasicLeptons->clear();
	
	// Loop over electrons
	for(unsigned int i=0; i < _data->GetElecPx()->size(); ++i) 
	{
		//Build 4 vector for muon
		TLorentzVector Elec(_data->GetElecPx()->at(i), 
				_data->GetElecPy()->at(i), 
				_data->GetElecPz()->at(i), 
				_data->GetElecEnergy()->at(i));
		//Fill Histograms
		/*if(fFillHistos) 
		{
			fHElecSelectionPT->Fill(Mu.Pt());
			fHElecSelectionEta->Fill(Mu.Eta());
		}*/
		
		//[Cut in Eta and Pt]
		//-------------------
		//if( ! this->IsPassAcceptanceCuts(i,Mu.Pt(),Mu.Eta()) )
		if( fabs(Elec.Eta()) >= kMaxAbsEta || Elec.Pt() <= kMinMuPt3 )
		{
			continue;
		}
		//[If the muon is standalone, and it is neither Tracker 
		//nor Global then get rid of it]
		//-------------------
		Bool_t isEB=false;
		
		if( fabs(_data->GetElecSCEta()->at(i)) < 1.479 ) isEB=true;
  		bool isGoodElectronBool=   ((( isEB && Elec.Pt() < 20 && _data->GetElecsigmaIetaIeta()->at(i) < 0.01 &&           
                    _data->GetElecdeltaPhiIn()->at(i) > -0.03 && _data->GetElecdeltaPhiIn()->at(i) < 0.03 && 
                    _data->GetElecdeltaEtaIn()->at(i) > -0.004 && _data->GetElecdeltaEtaIn()->at(i) < 0.004 && 
                    _data->GetElecHtoE()->at(i) < 0.025 && (_data->GetElecfBrem()->at(i) > 0.15 || 
		      ( fabs(_data->GetElecSCEta()->at(i)) < 1. && _data->GetEleceSuperClusterOverP()->at(i) > 0.95 )) ) || 
                    ( (!isEB) && Elec.Pt() < 20 && _data->GetElecsigmaIetaIeta()->at(i) < 0.03 &&            
                    _data->GetElecdeltaPhiIn()->at(i) > -0.02 && _data->GetElecdeltaPhiIn()->at(i) < 0.02 && 
                    _data->GetElecdeltaEtaIn()->at(i) > -0.005 && _data->GetElecdeltaEtaIn()->at(i) < 0.005 && 
                    _data->GetElecHtoE()->at(i) < 0.1 && _data->GetElecfBrem()->at(i) > 0.15) || 
                    ( isEB && Elec.Pt() > 20 && _data->GetElecsigmaIetaIeta()->at(i) < 0.01 &&            
                    _data->GetElecdeltaPhiIn()->at(i) > -0.06 && _data->GetElecdeltaPhiIn()->at(i) < 0.06 && 
                    _data->GetElecdeltaEtaIn()->at(i) > -0.004 && _data->GetElecdeltaEtaIn()->at(i) < 0.004 && 
                    _data->GetElecHtoE()->at(i) < 0.04) || 
                    ( (!isEB) && Elec.Pt() > 20 && _data->GetElecsigmaIetaIeta()->at(i) < 0.03  &&   
                    _data->GetElecdeltaPhiIn()->at(i) > -0.03 && _data->GetElecdeltaPhiIn()->at(i) < 0.03 && 
                    _data->GetElecdeltaEtaIn()->at(i) > -0.007 && _data->GetElecdeltaEtaIn()->at(i) < 0.007 &&   
                    _data->GetElecHtoE()->at(i) < 0.1 ) ) );

		if( ! isGoodElectronBool )
		{
			continue;
		}
  
		/*if(  _data->IsAllStandAloneMuons()->at(i) 
				&& !_data->IsGlobalMuon()->at(i) 
				&& !_data->IsAllTrackerMuons()->at(i) )
		{
			continue; // muStaOnly 
		}*/
		
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
unsigned int ElecSelection::SelectLeptonsCloseToPV() 
{
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
	//unsigned int iGoodVertex = 0;

	//Loop over selected muons
	for(std::vector<int>::iterator it = _selectedbasicLeptons->begin();
			it != _selectedbasicLeptons->end(); ++it)
	{
		unsigned int i = *it;

		//Build 4 vector for muon (por que no utilizar directamente Pt
		double ptMu = TLorentzVector(_data->GetElecPx()->at(i), 
				_data->GetElecPy()->at(i), 
				_data->GetElecPz()->at(i), 
				_data->GetElecEnergy()->at(i)).Pt();

		//[Require muons to be close to PV] --> FIXME: MiniTRees, buscar forma de cambiarlo...
		//-------------------
//#ifdef MINITREES
//		// Next two lines for pure minitrees
//		double deltaZMu = _data->GetMuonvz()->at(i) - _data->GetVertexz()->at(iGoodVertex);
//		double IPMu     = _data->GetMuonIP2DInTrack()->at(i);
//#endif
//#ifdef LATINOTREES
		// Next two lines for latinos
		double deltaZMu = 0;
		double IPMu = 0;
		// + Lara
//		if (fUseBiased) {
		deltaZMu = _data->GetElecdzPVBiasedPV()->at(i);
		IPMu     = _data->GetElecIP2DBiasedPV()->at(i);
//		}
		// + Jonatan
//		else {
//			deltaZMu = _data->GetMuondzPVUnBiasedPV()->at(i);
//			IPMu     = _data->GetMuonIP2DUnBiasedPV()->at(i);
//		}
//#endif
		// Apply cut on PV depending on region
		// + R1: PT >= 20
		// + R2: PT <  20
		if(ptMu >= 20.0 && fabs(IPMu) > kMaxMuIP2DInTrackR1 ) 
		{
			continue;
		}
		else if(ptMu < 20.0  && fabs(IPMu) > kMaxMuIP2DInTrackR2 ) 
		{
			continue;
		}
		
		if(fabs(deltaZMu) > kMaxDeltaZMu )
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
unsigned int ElecSelection::SelectIsoLeptons() 
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
		TLorentzVector Elec(_data->GetElecPx()->at(i), 
				_data->GetElecPy()->at(i),
				_data->GetElecPz()->at(i), 
				_data->GetElecEnergy()->at(i));
		
		//[Require muons to be isolated]
		//-------------------
//#ifdef MINITREES
//		double isolation = (_data->GetMuonSumIsoTrack()->at(i) + 
//				_data->GetMuonSumIsoCalo()->at(i)) / Mu.Pt();
//#endif
//#ifdef LATINOTREES
		double isolation =(_data->GetEleceleSmurfPF()->at(i) )/Elec.Pt();
//#endif
		
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
		const double mupt = Elec.Pt();
		const double mueta= Elec.Eta();
		// High Pt Region:
		if( mupt > ptLimit )
		{
			// Low eta region: R1
			if( fabs(mueta) < etaLimit ) 
			{
				IsoCut = kMaxPTIsolationR1;
			}
			// High eta region: R2
			else  
			{
				IsoCut = kMaxPTIsolationR2;
			}
		}
		else  // Low Pt Region:
		{
			// Low eta region: R3
			if( fabs(mueta) < etaLimit )
			{
				IsoCut = kMaxPTIsolationR3;
			}
			// High eta region: R4
			else
			{
				IsoCut = kMaxPTIsolationR4;
			}
		}
		
		const bool isolatedMuon = (isolation < IsoCut);
		
		if( !isolatedMuon )
		{
			continue;
		}

		// Extra: Conversion
		if( ! (_data->GetElecpassesNewConversion()->at(i)  && 
				_data->GetElecnHits()->at(i) == 0) )
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
unsigned int ElecSelection::SelectGoodIdLeptons()
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
	
		// If we got here it means the muon is good
		_selectedGoodIdLeptons->push_back(i);
      	}
	
      	return _selectedGoodIdLeptons->size();
}

