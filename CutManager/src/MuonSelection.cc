
#include "MuonSelection.h"

#include "TLorentzVector.h"

//#include<cmath>
#include<algorithm>

const double kZMass = 91.1876; // TO BE INCLUDED IN THE CONFIG

// Constructor
MuonSelection::MuonSelection( TreeManager * data, const int & nTights, const int & nLeptons,
		const char * runperiod) : 
	CutManager(data,nTights,nLeptons,runperiod),
	//_muonID(MuonID::VBTF),  // FIXME: Entered via argument?? or is good to be hardcoded?
	//_muonID(MuonID::HWWID),  // FIXME: Entered via argument?? or is good to be hardcoded?
	_muonID(-1),
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
	kMinNLayers(-1),
	kMaxDeltaPtMuOverPtMu(-1),
	kMaxLoosed0(-1),
	kMaxLooseIso(-1)
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
			<< "The InputParameter introduced is not for Muons!"
			<< " Some incoherence found, check your initialization code."
			<< std::endl;
		exit(-1);
	}
	InputParameters * ip = (*(ipmap.find(MUON))).second;

	// MuonId 
	std::string muonid( ip->TheNamedString("MuonID") );
	if( muonid == "HWWID" )
	{
		_muonID = MuonID::HWWID;
	}
	else if( muonid == "VBTF" )
	{
		_muonID = MuonID::VBTF;
	}
	else
	{
		std::cerr << "\033[1;31mElecSelection::LockCuts ERROR:\033[1;m "
			<< "The 'MuonID' parameter introduced is not recognized: '"
			<< _muonID << "'" << std::endl;
		exit(-1);
	}

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
		// Just to store
		this->SetCut(cut->first,cut->second);
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
			// Forcing to be null in the 2012
			if( std::string(ip->TheNamedString("RunPeriod")).find("2012") 
					!= std::string::npos )
			{
				kMinNValidHitsSATrk = -1;
			}
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
			// Forcing to be null in the 2012
			if( std::string(ip->TheNamedString("RunPeriod")).find("2012") 
					!= std::string::npos )
			{
				kMinNValidPixelHitsInTrk = -1;
			}
		}
		else if( cut->first == "MinNValidHitsInTrk" )
		{
			kMinNValidHitsInTrk = (int)cut->second;
			// Forcing to be null in the 2012
			if( std::string(ip->TheNamedString("RunPeriod")).find("2012") 
					!= std::string::npos )
			{
				kMinNValidHitsInTrk = -1;
			}
		}
		else if( cut->first == "MinNLayers" )
		{
			kMinNLayers = (int)cut->second;
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
		else if( cut->first == "MaxLoosed0" )
		{
			kMaxLoosed0 = cut->second;
		}
		else if( cut->first == "MaxLooseIso" )
		{
			kMaxLooseIso = cut->second;
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


// Proxy function to capture the client petitions and return the correct parameter related cut
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

		ispass = this->IsInsideZWindow((*varAux)[0]);
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
	//FIXME: Hardcoded number of lepton cuts: Potential bug!!
	std::vector<double> vptcut;
	vptcut.push_back(kMinMuPt1);
	vptcut.push_back(kMinMuPt2);
	vptcut.push_back(kMinMuPt3);
	int k = 2;
	// Ordered from higher to lower pt: begin from the lowest in order
	// to accomplish the old cut pt1 = 20 pt2 = 10 when you are dealing
	// with two leptons
        for(std::vector<LeptonRel>::reverse_iterator it = _selectedGoodIdLeptons->rbegin(); 
			it != _selectedGoodIdLeptons->rend() ; ++it)
	{
		const double ptcut = vptcut[k];
		if( it->getP4().Pt() < ptcut )
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

bool MuonSelection::PassVBTFTightID(const int & i) const
{
	bool Idcuts = (_data->Get<float>("T_Muon_NormChi2GTrk",i) < kMaxNormChi2GTrk )
		&& (_data->Get<int>("T_Muon_NValidHitsSATrk",i) > kMinNValidHitsSATrk )
		&& (_data->Get<int>("T_Muon_NumOfMatches",i) > kMinNumOfMatches )
		&& (_data->Get<int>("T_Muon_NValidPixelHitsInTrk",i) > kMinNValidPixelHitsInTrk )
		// Susbstituting NLayers cut
		&& (_data->Get<int>("T_Muon_NValidHitsInTrk",i) > kMinNValidHitsInTrk) ;

	return Idcuts;
}

bool MuonSelection::PassHWWMuonID(const int & i) const
{
	double ptResolution = _data->Get<float>("T_Muon_deltaPt",i)/_data->Get<float>("T_Muon_Pt",i);
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
		return false;
	}
	
	bool Idcuts = _data->Get<int>("T_Muon_NValidPixelHitsInTrk",i) > kMinNValidPixelHitsInTrk 
		// Note that T_Muon_InnerTrackFound = T_Muon_NValidHitsInTrk
		&& _data->Get<int>("T_Muon_InnerTrackFound",i) > kMinNValidHitsInTrk
		&& fabs(ptResolution) < kMaxDeltaPtMuOverPtMu;
	
	// New cuts for 2012
	if( _runperiod.find("2012") != std::string::npos )
	{
		Idcuts = Idcuts && _data->Get<bool>("T_Muon_isPFMuon",i) && 
		  _data->Get<int>("T_Muon_NLayers",i) > kMinNLayers;
	}

	return Idcuts;
}

// Get the muon isolation over pt depending of the ID applied. Also returns if
// the comparation w.r.t. input sheet values should be done inverse (2012) or not (2011)
std::pair<double,bool> MuonSelection::GetMuonIsolationOverPt( const int & i ) const
{
	if( this->_runperiod.find("2012") != std::string::npos )
	{
		return std::pair<double,int>(_data->Get<float>("T_Muon_MVARings",i),true);
	}

	double isovariableoverpt = 1.0/_data->Get<float>("T_Muon_Pt",i);
	if( this->_muonID == MuonID::HWWID )
	{
		isovariableoverpt *= _data->Get<float>("T_Muon_muSmurfPF",i);
	}
	else if( this->_muonID == MuonID::VBTF )
	{
		isovariableoverpt *= ( _data->Get<float>("T_Muon_pfCharged",i) 
				+ std::max(0.0,_data->Get<float>("T_Muon_pfNeutral",i)
				+ _data->Get<float>("T_Muon_pfPhoton",i) 
				- 0.5*_data->Get<float>("T_Muon_Beta",i)) );
	}
	else
	{
		std::cerr << "\033[1;31mMuonSelection::GetMuonIsolationOverPt ERROR:\033[1;m "
			<< "MuonID not recognized '" << this->_muonID << "'. Unexpected error, check the"
			<< " MuonSelection code for a call to this function. Exiting..."
			<< std::endl;
		exit(-1);
	}
	
	return std::pair<double,bool>(isovariableoverpt,false);
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
		// A way to force this function to be called
		// via the GetN methods
		//_selectedbasicLeptons = new std::vector<int>;
		this->GetNBasicLeptons();
	}

	// Empty the selected muons vector --> Redundant to be removed
	_selectedbasicLeptons->clear();
	
	// Be ready the notightLeptons if proceed
	if( _samplemode == CutManager::FAKEABLESAMPLE )
	{
		_notightLeptons = new std::vector<LeptonRel>;
		_registeredcols->push_back(&_notightLeptons);
	}

	
	// Loop over muons
	for(unsigned int i=0; i < _data->GetSize<float>("T_Muon_Px"); ++i) 
	{
		//Build 4 vector for muon
		TLorentzVector MuP4(_data->Get<float>("T_Muon_Px",i), 
				_data->Get<float>("T_Muon_Py",i), 
				_data->Get<float>("T_Muon_Pz",i), 
				_data->Get<float>("T_Muon_Energy",i));

		// Build the lepton with all 
		LeptonRel mu(MuP4,i);
		// these are muons
		mu.setleptontype(MUON);
		
		//[Cut in Eta and Pt]
		//-------------------
		//if( ! this->IsPassAcceptanceCuts(i,Mu.Pt(),Mu.Eta()) )
		if( fabs(mu.getP4().Eta()) >= kMaxAbsEta || mu.getP4().Pt() <= kMinMuPt3 )
		{
			continue;
		}
		// Just to avoid extra computation
		bool isnotglobal = false;
		if( _muonID == MuonID::HWWID )
		{	
			//[If the muon is standalone, and it is neither Tracker 
			//nor Global then get rid of it]
			//-------------------
			isnotglobal = ( _data->Get<bool>("T_Muon_IsAllStandAloneMuons",i) 
				&& !_data->Get<bool>("T_Muon_IsGlobalMuon",i) 
				&& !_data->Get<bool>("T_Muon_IsAllTrackerMuons",i) ); 
		}
		else if( _muonID == MuonID::VBTF )
		{
			isnotglobal = !_data->Get<bool>("T_Muon_IsGlobalMuon",i);
		}
		else
		{
			std::cerr << "\033[1;31mMuonSelection::GetMuonIsolationOverPt ERROR:\033[1;m "
				<< "MuonID not recognized '" << this->_muonID << "'. Unexpected error, check the"
				<< " MuonSelection code for a call to this function. Exiting..."
				<< std::endl;
			exit(-1);
		}
		
		if( isnotglobal )
		{
			continue; 
		}
		
		// If we got here it means the muon is good (keep track of hte new
		_selectedbasicLeptons->push_back(mu);
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
	if( _closeToPVLeptons == 0 )
	{
		this->GetNLeptonsCloseToPV();
		//_closeToPVLeptons = new std::vector<int>;
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
	for(std::vector<LeptonRel>::iterator it = _selectedbasicLeptons->begin();
			it != _selectedbasicLeptons->end(); ++it)
	{
		unsigned int i = it->index();

		//Build 4 vector for muon (por que no utilizar directamente Pt
		const double ptMu = it->getP4().Pt();

		//[Require muons to be close to PV] 
		//-------------------
		// FIXME: this function has to be merged with goodIDleptons 
		//        because how close is from a PV is part of the ID...
		//        So CloseToPV ---> become GoodID 
		double deltaZMu   = 0;
		// Note that T_Muon_IP2DBiasedPV = abs(T_Muon_dxyPVBiasedPV)
		const double IPMu = _data->Get<float>("T_Muon_IP2DBiasedPV",i);
		if( _muonID == MuonID::HWWID )
		{
			deltaZMu = _data->Get<float>("T_Muon_dzPVBiasedPV",i);
		}

		if(fabs(deltaZMu) > kMaxDeltaZMu )
		{
			continue;
		}		
		// Apply cut on PV depending on region
		// + R1: PT >= 20
		// + R2: PT <  20
		if(ptMu >= 20.0 && fabs(IPMu) > kMaxMuIP2DInTrackR1 ) 
		{
			if( _samplemode == CutManager::FAKEABLESAMPLE )
			{
				_notightLeptons->push_back(*it);
			}
			continue;
		}
		else if(ptMu < 20.0  && fabs(IPMu) > kMaxMuIP2DInTrackR2 ) 
		{
			if( _samplemode == CutManager::FAKEABLESAMPLE )
			{
				_notightLeptons->push_back(*it);
			}
			continue;
		}
		
		// If we got here it means the muon is good
		_closeToPVLeptons->push_back(*it);
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
		this->GetNIsoLeptons();
		//_selectedIsoLeptons = new std::vector<int>;
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
	for(std::vector<LeptonRel>::iterator it = _closeToPVLeptons->begin();
			it != _closeToPVLeptons->end(); ++it)
	{
		unsigned int i = it->index();
		
		//[Require muons to be isolated]
		//-------------------
		const std::pair<double,bool> isoandorder = this->GetMuonIsolationOverPt(i);
		const double isolation = isoandorder.first;
		const bool reversecmp = isoandorder.second;

		//WARNING: HARDCODED limit of the eta regions and Pt
		//The eta/pt plane is divided in 4 regions and the cut on isolation
		//is different in each region
		//
		// PT ^
		//   /|\ |
		//    |  |
		//    |R3|R4
		// 20-+--+---
		//    |R1|R2
		//    +--+---> eta
		//       |
		//      1.479 
		const double etaLimit = 1.479;
		const double ptLimit  = 20.0;

		double IsoCut = -1;
		const double mupt = it->getP4().Pt();
		const double mueta= it->getP4().Eta();
		// Low Pt Region:
		if( mupt <= ptLimit )
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
		else  // High Pt Region:
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
		
		// Note that when deal with the 2012 iso variable the cut is 
		// reversed, the condition to be satisfied is mva > IsoCut
		bool isolatedMuon = (isolation < IsoCut);
		if( reversecmp )
		{
			isolatedMuon = !isolatedMuon;
		}
		
		if( !isolatedMuon )
		{
			if( _samplemode == CutManager::FAKEABLESAMPLE )
			{
				_notightLeptons->push_back(*it);
			}
			continue;
		}
		// If we got here it means the muon is good
		_selectedIsoLeptons->push_back(*it);
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
		this->GetNGoodIdLeptons();
		//_selectedGoodIdLeptons = new std::vector<int>;
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
	for(std::vector<LeptonRel>::iterator it = _selectedIsoLeptons->begin();
			it != _selectedIsoLeptons->end(); ++it)
	{
		const unsigned int i = it->index();
	
		bool Idcuts = false;
		if( _muonID == MuonID::HWWID )
		{
			Idcuts = this->PassHWWMuonID(i);
		}
		else if( _muonID == MuonID::VBTF )
		{
			Idcuts = this->PassVBTFTightID(i);
		}
		else
		{
			std::cerr << "\033[1;31mMuonSelection::GetMuonIsolationOverPt ERROR:\033[1;m "
				<< "MuonID not recognized '" << _muonID << "'. Unexpected error, check the"
				<< " MuonSelection code for a call to this function. Exiting..."
				<< std::endl;
			exit(-1);
		}

		
		// Remember, if you are here, passSpecific=true
		if( ! Idcuts )
		{
			continue;
		}
		// If we got here it means the muon is good
		// and also we can include the rest of datamembers
		it->setcharge(_data->Get<int>("T_Muon_Charge",it->index()));
		_selectedGoodIdLeptons->push_back(*it);
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

	std::vector<LeptonRel> tokeep;

	//Loop over selected muons
	for(std::vector<LeptonRel>::iterator it = _selectedbasicLeptons->begin();
			it != _selectedbasicLeptons->end(); ++it)
	{
		unsigned int i = it->index();

		//[ID d0 Cut] 
		double IPMu = 0;
		IPMu     = _data->Get<float>("T_Muon_IP2DBiasedPV",i);
		// Apply cut on d0
		if( fabs(IPMu) > kMaxLoosed0 )
		{
			continue;
		}

		//[ISO cut]
		const std::pair<double,bool> isoandorder = this->GetMuonIsolationOverPt(i);
		const double isolation = isoandorder.first;
		const bool reversecmp = isoandorder.second;

		bool isIsolated = (isolation < kMaxLooseIso);
		
		// If 2012 data, reverse the previous cut in order to accomplish
		// correct condition MVA_output > kMaxLooseIso
		if( reversecmp )
		{
			isIsolated = !isIsolated;
		}

		if( ! isIsolated )
		{
			continue;
		}
		// If we got here it means the muon is loose
		tokeep.push_back(*it);
	}

	// rebuilding the selected leptons, now are loose too
	_selectedbasicLeptons->clear();
	for(unsigned int k = 0; k < tokeep.size(); ++k)
	{
		_selectedbasicLeptons->push_back( tokeep[k] );
	}

	return _selectedbasicLeptons->size();
}
