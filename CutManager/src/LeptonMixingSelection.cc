
#include "LeptonMixingSelection.h"
#include "MuonSelection.h"
#include "ElecSelection.h"

#include "TLorentzVector.h"

#include<cmath>
#include<map>
#include<algorithm>


//Constructor
LeptonMixingSelection::LeptonMixingSelection( TreeManager * data, const int & nLeptons) : 
	CutManager(data,nLeptons),
	fMuonSelection(0),
	fElecSelection(0)
{ 
	fMuonSelection = new MuonSelection(data,nLeptons);
	fElecSelection = new ElecSelection(data,nLeptons);
}

LeptonMixingSelection::~LeptonMixingSelection()
{
	if( fMuonSelection != 0 )
	{
		delete fMuonSelection;
		fMuonSelection = 0;
	}
	
	if( fElecSelection != 0 )
	{
		delete fElecSelection;
		fElecSelection = 0;
	}
	
	if( _leptontypebasicLeptons != 0)
	{
		delete _leptontypebasicLeptons;
		_leptontypebasicLeptons = 0;
	}
	if( _leptontypecloseToPVLeptons != 0)
	{
		delete _leptontypecloseToPVLeptons;
		_leptontypecloseToPVLeptons = 0;
	}
	if( _leptontypeIsoLeptons != 0)
	{
		delete _leptontypeIsoLeptons;
		_leptontypeIsoLeptons = 0;
	}
	if( _leptontypeGoodIdLeptons != 0)
	{
		delete _leptontypeGoodIdLeptons;
		_leptontypeGoodIdLeptons = 0;
	}
}


void LeptonMixingSelection::LockCuts(const std::map<LeptonTypes,InputParameters*> & ipmap,
		const std::vector<std::string> & cuts)
{
	fMuonSelection->LockCuts(ipmap,cuts);
	fElecSelection->LockCuts(ipmap,cuts);
}

//---  Helper functions

// There is no difference between lepton in the codenames so taking
// whatever
std::vector<std::string> LeptonMixingSelection::GetCodenames() const
{
	return fMuonSelection->GetCodenames();
}
// 
/*std::vector<std::string> LeptonMixingSelection::GetCodenames(const LeptonTypes & leptontype) const
{
	if( leptontype == MUON )
	{
		return fMuonSelection->GetCodenames();
	}
	else if( leptontype == ELECTRON )
	{
		return fElecSelection->GetCodenames();
	}
	else
	{
		return;
	}
}*/

// Wrapper function to evaluate cuts called directly from the client (Analysis class)
bool LeptonMixingSelection::IsPass(const std::string & codename, const std::vector<double> * varAux ) const
{
	// Checking
	if( _codenames.count( codename ) != 1 )
	{
		std::cerr << "\033[1;31mLeptonMixingSelection::IsPass ERROR\033[1;m: "
			<< "There is no selection chain called '" << codename
			<< "' implemented. The available selection names are:"
			<< std::endl;
		// No distintion between the cuts accepted by muons that by electrons
		// use whatever you want to extract them
		std::vector<std::string> v = this->fMuonSelection->GetCodenames();
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
			std::cerr << "LeptonMixingSelection::IsPass ERROR: "
				<< "Don't pass as second argument a vector<double> "
				<< "which contains the DeltaR between the muons. Exiting!!"
				<< std::endl;
			exit(-1);
		}
		// We need the arguments index of the vector, pt and eta
		if( varAux->size() != 1 )
		{
			std::cerr << "LeptonMixingSelection::IsPass ERROR: "
				<< "Don't pass as second argument a vector<double> "
				<< "which contains the DeltaR between the muons. Exiting!!"
				<< std::endl;
			exit(-1);
		}
		// Not depend of the lepton, using whatever you want
		ispass = this->fMuonSelection->IsPassDeltaRCut((*varAux)[0]);
	}
	else if( codename == "ZMassWindow" )
	{
		// We need the invariant mass of the muon system as second argument
		if( varAux == 0 )
		{
			std::cerr << "LeptonMixingSelection::IsPass ERROR: "
				<< "Don't pass as second argument a vector<double> "
				<< "which contains the invariant mass of the muon system."
			        << " Exiting!!"
				<< std::endl;
			exit(-1);
		}
		if( varAux->size() != 1 )
		{
			std::cerr << "LeptonMixingSelection::IsPass ERROR: "
				<< "Don't pass as second argument a vector<double> "
				<< "which contains the invariant mass of the muon system."
			        << " Exiting!!"
				<< std::endl;
			exit(-1);
		}
		// Not depend of the lepton, using whatever you want
		ispass = (! this->fMuonSelection->IsInsideZWindow((*varAux)[0]));
	}
	else if( codename == "MinMET" )
	{
		if( varAux == 0 )
		{
			std::cerr << "LeptonMixingSelection::IsPass ERROR: "
				<< "Don't pass as second argument a vector<double> "
				<< "which contains the MET. Exiting!!"
				<< std::endl;
			exit(-1);
		}
		// We need the arguments MET of the event
		if( varAux->size() != 1 )
		{
			std::cerr << "LeptonMixingSelection::IsPass ERROR: "
				<< "Don't pass as second argument a vector<double> "
				<< "which contains the MET. Exiting!!"
				<< std::endl;
			exit(-1);
		}
		// Not depend of the lepton, using whatever you want
		ispass = this->fMuonSelection->IsPassMETCut((*varAux)[0]);
	}
	else
	{
			std::cerr << "LeptonMixingSelection::IsPass NOT IMPLEMENTED ERROR\n"
				<< "The codename '" << codename << "' is "
				<< "not implemented as a cut, you should update this"
				<< " function. Exiting!!"
				<< std::endl;
			exit(-1);
	}

	return ispass;
}

// Note as the cuts are independent of the lepton used, we can
// indistinguishly used the Elec o Muon selection manager, ...
/* 
bool LeptonMixingSelection::IsPassMETCut(const double & MET) const
{	
	return ( MET > kMinMET );
}

bool MuonSelection::IsPassDeltaRCut( const double & minDeltaRMuMu ) const
{
	return ( minDeltaRMuMu <= kMaxDeltaRMuMu );
} 

// Return true if it is inside the Z window
bool MuonSelection::IsInsideZWindow( const double & invariantMass ) const
{
	return ( kMaxZMass > invariantMass && invariantMass > kMinZMass);
}*/

// Specific muon pt-cuts (for the good identified-isolated muons)
bool LeptonMixingSelection::IsPassPtCuts() const
{
	// Extracting the cuts from the correspondent lepton
	std::vector<double> vptcutMuon;
	vptcutMuon.push_back(fMuonSelection->kMinMuPt1);
	vptcutMuon.push_back(fMuonSelection->kMinMuPt2);
	vptcutMuon.push_back(fMuonSelection->kMinMuPt3);

	std::vector<double> vptcutElec;
	vptcutElec.push_back(fElecSelection->kMinMuPt1);
	vptcutElec.push_back(fElecSelection->kMinMuPt2);
	vptcutElec.push_back(fElecSelection->kMinMuPt3);
	
	// Found the real pt used giving the lepton type
	std::vector<double> vptcut;
	std::vector<std::string> leptonstr;
	for(unsigned int i = 0; i < _leptontypeGoodIdLeptons->size(); ++i)
	{
		if( (*_leptontypeGoodIdLeptons)[i] == MUON )
		{
			vptcut.push_back(vptcutMuon[i]);
			leptonstr.push_back("T_Muon_Pt");
		}
		else if( (*_leptontypeGoodIdLeptons)[i] == ELECTRON )
		{
			vptcut.push_back(vptcutElec[i]);
			leptonstr.push_back("T_Elec_Pt");
		}
	}

	int k = 2;
	// Ordered from higher to lower pt: begin from the lowest in order
	// to accomplish the old cut pt1 = 20 pt2 = 10 when you are dealing
	// with two leptons
        for(std::vector<int>::reverse_iterator it = _selectedGoodIdLeptons->rbegin(); 
			it != _selectedGoodIdLeptons->rend() ; ++it)
	{
		const unsigned int i = *it;
		const double ptcut = vptcut[k];
		if( _data->Get<float>(leptonstr[i].c_str(),i) < ptcut )
		{
			return false;
		}
		k--;
	}
	// allright, all leptons passed their cuts
	return true;
}


// Helper function to find if an index is inside the vector of indices
bool LeptonMixingSelection::isfoundindex(const std::vector<int> * const leptonsvector, 
		const int & index) const
{
	if( find(leptonsvector->begin(),leptonsvector->end(), index ) != leptonsvector->end() )
	{
		return true;
	}
		
	return false;
}


//FIXME: Asumo que estan ordenados por Pt!!! Commprobar
//---------------------------------------------
// Select muons and electrons
// - Return the size of the vector with the index of the muons 
//   passing our cuts (Kinematical cut -- pt and eta -- and must
//   be not standalone muon
// - Depends on fCutMinMuPt, fCutMaxMuEta 
//---------------------------------------------
unsigned int LeptonMixingSelection::SelectBasicLeptons() 
{
	//FIXME: It has to be always-- this function is called only 
	//      once -- to be checked
	if( _selectedbasicLeptons == 0 )
	{
		_selectedbasicLeptons = new std::vector<int>;
	}

	// Extract muons
	const int nselectedMuons = fMuonSelection->SelectBasicLeptons();
	const int nselectedElecs = fElecSelection->SelectBasicLeptons();

	// ordering by Pt
	// -- Muons
	std::map<double,std::pair<LeptonTypes,int> > ordermap;
	for(int i = 0 ; i < nselectedMuons; ++i)
	{
		const int index = fMuonSelection->_selectedbasicLeptons->at(i);
		ordermap[_data->Get<float>("T_Muon_Pt",index)] = std::pair<LeptonTypes,int>(MUON,index);
	}
	// -- Electrons
	for(int i = 0 ; i < nselectedElecs; ++i)
	{
		const int index = fElecSelection->_selectedbasicLeptons->at(i);
		ordermap[_data->Get<float>("T_Elec_Pt",index)] = std::pair<LeptonTypes,int>(ELECTRON,index);
	}
	// Consistency check (to avoid the repetition of doubles, very improbable!!)
	if( ordermap.size() != (unsigned int)(nselectedMuons+nselectedElecs) )
	{
		std::cerr << "\033[1;33mLeptonMixingSelection::SelectBasicLeptons WARNING\033[1;m"
			<< " The ordering pt algorithm has lost some of the leptons!"
			<< " Maybe it is necessary to check the algorithm and change it"
			<< "\nbecause of its leak of robustness!." 
			<< std::endl;
	}
			
	
	// Storing the  index (in _selectedbasicLeptons inherit datamember) and
	// the reference to the kind of lepton (_leptontypebasicLepton)
	for(std::map<double,std::pair<LeptonTypes,int> >::reverse_iterator it = ordermap.rbegin();
			it != ordermap.rend(); ++it)
	{
		// it->second is std::pair<LeptonType,int>
		_leptontypebasicLeptons->push_back( it->second.first );
		_selectedbasicLeptons->push_back( it->second.second );
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
unsigned int LeptonMixingSelection::SelectLeptonsCloseToPV() 
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

	// Muons
	/*const int nmuons =*/ fMuonSelection->SelectLeptonsCloseToPV();
	// Electrons
	/*const int nelecs =*/ fElecSelection->SelectLeptonsCloseToPV();

	//Loop over selected leptons
	for(std::vector<int>::iterator it = _selectedbasicLeptons->begin();
			it != _selectedbasicLeptons->end(); ++it)
	{
		unsigned int i = *it;
		LeptonTypes lepton = (*_leptontypebasicLeptons)[i];
		if( lepton == MUON )
		{
			if( ! isfoundindex(fMuonSelection->_closeToPVLeptons,i) )
			{
				continue;
			}
		}
		else if( lepton == ELECTRON )
		{
			if( ! isfoundindex(fElecSelection->_closeToPVLeptons,i) )
			{
				continue;
			}
		}

		_leptontypecloseToPVLeptons->push_back(lepton);
		_closeToPVLeptons->push_back(i);
	}
	
	return _closeToPVLeptons->size();
}

//---------------------------------------------
// Select isolated muons
// - Returns the number of selected isolated muons
// - Depends on MaxIsoMu cut
//---------------------------------------------
unsigned int LeptonMixingSelection::SelectIsoLeptons() 
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
	// Muons
	/*const int nmuons =*/ fMuonSelection->SelectIsoLeptons();
	// Electrons
	/*const int nelecs =*/ fElecSelection->SelectIsoLeptons();

	//Loop over selected muons
	for(std::vector<int>::iterator it = _closeToPVLeptons->begin();
			it != _closeToPVLeptons->end(); ++it)
	{
		unsigned int i = *it;
		LeptonTypes lepton = (*_leptontypecloseToPVLeptons)[i];
		if( lepton == MUON )
		{
			if( ! isfoundindex(fMuonSelection->_selectedIsoLeptons, i) )
			{
				continue;
			}
		}
		else if( lepton == ELECTRON )
		{
			if( ! isfoundindex(fElecSelection->_selectedIsoLeptons, i) )
			{
				continue;
			}
		}

		_leptontypeIsoLeptons->push_back(lepton);
		_selectedIsoLeptons->push_back(i);
	}
	
	return _selectedIsoLeptons->size();
}

//---------------------------------------------
// Select isolated good muons
// - Returns the number of selected isolated good muons
// - No dependencies: FIX CUTS!!!
//---------------------------------------------
unsigned int LeptonMixingSelection::SelectGoodIdLeptons()
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
	
	// Muons
	/*const int nmuons =*/ fMuonSelection->SelectGoodIdLeptons();
	// Electrons
	/*const int nelecs =*/ fElecSelection->SelectGoodIdLeptons();

	//Loop over selected muons
	for(std::vector<int>::iterator it = _selectedIsoLeptons->begin();
			it != _selectedIsoLeptons->end(); ++it)
	{
		unsigned int i = *it;
		LeptonTypes lepton = (*_leptontypeIsoLeptons)[i];
		if( lepton == MUON )
		{
			if( ! isfoundindex(fMuonSelection->_selectedGoodIdLeptons,i) )
			{
				continue;
			}
		}
		else if( lepton == ELECTRON )
		{
			if( ! isfoundindex(fElecSelection->_selectedGoodIdLeptons, i) )
			{
				continue;
			}
		}

		_leptontypeGoodIdLeptons->push_back(lepton);
		_selectedGoodIdLeptons->push_back(i);
	}
	
      	return _selectedGoodIdLeptons->size();
}



