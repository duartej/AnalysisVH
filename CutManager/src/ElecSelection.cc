
#include "ElecSelection.h"
#include "WPElecID.h"

#include "TLorentzVector.h"

#include<cmath>
#include<algorithm>
const double kZMass = 91.1876; // TO BE INCLUDED IN THE CONFIG


// CONSTRUCTOR
ElecSelection::ElecSelection( TreeManager * data, const int & WPlowpt,
		const int & WPhighpt, const int & nTights, const int & nLeptons,
		const char * runperiod) : 
	CutManager(data,nTights,nLeptons,runperiod),
	pWP_lowPt(0),   
	pWP_highPt(0),
	_ElecType(ElecSelection::CUTBASED),
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
	kMinMVAValueR1(-1),
	kMinMVAValueR2(-1),
	kMinMVAValueR3(-1),
	kMinMVAValueR4(-1),
	kMinMVAValueR5(-1),
	kMinMVAValueR6(-1),
	kMaxSigmaietaietaR1(-1),
	kMaxdeltaPhiInR1(-1), 
	kMaxdeltaEtaInR1(-1),
	kMaxHtoER1(-1),       
	kMaxSigmaietaietaR2(-1),
	kMaxdeltaPhiInR2(-1), 
	kMaxdeltaEtaInR2(-1),
	kMaxHtoER2(-1),  
	kMaxdr03TkSumPtOverPt(-1),
	kMaxdr03EcalSumPtOverPt(-1),
	kMaxdr03HcalSumPtOverPt(-1)
{ 
	// Initialize the selection codenames
	_codenames.insert("PtMuonsCuts");
	_codenames.insert("DeltaRMuMuCut");
	_codenames.insert("ZMassWindow");
	_codenames.insert("MinMET");

	// Set the Id related cuts datamembers
	// WARNING HARDCODED (if this ok?)
	pWP_lowPt = new WPElecID( WPlowpt );
	pWP_highPt = new WPElecID( WPhighpt );
}

ElecSelection::ElecSelection( TreeManager * data, const int & nTights, const int & nLeptons,
		const char * runperiod) : 
	CutManager(data,nTights,nLeptons,runperiod),
	pWP_lowPt(0),   
	pWP_highPt(0),
	_ElecType(ElecSelection::BDTBASED),
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
	kMinMVAValueR1(-1),
	kMinMVAValueR2(-1),
	kMinMVAValueR3(-1),
	kMinMVAValueR4(-1),
	kMinMVAValueR5(-1),
	kMinMVAValueR6(-1),
	kMaxSigmaietaietaR1(-1),
	kMaxdeltaPhiInR1(-1), 
	kMaxdeltaEtaInR1(-1),
	kMaxHtoER1(-1),       
	kMaxSigmaietaietaR2(-1),
	kMaxdeltaPhiInR2(-1), 
	kMaxdeltaEtaInR2(-1),
	kMaxHtoER2(-1),  
	kMaxdr03TkSumPtOverPt(-1),
	kMaxdr03EcalSumPtOverPt(-1),
	kMaxdr03HcalSumPtOverPt(-1)
{ 
	// Initialize the selection codenames
	_codenames.insert("PtMuonsCuts");
	_codenames.insert("DeltaRMuMuCut");
	_codenames.insert("ZMassWindow");
	_codenames.insert("MinMET");
}


ElecSelection::~ElecSelection()
{
	if( pWP_lowPt != 0 )
	{
		delete pWP_lowPt;
		pWP_lowPt = 0;
	}
	
	if( pWP_highPt != 0 )
	{
		delete pWP_highPt;
		pWP_highPt = 0;
	}
}


void ElecSelection::LockCuts(const std::map<LeptonTypes,InputParameters*> & ipmap,
		const std::vector<std::string> & cuts)
{
	// Establishing the right
	if( ipmap.find(ELECTRON) == ipmap.end() )
	{
		std::cerr << "\033[1;31mElecSelection::LockCuts ERROR:\033[1;m "
			<< "The InputParameter introduced is not for electrons!"
			<< " Some incoherence found, check your initialization code."
			<< std::endl;
		exit(-1);
	}
	InputParameters * ip = (*(ipmap.find(ELECTRON))).second;

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
	double kZMass = 0;
	ip->TheNamedDouble("DeltaZMass", deltazmass);
	this->SetCut("MaxZMass",kZMass+deltazmass);
	this->SetCut("MinZMass",kZMass-deltazmass);
	
	// Filling the datamembers: Note that before use some cut
	// it has to be checked if it was initialized
	for(std::map<std::string,double>::iterator cut = _cuts->begin(); 
			cut != _cuts->end();++cut)
	{
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
		else if( cut->first == "MaxSigmaietaietaR1" )
		{
			kMaxSigmaietaietaR1 = cut->second;
		}
		else if( cut->first == "MaxSigmaietaietaR2" )
		{
			kMaxSigmaietaietaR2 = cut->second;
		}
		else if( cut->first == "MaxdeltaPhiInR1" )
		{
			kMaxdeltaPhiInR1 = cut->second;
		}
		else if( cut->first == "MaxdeltaPhiInR2" )
		{
			kMaxdeltaPhiInR2 = cut->second;
		}
		else if( cut->first == "MaxdeltaEtaInR1" )
		{
			kMaxdeltaEtaInR1 = cut->second;
		}
		else if( cut->first == "MaxdeltaEtaInR2" )
		{
			kMaxdeltaEtaInR2 = cut->second;
		}
		else if( cut->first == "MaxHtoER1" )
		{
			kMaxHtoER1 = cut->second;
		}
		else if( cut->first == "MaxHtoER2" )
		{
			kMaxHtoER2 = cut->second;
		}
		else if( cut->first == "Maxdr03TkSumPtOverPt" )
		{
			kMaxdr03TkSumPtOverPt = cut->second;
		}
		else if( cut->first == "Maxdr03EcalSumPtOverPt" )
		{
			kMaxdr03EcalSumPtOverPt = cut->second;
		}
		else if( cut->first == "Maxdr03HcalSumPtOverPt" )
		{
			kMaxdr03HcalSumPtOverPt = cut->second;
		}
		else if( cut->first == "MaxZMass" )
		{
			kMaxZMass = cut->second;
		}
		else if( cut->first == "MinZMass" )
		{
			kMinZMass = cut->second;
		}
		else if( cut->first == "MinMVAValueR1" )
		{
			kMinMVAValueR1 = cut->second;
		}
		else if( cut->first == "MinMVAValueR2" )
		{
			kMinMVAValueR2 = cut->second;
		}
		else if( cut->first == "MinMVAValueR3" )
		{
			kMinMVAValueR3 = cut->second;
		}
		else if( cut->first == "MinMVAValueR4" )
		{
			kMinMVAValueR4 = cut->second;
		}
		else if( cut->first == "MinMVAValueR5" )
		{
			kMinMVAValueR5 = cut->second;
		}
		else if( cut->first == "MinMVAValueR6" )
		{
			kMinMVAValueR6 = cut->second;
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

//---  Helper functions

// 
std::vector<std::string> ElecSelection::GetCodenames() const
{
	return std::vector<std::string>(_codenames.begin(),_codenames.end());
}

// Wrapper function to evaluate cuts called directly from the client (Analysis class)
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
				<< "Waiting for a second argument a vector<double> "
				<< "which contains the DeltaR between the muons. Exiting!!"
				<< std::endl;
			exit(-1);
		}
		// We need the arguments index of the vector, pt and eta
		if( varAux->size() != 1 )
		{
			std::cerr << "ElecSelection::IsPass ERROR: "
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
			std::cerr << "ElecSelection::IsPass ERROR: "
				<< "Waiting for a second argument a vector<double> "
				<< "which contains the invariant mass of the muon system."
			        << " Exiting!!"
				<< std::endl;
			exit(-1);
		}
		if( varAux->size() != 1 )
		{
			std::cerr << "ElecSelection::IsPass ERROR: "
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
			std::cerr << "ElecSelection::IsPass ERROR: "
				<< "Waiting for a second argument a vector<double> "
				<< "which contains the MET. Exiting!!"
				<< std::endl;
			exit(-1);
		}
		// We need the arguments MET of the event
		if( varAux->size() != 1 )
		{
			std::cerr << "ElecSelection::IsPass ERROR: "
				<< "Waiting for a second argument a vector<double> "
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
        for(std::vector<LeptonRel*>::reverse_iterator it = _selectedGoodIdLeptons->rbegin(); 
			it != _selectedGoodIdLeptons->rend() ; ++it)
	{
		const double ptcut = vptcut[k];
		if( (*it)->getP4().Pt() < ptcut )
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

// BDT Electrons
bool ElecSelection::IsPassBDT( const unsigned int & index ) const
{
	//Variables:
	const double pt       = _data->Get<float>("T_Elec_Pt",index);
	const double absSCeta = fabs(_data->Get<float>("T_Elec_SC_Eta",index));
	
	const char * mvanamestr = "T_Elec_BDT";
	double regionloweta = 1.0;
	if( this->_runperiod.find("2012") != std::string::npos )
	{
		mvanamestr = "T_Elec_MVA";
		regionloweta = 0.8;
	}
	const double bdtValue = _data->Get<float>(mvanamestr,index);

	//The eta/pt plane is divided in 6 regions and the cut on mva values
	//is different in each region
	//
	// PT ^
	//    |
	//    |  |  |  |
	//    |R4|R5|R6|
	// 20-+--+--+--+
	//    |R1|R2|R3|
	//    +--+--+---> eta
	//       |  |
	//      r1  1.479 
	double mvacut = 999;
	// Low pt electrons
	if( pt < 20.0 )
	{
		if( absSCeta <= regionloweta ) 
		{
			mvacut = kMinMVAValueR1;
		}
		else if( absSCeta > regionloweta && absSCeta <= 1.479 )
		{
			mvacut = kMinMVAValueR2;
		}
		else if( absSCeta > 1.479 )
		{
			mvacut = kMinMVAValueR3;
		}
	}
	else
	{
		if( absSCeta <= regionloweta ) 
		{
			mvacut = kMinMVAValueR4;
		}
		else if( absSCeta > regionloweta && absSCeta <= 1.479 )
		{
			mvacut = kMinMVAValueR5;
		}
		else if( absSCeta > 1.479 )
		{
			mvacut = kMinMVAValueR6;
		}
	}
	
	return (bdtValue > mvacut);
}


// Working point
bool ElecSelection::IsPassWP( const unsigned int & index ) const
{
	//Variables:
	const double sigmaietaieta = _data->Get<float>("T_Elec_sigmaIetaIeta",index);
	const double deltaPhiIn    = _data->Get<float>("T_Elec_deltaPhiIn",index);
	const double deltaEtaIn    = _data->Get<float>("T_Elec_deltaEtaIn",index);
	const double HtoE          = _data->Get<float>("T_Elec_HtoE",index);

	const double pt   = _data->Get<float>("T_Elec_Pt",index);
	const double absSCeta= fabs(_data->Get<float>("T_Elec_SC_Eta",index));

	bool ispass = false;
	
	const double trkIso03 = _data->Get<float>("T_Elec_dr03TkSumPt",index);
	const double emIso03  = _data->Get<float>("T_Elec_dr03EcalSumEt",index);
	const double max_emIso03 = std::max(emIso03-1.0,0.0);
	const double hadIso03 = _data->Get<float>("T_Elec_dr03HcalSumEt",index);
	// BARREL Electrons: Region 1 (R1)
	if( absSCeta < 1.479 )
	{
		ispass = sigmaietaieta < kMaxSigmaietaietaR1 && fabs(deltaPhiIn) < kMaxdeltaPhiInR1 
			&& fabs(deltaEtaIn) < kMaxdeltaEtaInR1 && HtoE < kMaxHtoER1
			// Isolation
			&& trkIso03/pt < kMaxdr03TkSumPtOverPt && max_emIso03/pt < kMaxdr03EcalSumPtOverPt
			       && hadIso03/pt < kMaxdr03HcalSumPtOverPt;
	}
	else  // Endcap electrons: Region 2 (R2)
	{
		ispass = sigmaietaieta < kMaxSigmaietaietaR2 && fabs(deltaPhiIn) < kMaxdeltaPhiInR2 
			&& fabs(deltaEtaIn) < kMaxdeltaEtaInR2 && HtoE < kMaxHtoER2
			// Isolation
			&& trkIso03/pt < kMaxdr03TkSumPtOverPt && emIso03/pt < kMaxdr03EcalSumPtOverPt
			       && hadIso03/pt < kMaxdr03HcalSumPtOverPt;
	}
	
	// Conversion and number of expected hits
	const char * conversionstr = "T_Elec_passesNewConversion";
	if( _runperiod.find("2012") != std::string::npos )
	{
		conversionstr = "T_Elec_passConversionVeto";
	}
	const bool passconversion = _data->Get<bool>(conversionstr,index);
	const bool passExpectedHits = _data->Get<int>("T_Elec_nHits",index) <= 0; 
	
	return ispass && passconversion && passExpectedHits;
}

//---------------------------------------------
// Select electrons
// - Return the size of the vector with the index of the muons 
//   passing our cuts (Kinematical cut -- pt and eta -- and must
//   be not standalone muon
// - Depends on fCutMinMuPt, fCutMaxMuEta 
//---------------------------------------------
unsigned int ElecSelection::SelectBasicLeptons() 
{
	if( _selectedbasicLeptons == 0 )
	{
		this->GetNBasicLeptons();
	}

	// Empty the selected muons vector --> Redundant to be removed
	_selectedbasicLeptons->clear();
	
	// Be ready the notightLeptons if proceed
	if( _samplemode == CutManager::FAKEABLESAMPLE )
	{
		_notightLeptons = new std::vector<LeptonRel*>;
		_registeredcols->push_back(_notightLeptons);
	}
	
	// Loop over electrons
	for(unsigned int i=0; i < _data->GetSize<float>("T_Elec_Px"); ++i) 
	{
		//Build 4 vector for muon
		TLorentzVector ElecP4(_data->Get<float>("T_Elec_Px",i), 
				_data->Get<float>("T_Elec_Py",i), 
				_data->Get<float>("T_Elec_Pz",i), 
				_data->Get<float>("T_Elec_Energy",i));

		// Build the relative lepton 
		LeptonRel elec(ElecP4,i);
		elec.setleptontype(ELECTRON);
		
		
		//[Cut in Eta and Pt]
		//-------------------
		if( fabs(elec.getP4().Eta()) >= kMaxAbsEta || elec.getP4().Pt() <= kMinMuPt3 )
		{
			continue;
		}

		//[ Using only electrons with a minimum quality]
		if( ! this->IsPassWP(i) )
		{
			continue;
		}
		
		// If we got here it means the muon is good
		_selectedbasicLeptons->push_back(new LeptonRel(elec));
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
		this->GetNLeptonsCloseToPV();
	}

	//Empty the vector of indices --> Redundant
	_closeToPVLeptons->clear();

	// First check is already was run over selected muons
	// if not do it
	if( _selectedbasicLeptons == 0)
	{
		this->SelectBasicLeptons();
	}

	//Loop over selected muons
	for(std::vector<LeptonRel*>::iterator it = _selectedbasicLeptons->begin();
			it != _selectedbasicLeptons->end(); ++it)
	{
		unsigned int i = (*it)->index();

		//Build 4 vector for muon (por que no utilizar directamente Pt
		// FIXME: Not needed: just extract Pt
		const double ptMu = (*it)->getP4().Pt();

		//[Require muons to be close to PV] 
		//-------------------
		double deltaZMu = 0;
		double IPMu = 0;
		deltaZMu = _data->Get<float>("T_Elec_dzPVBiasedPV",i);
		IPMu     = _data->Get<float>("T_Elec_IP2DBiasedPV",i);
		if( fabs(deltaZMu) > kMaxDeltaZMu )
		{
			continue;
		}

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
unsigned int ElecSelection::SelectIsoLeptons() 
{
	//FIXME: It has to be always-- this function is called only 
	//      once -- to be checked
	if( _selectedIsoLeptons == 0)
	{
		this->GetNIsoLeptons();
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
	for(std::vector<LeptonRel*>::iterator it = _closeToPVLeptons->begin();
			it != _closeToPVLeptons->end(); ++it)
	{
		unsigned int i = (*it)->index();

		//[Require muons to be isolated]
		//-------------------
		const char * isonamestr = "T_Elec_eleSmurfPF";
		double elecpt = (*it)->getP4().Pt();
		if( _runperiod.find("2012") != std::string::npos )
		{
			isonamestr = "T_Elec_pfComb";
			elecpt = 1.0;
		}
		double isolation =(_data->Get<float>(isonamestr,i) )/elecpt;
		//The eta/pt plane is divided in 4 regions and the cut on isolation
		//is different in each region
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
		const double eleceta= (*it)->getP4().Eta();
		// Low Pt Region:
		if( elecpt <= ptLimit )
		{
			// Low eta region: R1
			if( fabs(eleceta) < etaLimit ) 
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
			if( fabs(eleceta) < etaLimit )
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
			if( _samplemode == CutManager::FAKEABLESAMPLE )
			{
				_notightLeptons->push_back(*it);
			}
			continue;
		}
		
		// Technically it is in this function when the no tights must be totally
		// filled altough conceptually is not here where we have to use the 
		// following cut because is not an isolated cut
		if( ! this->IsPassBDT(i) )
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
// FIXME: ESTA FUNCTION NO HACE NADA
unsigned int ElecSelection::SelectGoodIdLeptons()
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


	// Note we don't need to do anything more...
	*_selectedGoodIdLeptons = *_selectedIsoLeptons;
	// Just adding the missing charge
	for(std::vector<LeptonRel*>::iterator it = _selectedGoodIdLeptons->begin();
			it != _selectedGoodIdLeptons->end(); ++it)
	{
		(*it)->setcharge(_data->Get<int>("T_Elec_Charge",(*it)->index()));
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
unsigned int ElecSelection::SelectLooseLeptons() 
{
	// First check is already was run over selected muons
	// if not do it
	if( _selectedbasicLeptons == 0)
	{
		this->SelectBasicLeptons();
	}
	
	// Note that all the selected basic electrons are loose
	return _selectedbasicLeptons->size();
}

