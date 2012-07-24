
#include "LeptonMixingSelection.h"
#include "MuonSelection.h"
#include "ElecSelection.h"

#include "TLorentzVector.h"

#include<cmath>
#include<map>
#include<algorithm>

//Constructor cut-based electrons: 
LeptonMixingSelection::LeptonMixingSelection( TreeManager * data, const int & WPlowpt,
		const int & WPhighpt, const int & nTights, const int & nLeptons,
		const char  * runperiod) : 
	CutManager(data,nTights,nLeptons,runperiod),
	fMuonSelection(0),
	fElecSelection(0),
	_leptontypebasicLeptons(0),
	_leptontypecloseToPVLeptons(0),
	_leptontypeIsoLeptons(0),
	_leptontypeGoodIdLeptons(0),
	_tightLeptonTypes(0),
	_notightLeptonTypes(0)
{ 
	fMuonSelection = new MuonSelection(data,nTights,nLeptons,runperiod);
	fElecSelection = new ElecSelection(data,WPlowpt,WPhighpt,nTights,nLeptons,runperiod);
}

//Constructor BDT-based electrons: 
LeptonMixingSelection::LeptonMixingSelection( TreeManager * data, 
		const int & nTights, const int & nLeptons,
		const char * runperiod) : 
	CutManager(data,nTights,nLeptons,runperiod),
	fMuonSelection(0),
	fElecSelection(0),
	_leptontypebasicLeptons(0),
	_leptontypecloseToPVLeptons(0),
	_leptontypeIsoLeptons(0),
	_leptontypeGoodIdLeptons(0),
	_tightLeptonTypes(0),
	_notightLeptonTypes(0)
{ 
	fMuonSelection = new MuonSelection(data,nTights,nLeptons,runperiod);
	fElecSelection = new ElecSelection(data,nTights,nLeptons,runperiod);
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

	if( _tightLeptonTypes != 0)
	{
		delete _tightLeptonTypes;
		_tightLeptonTypes = 0;
	}

	if( _notightLeptonTypes != 0)
	{
		delete _notightLeptonTypes;
		_notightLeptonTypes = 0;
	}
}

void LeptonMixingSelection::Reset()
{
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

	if( _tightLeptonTypes != 0)
	{
		delete _tightLeptonTypes;
		_tightLeptonTypes = 0;
	}

	if( _notightLeptonTypes != 0)
	{
		delete _notightLeptonTypes;
		_notightLeptonTypes = 0;
	}
	fMuonSelection->Reset(); 
	fElecSelection->Reset();
	CutManager::Reset();
}

void LeptonMixingSelection::LockCuts(const std::map<LeptonTypes,InputParameters*> & ipmap,
		const std::vector<std::string> & cuts)
{
	fMuonSelection->LockCuts(ipmap,cuts);
	fElecSelection->LockCuts(ipmap,cuts);
	// Updating the dict (based in Muon and ...
	this->_cuts = fMuonSelection->_cuts;
	// .. what it is missing from electrons
	for(std::map<std::string,double>::iterator it = fElecSelection->_cuts->begin(); 
		it != fElecSelection->_cuts->end(); ++it)
	{
		if( this->_cuts->find(it->first) != this->_cuts->end() )
		{
			(*this->_cuts)[it->first] = it->second;
		}
	}
}

//---  Helper functions

// There is no difference between lepton in the codenames so taking
// whatever
std::vector<std::string> LeptonMixingSelection::GetCodenames() const
{
	return fMuonSelection->GetCodenames();
}

LeptonTypes LeptonMixingSelection::GetLeptonType(const unsigned int & index) const
{
	if( _leptontypeGoodIdLeptons == 0 )
	{
		std::cerr << "\033[1;31mLeptonMixingSelection::GetLeptonType ERROR\033[1;m"
			<< " This function can not be used before calling the"
			<< " LeptonMixingSelection::SelectGoodIdLeptons, call it first!"
			<< std::endl;
		exit(-1);
	}
	if( index >= _leptontypeGoodIdLeptons->size() )
	{
		std::cerr << "\033[1;31mLeptonMixingSelection::GetLeptonType ERROR\033[1;m"
			<< " The argument of this function must be the REAL VECTOR INDEX"
			<< " of the selectedGoodIdLeptons not the index of the original TBranch"
			<< " object. Correct that in the code and launch it again."
			<< std::endl;
		exit(-1);
	}

	// Recall index is the vector index
	return _leptontypeGoodIdLeptons->at(index);
}

// Tight leptons
LeptonTypes LeptonMixingSelection::GetTightLeptonType(const unsigned int & index) const
{
	if( _samplemode != CutManager::FAKEABLESAMPLE )
	{
		std::cerr << "\033[1;31mLeptonMixingSelection::GetTightLeptonType ERROR\033[1;m "
			<< " Incoherent use of"
			<< " this function because it cannot be called in NORMALSAMPLE mode."
			<< " Check the client of this function why has been made this call"
			<< std::endl;
		exit(-1);
	}

	if( _tightLeptonTypes == 0 )  // Not needed
	{
		std::cerr << "\033[1;31mLeptonMixingSelection::GetTightLeptonType ERROR\033[1;m"
			<< " This function can not be used before calling the"
			<< " LeptonMixingSelection::GetNGoodIdLeptons, call it first!"
			<< std::endl;
		exit(-1);
	}
	if( index >= _tightLeptonTypes->size() )
	{
		std::cerr << "\033[1;31mLeptonMixingSelection::GetTightLeptonType ERROR\033[1;m"
			<< " The argument of this function must be the REAL VECTOR INDEX"
			<< " of the _tightLeptons not the index of the original TBranch"
			<< " object. Correct that in the code and launch it again."
			<< std::endl;
		exit(-1);
	}

	// Recall index is the vector index
	return _tightLeptonTypes->at(index);
}
// No tight leptons
LeptonTypes LeptonMixingSelection::GetNoTightLeptonType(const unsigned int & index) const
{
	if( _samplemode != CutManager::FAKEABLESAMPLE )
	{
		std::cerr << "\033[1;31mLeptonMixingSelection::GetNoTightLeptonType ERROR\033[1;m "
			<< " Incoherent use of"
			<< " this function because it cannot be called in NORMALSAMPLE mode."
			<< " Check the client of this function why has been made this call"
			<< std::endl;
		exit(-1);
	}

	if( _notightLeptonTypes == 0 )  // Not needed
	{
		std::cerr << "\033[1;31mLeptonMixingSelection::GetNoTightLeptonType ERROR\033[1;m"
			<< " This function can not be used before calling the"
			<< " LeptonMixingSelection::GetNGoodIdLeptons, call it first!"
			<< std::endl;
		exit(-1);
	}
	if( index >= _notightLeptonTypes->size() )
	{
		std::cerr << "\033[1;31mLeptonMixingSelection::GetNoTightLeptonType ERROR\033[1;m"
			<< " The argument of this function must be the REAL VECTOR INDEX"
			<< " of the _notightLeptons not the index of the original TBranch"
			<< " object. Correct that in the code and launch it again."
			<< std::endl;
		exit(-1);
	}

	// Recall index is the vector index
	return _notightLeptonTypes->at(index);
}

// Wrapper function to evaluate cuts called directly from the client (Analysis class)
bool LeptonMixingSelection::IsPass(const std::string & codename, const std::vector<double> * varAux ) const
{
	// Checking
	// No distintion between the cuts accepted by muons that by electrons
	// use whatever you want to extract them
	if( this->fMuonSelection->_codenames.count( codename ) != 1 )
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
		if( varAux == 0 )
		{
			std::cerr << "\033[1;31mLeptonMixingSelection::IsPass ERROR\033[1;m "
				<< "Don't pass as second argument a vector<double> * "
				<< "which contains the number of muons and number of "
				<< "electrons must be selected."
				<< std::endl;
			exit(-1);
		}
		if( varAux->size() != 2 )
		{
			std::cerr << "\033[1;31mLeptonMixingSelection::IsPass ERROR\033[1;m "
				<< "Need exactly two elements in the second argument"
				<< " [vector<double> *] "
				<< "which contains the number of muons and number of "
				<< "electrons must be selected."
				<< std::endl;
			exit(-1);
		}
		ispass = this->IsPassPtCuts((int)(*varAux)[0],(int)(*varAux)[1]);
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


// Specific muon pt-cuts (for the good identified-isolated muons)
bool LeptonMixingSelection::IsPassPtCuts(const int & nwantMuons, const int & nwantElecs) const
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
	
	// SANITY CHECK: Filling the needed elements with the low cut
	for(unsigned int k = 3; k < _leptontypeGoodIdLeptons->size(); ++k)
	{
		vptcutMuon.push_back(fMuonSelection->kMinMuPt3);
		vptcutElec.push_back(fElecSelection->kMinMuPt3);
	}
	
	// Found the real pt used giving the lepton type
	std::vector<double> vptcut;
	std::vector<std::string> leptonstr;
	int nMuons = 0;
	int nElectrons = 0;
	for(unsigned int k = 0; k < _leptontypeGoodIdLeptons->size(); ++k)
	{
		if( (*_leptontypeGoodIdLeptons)[k] == MUON )
		{
			vptcut.push_back(vptcutMuon[k]);
			leptonstr.push_back("T_Muon_Pt");
			++nMuons;
		}
		else if( (*_leptontypeGoodIdLeptons)[k] == ELECTRON )
		{
			vptcut.push_back(vptcutElec[k]);
			leptonstr.push_back("T_Elec_Pt");
			++nElectrons;
		}
		else
		{
			std::cerr << "\033[1;31LeptonMixingSelection::IsPassPtCuts ERROR\033[1;m"
				<< " UNEXPECTED ERROR! Some inconsistency has been found, the code"
				<< " should be revisited, search around this line.\n"
				<< "cd to the CutManager package and use 'grep -R CODESEVERE .'"
				<< " Or contact me: jorge.duarte.campderros.AT.cern.ch"
				<< std::endl;
			exit(-2);
		}
	}

	int k = _selectedGoodIdLeptons->size()-1;
	// Ordered from higher to lower pt: begin from the lowest in order
	// to accomplish the old cut pt1 = 20 pt2 = 10 when you are dealing
	// with two leptons
        for(std::vector<int>::reverse_iterator it = _selectedGoodIdLeptons->rbegin(); 
			it != _selectedGoodIdLeptons->rend() ; ++it)
	{
		const unsigned int i = *it;
		const double ptcut = vptcut[k];
		if( _data->Get<float>(leptonstr[k].c_str(),i) < ptcut )
		{
			return false;
		}
		--k;
	}
	// Now check that we have the signature we want
	if( nMuons != nwantMuons && nElectrons != nwantElecs)  
	{
		return false;
	}

	// allright, all leptons passed their cuts
	return true;
}


// Helper function to find if an index is inside the vector of indices
bool LeptonMixingSelection::isfoundindex(const std::vector<int> * const leptonsvector, 
		const int & index) const
{
	if( std::find(leptonsvector->begin(),leptonsvector->end(), index ) != leptonsvector->end() )
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
		this->GetNBasicLeptons();
		//_selectedbasicLeptons = new std::vector<int>;
	}
	if( _leptontypebasicLeptons == 0 )
	{
		_leptontypebasicLeptons = new std::vector<LeptonTypes>;
	}
	_leptontypebasicLeptons->clear();  // As this function do not have reset!!!

	// Extract muons and electrons
	const int nselectedMuons = fMuonSelection->GetNBasicLeptons();
	const int nselectedElecs = fElecSelection->GetNBasicLeptons();
	
	// Be ready the notightLeptons if proceed
	if( _samplemode == CutManager::FAKEABLESAMPLE )
	{
		_notightLeptons = new std::vector<int>;
		_notightLeptonTypes = new std::vector<LeptonTypes>;
	}

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
		this->GetNLeptonsCloseToPV();
		//_closeToPVLeptons = new std::vector<int>;
	}
	if( _leptontypecloseToPVLeptons == 0 )
	{
		_leptontypecloseToPVLeptons = new std::vector<LeptonTypes>;
	}
	_leptontypecloseToPVLeptons->clear();

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
	fMuonSelection->GetNLeptonsCloseToPV();
	// Electrons
	fElecSelection->GetNLeptonsCloseToPV();

	//Loop over selected leptons
	for(unsigned int k = 0; k < _selectedbasicLeptons->size(); ++k)
	{
		unsigned int i = (*_selectedbasicLeptons)[k];
		LeptonTypes lepton = (*_leptontypebasicLeptons)[k];
		if( lepton == MUON )
		{
			if( ! isfoundindex(fMuonSelection->_closeToPVLeptons,i) )
			{
				if( _samplemode == CutManager::FAKEABLESAMPLE )
				{
					_notightLeptons->push_back(i);
					_notightLeptonTypes->push_back(lepton);
				}
				continue;
			}
		}
		else if( lepton == ELECTRON )
		{
			if( ! isfoundindex(fElecSelection->_closeToPVLeptons,i) )
			{
				if( _samplemode == CutManager::FAKEABLESAMPLE )
				{
					_notightLeptons->push_back(i);
					_notightLeptonTypes->push_back(lepton);
				}
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
		this->GetNIsoLeptons();
		//_selectedIsoLeptons = new std::vector<int>;
	}
	if( _leptontypeIsoLeptons == 0 )
	{
		_leptontypeIsoLeptons = new std::vector<LeptonTypes>;
	}

	//Empty the vector of indices --> Redundant
	//_selectedIsoLeptons->clear();
	_leptontypeIsoLeptons->clear();

	// First check is already was run over close to PV muons
	// if not do it
	if( _closeToPVLeptons == 0)
	{
		this->SelectLeptonsCloseToPV();
	}
	// Muons
	fMuonSelection->GetNIsoLeptons();
	// Electrons
	fElecSelection->GetNIsoLeptons();
	//Loop over selected muons
	for(unsigned int k = 0 ; k < _closeToPVLeptons->size(); ++k)
	{
		unsigned int i = (*_closeToPVLeptons)[k];
		LeptonTypes lepton = (*_leptontypecloseToPVLeptons)[k];
		// CAVEAT: here the algorithm is different w.r.t. last method,
		// because fMuonSelection->_selectedIsoLeptons and 
		// fElecSelection->_selectedIsoLeptons are composed by tights+notights
		if( lepton == MUON )
		{
			// --- First check: the leptons was lost: not tight neither no-tight
			if( ! isfoundindex(fMuonSelection->_selectedIsoLeptons, i) )
			{
				continue;
			}
				
			if( _samplemode == CutManager::FAKEABLESAMPLE )
			{
				// Otherwise, store the no-tight info 
				if( ! isfoundindex(fMuonSelection->_tightLeptons, i) )
				{
					_notightLeptons->push_back(i);
					_notightLeptonTypes->push_back(lepton);
					// And continue because the GetNIsoLepton function
					// already take into account to incorporate the notights
					// to the selected collection
					continue;
				}
			}
		}
		else if( lepton == ELECTRON )
		{
			// --- First check: the leptons was lost: not tight neither no-tight
			if( ! isfoundindex(fElecSelection->_selectedIsoLeptons, i) )
			{
				continue;
			}
			
			if( _samplemode == CutManager::FAKEABLESAMPLE )
			{
				// Otherwise, store the no-tight info
				if( ! isfoundindex(fElecSelection->_tightLeptons, i) )
				{
					_notightLeptons->push_back(i);
					_notightLeptonTypes->push_back(lepton);
					// And continue because the GetNIsoLepton function
					// already take into account to incorporate the notights
					// to the selected collection
					continue;
				}
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
		this->GetNGoodIdLeptons();
		//_selectedGoodIdLeptons = new std::vector<int>;
	}
	if( _leptontypeGoodIdLeptons == 0 )
	{
		_leptontypeGoodIdLeptons = new std::vector<LeptonTypes>;
	}

	//Empty the vector of indices --> Redundant
	//_selectedGoodIdLeptons->clear();
	_leptontypeGoodIdLeptons->clear();

	// First check is already was run over close Iso muons
	// if not do it
	if( _selectedIsoLeptons == 0)
	{
		this->SelectIsoLeptons();
	}

	// Before go on, the _tight leptons was introduced by GetNIsoLeptons, so 
	// the _tightLeptonTypes must be introduced also to keep track of the types
	if( this->IsInFakeableMode() )
	{
		_tightLeptonTypes =  new std::vector<LeptonTypes>;
		// Recall _selectedIsoLeptons = [ Tights1,..,TightsN, noTight1,...,noTightN],
		// so     _leptontypeIsoLeptons=[ T_type1,..,T_typeN, nT_type1,...,nT_typeN ]
		for(unsigned int k = 0; k < _tightLeptons->size(); ++k)
		{
			_tightLeptonTypes->push_back( (*_leptontypeIsoLeptons)[k] );
		}
	}
	// Muons
	fMuonSelection->GetNGoodIdLeptons();
	// Electrons
	fElecSelection->GetNGoodIdLeptons();

	//Loop over selected muons
	for(unsigned int k = 0 ; k < _selectedIsoLeptons->size(); ++k)
	{
		unsigned int i = (*_selectedIsoLeptons)[k];
		LeptonTypes lepton = (*_leptontypeIsoLeptons)[k];
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
	
	// Updating the tight, no-tight info if proceed
	if( this->IsInFakeableMode() )
	{
		this->UpdateFakeableCollections();
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
unsigned int LeptonMixingSelection::SelectLooseLeptons() 
{
	// First check is already was run over selected muons
	// if not do it
	if( _selectedbasicLeptons == 0)
	{
		this->SelectBasicLeptons();
	}

	// To keep track of the lepton type
	std::vector<int> selectedleptons    = *_selectedbasicLeptons;
	std::vector<LeptonTypes> leptontype = *_leptontypebasicLeptons;
	
	// Muons
	fMuonSelection->SelectLooseLeptons();
	// Electrons
	fElecSelection->SelectLooseLeptons();

	std::vector<int> tokeepIndex;
	std::vector<LeptonTypes> tokeepLeptonType;

	// What muons/electrons were lost?
	for(unsigned int k = 0; k < leptontype.size(); ++k)
	{
		unsigned int i = selectedleptons[k];
		LeptonTypes lepton = leptontype[k];
		if( lepton == MUON )
		{
			if( ! isfoundindex(fMuonSelection->_selectedbasicLeptons,i) )
			{
				continue;
			}
		}
		else if( lepton == ELECTRON )
		{
			if( ! isfoundindex(fElecSelection->_selectedbasicLeptons,i) )
			{
				continue;
			}
		}
		tokeepIndex.push_back(i);
		tokeepLeptonType.push_back(lepton);
	}

	// rebuilding the selected leptons, now are loose too
	_selectedbasicLeptons->clear();
	_leptontypebasicLeptons->clear();
	for(unsigned int k = 0; k < tokeepIndex.size(); ++k)
	{
		_selectedbasicLeptons->push_back( tokeepIndex[k] );
		_leptontypebasicLeptons->push_back( tokeepLeptonType[k] );
	}

	return _selectedbasicLeptons->size();
}


// Propagating the lepton type
void LeptonMixingSelection::SyncronizeLeptonType()
{
	//Loop over no tight muons
	for(unsigned int k = 0 ; k < _notightLeptons->size(); ++k)
	{
		LeptonTypes lepton = (*_notightLeptonTypes)[k];
		_leptontypeIsoLeptons->push_back(lepton);
	}
	// Already Updated the lepton type
}

// Particular method for the mixing channel, note it is overloaded
void LeptonMixingSelection::UpdateFakeableCollections()
{
	if( ! this->IsInFakeableMode() )
	{
		return;
	}

	if(_tightLeptons == 0 || _notightLeptons == 0)
	{
		std::cerr << "\033[1;31mCutManager::UpdateFakeableCollections ERROR\033[1;m" 
			<< "Incoherent use of this function" << std::endl;
		exit(-1);
	}

	std::vector<int> *notight = new std::vector<int>;
	std::vector<LeptonTypes> *notightLT = new std::vector<LeptonTypes>;
	for(unsigned int k = 0; k < _notightLeptonTypes->size(); ++k)
	{
		LeptonTypes lt =(* _notightLeptonTypes)[k];
		const int index = (*_notightLeptons)[k];
		CutManager * tmp = 0;
		if( lt == MUON )
		{
			tmp = fMuonSelection;
		}
		else
		{
			tmp = fElecSelection;
		}
		if( std::find(tmp->GetNoTightLeptons()->begin(),tmp->GetNoTightLeptons()->end(), index ) !=
				tmp->GetNoTightLeptons()->end() )
		{
			notight->push_back( index );
			notightLT->push_back( lt );
		}		
	}
	_notightLeptons->clear();
	*_notightLeptons = *notight;
	delete notight;
	notight = 0;
	_notightLeptonTypes->clear();
	*_notightLeptonTypes = *notightLT;
	delete notightLT;
	notightLT = 0;
	
	std::vector<int> *tight = new std::vector<int>;
	std::vector<LeptonTypes> *tightLT = new std::vector<LeptonTypes>;
	for(unsigned int k = 0; k < _tightLeptonTypes->size(); ++k)
	{
		LeptonTypes lt =(* _tightLeptonTypes)[k];
		const int index = (*_tightLeptons)[k];
		CutManager * tmp = 0;
		if( lt == MUON )
		{
			tmp = fMuonSelection;
		}
		else
		{
			tmp = fElecSelection;
		}
		if( std::find(tmp->GetTightLeptons()->begin(),tmp->GetTightLeptons()->end(), index ) !=
				tmp->GetTightLeptons()->end() )
		{
			tight->push_back( index );
			tightLT->push_back( lt );
		}
	}
	_tightLeptons->clear();
	*_tightLeptons = *tight;
	delete tight;
	tight=0;
	_tightLeptonTypes->clear();
	*_tightLeptonTypes = *tightLT;
	delete tightLT;
	tightLT = 0;
}	
