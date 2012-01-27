
#include "MuonSelection.h"

#include "TLorentzVector.h"

#include<cmath>

const double kZMass = 91.1876; // TO BE INCLUDED IN THE CONFIG


//Constructor
MuonSelection::MuonSelection( TreeManager * data, const int & nLeptons) : 
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


void MuonSelection::LockCuts(const std::map<LeptonTypes,InputParameters*> & ipmap,
		const std::vector<std::string> & cuts)
{
	// Establishing the right 
	if( ipmap.find(MUON) == ipmap.end() )
	{
		std::cerr << "\033[1;31mElecSelection::LockCuts ERROR:\033[1;m "
			<< "The InputParameter introduced is not for electrons!"
			<< " Some incoherence found, check your initialization code."
			<< std::endl;
		exit(-1);
	}
	InputParameters * ip = (*(ipmap.find(MUON))).second;

	double dummy = 0;
	// Putting all the cuts (received from InitialiseCuts)
	for(std::vector<std::string>::const_iterator it = cuts.begin();
			it != cuts.end(); ++it)
	{
		ip->TheNamedDouble(it->c_str(), dummy);
		this->SetCut(it->c_str(),dummy);
		dummy = 0;
	}
	//   - Z mass window
	double deltazmass=0;
	ip->TheNamedDouble("DeltaZMass", deltazmass);
	this->SetCut("MaxZMass",kZMass+deltazmass);
	this->SetCut("MinZMass",kZMass-deltazmass);

	// Filling the datamembers: Note that before use some cut
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
			std::cerr << "MuonSelection::LockCuts ERROR:"
				<< " The cut named '" << cut->second << "' is not"
				<< " implemented. It has to be included in this method"
				<< " if you want to use it. Exiting..."
				<< std::endl;
			exit(-1);
		}*/
	}
}

// Helper function
std::vector<std::string> MuonSelection::GetCodenames() const
{
	return std::vector<std::string>(_codenames.begin(),_codenames.end());
}

bool MuonSelection::IsPass(const std::string & codename, const std::vector<double> * varAux ) const
{
	// Checking
	if( _codenames.count( codename ) != 1 )
	{
		std::cerr << "MuonSelection::IsPass ERROR: "
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
			std::cerr << "MuonSelection::IsPass ERROR: "
				<< "Waiting for a second argument a vector<double> "
				<< "which contains the DeltaR between the muons. Exiting!!"
				<< std::endl;
			exit(-1);
		}
		// We need the arguments index of the vector, pt and eta
		if( varAux->size() != 1 )
		{
			std::cerr << "MuonSelection::IsPass ERROR: "
				<< "Waiting for a second argument a vector<double> "
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
			std::cerr << "MuonSelection::IsPass ERROR: "
				<< "Waiting for a second argument a vector<double> "
				<< "which contains the invariant mass of the muon system."
			        << " Exiting!!"
				<< std::endl;
			exit(-1);
		}
		if( varAux->size() != 1 )
		{
			std::cerr << "MuonSelection::IsPass ERROR: "
				<< "Waiting for a second argument a vector<double> "
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
			std::cerr << "MuonSelection::IsPass ERROR: "
				<< "Waiting for a second argument a vector<double> "
				<< "which contains the MET. Exiting!!"
				<< std::endl;
			exit(-1);
		}
		// We need the arguments MET of the event
		if( varAux->size() != 1 )
		{
			std::cerr << "MuonSelection::IsPass ERROR: "
				<< "Waiting for a second argument a vector<double> "
				<< "which contains the MET. Exiting!!"
				<< std::endl;
			exit(-1);
		}

		ispass = this->IsPassMETCut((*varAux)[0]);
	}
	else
	{
			std::cerr << "MuonSelection::IsPass NOT IMPLEMENTED ERROR\n"
				<< "The codename '" << codename << "' is "
				<< "not implemented as a cut, you should update this"
				<< " function. Exiting!!"
				<< std::endl;
			exit(-1);
	}

	return ispass;
}


bool MuonSelection::IsPassMETCut(const double & MET) const
{	
	return ( MET > kMinMET );
}


// Specific muon pt-cuts (for the good identified-isolated muons)
bool MuonSelection::IsPassPtCuts() const
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
		if( _data->Get<float>("T_Muon_Pt",i) < ptcut )
		{
			return false;
		}
		k--;
	}
	// allright, all muons passed their cuts
	return true;
}


bool MuonSelection::IsPassDeltaRCut( const double & minDeltaRMuMu ) const
{
	return ( minDeltaRMuMu <= kMaxDeltaRMuMu );
}

// Return true if it is inside the Z window
bool MuonSelection::IsInsideZWindow( const double & invariantMass ) const
{
	return ( kMaxZMass > invariantMass && invariantMass > kMinZMass);
}

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
	for(unsigned int i=0; i < _data->GetSize<float>("T_Muon_Px"); ++i) 
	{
		//Build 4 vector for muon
		TLorentzVector Mu(_data->Get<float>("T_Muon_Px",i), 
				_data->Get<float>("T_Muon_Py",i), 
				_data->Get<float>("T_Muon_Pz",i), 
				_data->Get<float>("T_Muon_Energy",i));
		//Fill Histograms
		/*if(fFillHistos) 
		{
			fHMuonSelectionPT->Fill(Mu.Pt());
			fHMuonSelectionEta->Fill(Mu.Eta());
		}*/
		
		//[Cut in Eta and Pt]
		//-------------------
		//if( ! this->IsPassAcceptanceCuts(i,Mu.Pt(),Mu.Eta()) )
		if( fabs(Mu.Eta()) >= kMaxAbsEta || Mu.Pt() <= kMinMuPt3 )
		{
			continue;
		}
		//[If the muon is standalone, and it is neither Tracker 
		//nor Global then get rid of it]
		//-------------------
		if(  _data->Get<bool>("T_Muon_IsAllStandAloneMuons",i) 
				&& !_data->Get<bool>("T_Muon_IsGlobalMuon",i) 
				&& !_data->Get<bool>("T_Muon_IsAllTrackerMuons",i) )
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
		double ptMu = TLorentzVector(_data->Get<float>("T_Muon_Px",i), 
				_data->Get<float>("T_Muon_Py",i), 
				_data->Get<float>("T_Muon_Pz",i), 
				_data->Get<float>("T_Muon_Energy",i)).Pt();

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
		deltaZMu = _data->Get<float>("T_Muon_dzPVBiasedPV",i);
		IPMu     = _data->Get<float>("T_Muon_IP2DBiasedPV",i);
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
		TLorentzVector Mu(_data->Get<float>("T_Muon_Px",i), 
				_data->Get<float>("T_Muon_Py",i),
				_data->Get<float>("T_Muon_Pz",i), 
				_data->Get<float>("T_Muon_Energy",i));
		
		//[Require muons to be isolated]
		//-------------------
//#ifdef MINITREES
//		double isolation = (_data->GetMuonSumIsoTrack()->at(i) + 
//				_data->GetMuonSumIsoCalo()->at(i)) / Mu.Pt();
//#endif
//#ifdef LATINOTREES
		double isolation =(_data->Get<float>("T_Muon_muSmurfPF",i) )/Mu.Pt();
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
		const double mupt = Mu.Pt();
		const double mueta= Mu.Eta();
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
	
		double ptResolution = _data->Get<float>("T_Muon_deltaPt",i)/
			_data->Get<float>("T_Muon_Pt",i);
	        //Lepton ID and quality cuts
		bool passcutsforGlb = false;
		// If is global Muon using its ID cuts
		if( _data->Get<bool>("T_Muon_IsGlobalMuon",i) )
		{
			passcutsforGlb = _data->Get<int>("T_Muon_NValidHitsSATrk",i) > kMinNValidHitsSATrk
			    && _data->Get<float>("T_Muon_NormChi2GTrk",i) < kMaxNormChi2GTrk 
			    && _data->Get<int>("T_Muon_NumOfMatches",i) > kMinNumOfMatches;
		}
		
		bool passcutsforSA = false;
		if( _data->Get<bool>("T_Muon_IsAllTrackerMuons",i) ) // Tracker muons
		{
			passcutsforSA = _data->Get<bool>("T_Muon_IsTMLastStationTight",i);
		}
	
		const bool passSpecific = passcutsforGlb || passcutsforSA;
	
		// If is not global with quality or tracker with quality TMLast..
		// can continue
		if( ! passSpecific )
		{
			continue;
		}

		bool Idcuts = _data->Get<int>("T_Muon_NValidPixelHitsInTrk",i) > kMinNValidPixelHitsInTrk 
//#ifdef MINITREES
//		    && _data->GetMuonNValidHitsInTrk()->at(i) > kMinNValidHitsInTrk 
//#endif
//#ifdef LATINOTREES
 	            && _data->Get<int>("T_Muon_InnerTrackFound",i) > kMinNValidHitsInTrk 
//#endif
	           && fabs(ptResolution) < kMaxDeltaPtMuOverPtMu;

		//Fill Histograms
		//if (fFillHistos) 
		//{
		//	fHMuonSelectionDeltaPTOverPT->Fill(ptResolution);
		//}*/
		
		// Remember, if you are here, passSpecific=true
		if( ! Idcuts )
		{
			continue;
		}
		// If we got here it means the muon is good
		_selectedGoodIdLeptons->push_back(i);
      	}
	
      	return _selectedGoodIdLeptons->size();
}

//
// Select the fakeable objects. The _selectedbasicLeptons
// are substituted by this loose objects. At this level 
// a loose ISO and ID cuts are applied (@fakeablevar), so
// the sample produced are smaller than the basicLeptons
// This function is only activated when the CutManager was
// called in mode  CutManager::FAKEABLE
//
unsigned int MuonSelection::SelectLooseLeptons() 
{
	// First check is already was run over selected muons
	// if not do it
	if( _selectedbasicLeptons == 0)
	{
		this->SelectBasicLeptons();
	}

	std::vector<int> tokeep;

	//Loop over selected muons
	for(std::vector<int>::iterator it = _selectedbasicLeptons->begin();
			it != _selectedbasicLeptons->end(); ++it)
	{
		unsigned int i = *it;

		//Build 4 vector for muon (por que no utilizar directamente Pt
		double ptMu = TLorentzVector(_data->Get<float>("T_Muon_Px",i), 
				_data->Get<float>("T_Muon_Py",i), 
				_data->Get<float>("T_Muon_Pz",i), 
				_data->Get<float>("T_Muon_Energy",i)).Pt();

		//[ID d0 Cut] 
		double deltaZMu = 0;
		double IPMu = 0;
		deltaZMu = _data->Get<float>("T_Muon_dzPVBiasedPV",i);
		IPMu     = _data->Get<float>("T_Muon_IP2DBiasedPV",i);
		// Apply cut on d0
		if( fabs(IPMu) > 0.2 )//kLoosed0 ) FIXME HARDCODED
		{
			continue;
		}

		//[ISO cut]
		double isolation =(_data->Get<float>("T_Muon_muSmurfPF",i) )/ptMu;

		if( isolation > 0.4 ) //kLooseIso
		{
			continue;
		}
		// If we got here it means the muon is loose
		tokeep.push_back(i);
	}

	// rebuilding the selected leptons, now are loose too
	_selectedbasicLeptons->clear();
	for(unsigned int k = 0; k < tokeep.size(); ++k)
	{
		_selectedbasicLeptons->push_back( tokeep[k] );
	}

	return _selectedbasicLeptons->size();
}
