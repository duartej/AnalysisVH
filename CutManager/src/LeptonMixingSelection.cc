
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
	fElecSelection(0)
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
	fElecSelection(0)
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
	
}

void LeptonMixingSelection::Reset()
{
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
		ispass = this->fMuonSelection->IsInsideZWindow((*varAux)[0]);
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
	// FIXME: Change this to a vector of cuts, otherwise the number of leptons
	// is hardcoded!
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
	for(unsigned int k = 3; k < _selectedGoodIdLeptons->size(); ++k)
	{
		vptcutMuon.push_back(fMuonSelection->kMinMuPt3);
		vptcutElec.push_back(fElecSelection->kMinMuPt3);
	}

	// Ordered from higher to lower pt: begin from the lowest in order
	// to accomplish the old cut pt1 = 20 pt2 = 10 when you are dealing
	// with two leptons
	// FIXME: A potential bug in here , hardcoded number of ptcuts (3) against
	// not hardcoded number of total leptons in the analysis!!
	int k = _selectedGoodIdLeptons->size()-1;
	int nMuons = 0;
	int nElectrons = 0;
	for(std::vector<LeptonRel>::reverse_iterator it = _selectedGoodIdLeptons->rbegin();
			it != _selectedGoodIdLeptons->rend(); ++it)
	{
		double ptcut = 0.0;
		if( it->leptontype() == MUON )
		{
			ptcut = vptcutMuon[k];
			++nMuons;
		}
		else if( it->leptontype() == ELECTRON )
		{
			ptcut = vptcutElec[k];
			++nElectrons;
		}

		if( it->getP4().Pt() < ptcut )
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
bool LeptonMixingSelection::isfound(const LeptonRel & lepton, 
		const std::vector<LeptonRel> * const leptonsvector) const 
{
	if( std::find(leptonsvector->begin(),leptonsvector->end(), lepton ) != leptonsvector->end() )
	{
		return true;
	}
		
	return false;
}


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
	}

	// Extract muons and electrons
	const int nselectedMuons = fMuonSelection->GetNBasicLeptons();
	const int nselectedElecs = fElecSelection->GetNBasicLeptons();
	
	// Be ready the notightLeptons if proceed
	if( _samplemode == CutManager::FAKEABLESAMPLE )
	{
		_notightLeptons = new std::vector<LeptonRel>;
		_registeredcols->push_back(&_notightLeptons);
	}

	// ordering by Pt
	// -- Muons
	std::map<double,LeptonRel> ordermap;
	for(int i = 0 ; i < nselectedMuons; ++i)
	{
		const LeptonRel & mu = fMuonSelection->_selectedbasicLeptons->at(i);
		ordermap[mu.getP4().Pt()] = mu;
	}
	// -- Electrons
	for(int i = 0 ; i < nselectedElecs; ++i)
	{
		const LeptonRel & elec = fElecSelection->_selectedbasicLeptons->at(i);
		ordermap[elec.getP4().Pt()] = elec;
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
	for(std::map<double,LeptonRel>::reverse_iterator it = ordermap.rbegin();
			it != ordermap.rend(); ++it)
	{
		_selectedbasicLeptons->push_back( it->second );
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
	fMuonSelection->GetNLeptonsCloseToPV();
	// Electrons
	fElecSelection->GetNLeptonsCloseToPV();

	//Loop over selected leptons
	for(std::vector<LeptonRel>::iterator it = _selectedbasicLeptons->begin(); 
			it != _selectedbasicLeptons->end(); ++it)
	{
		if( it->leptontype() == MUON )
		{
			if( ! isfound(*it,fMuonSelection->_closeToPVLeptons) )
			{
				if( _samplemode == CutManager::FAKEABLESAMPLE )
				{
					_notightLeptons->push_back(*it);
				}
				continue;
			}
		}
		else if( it->leptontype() == ELECTRON )
		{
			if( ! isfound(*it,fElecSelection->_closeToPVLeptons) )
			{
				if( _samplemode == CutManager::FAKEABLESAMPLE )
				{
					_notightLeptons->push_back(*it);
				}
				continue;
			}
		}

		_closeToPVLeptons->push_back(*it);
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
	//Loop over selected leptons
for(unsigned int i = 0; i < _closeToPVLeptons->size(); ++i)
{
} 
	for(std::vector<LeptonRel>::iterator it = _closeToPVLeptons->begin(); 
			it != _closeToPVLeptons->end(); ++it)
	{
		// CAVEAT: here the algorithm is different w.r.t. last method,
		// because fMuonSelection->_selectedIsoLeptons and 
		// fElecSelection->_selectedIsoLeptons are composed by tights+notights
		if( it->leptontype() == MUON )
		{
			// --- First check: the leptons was lost: not tight neither no-tight
			if( ! isfound(*it,fMuonSelection->_selectedIsoLeptons) )
			{
				continue;
			}
			
			if( _samplemode == CutManager::FAKEABLESAMPLE )
			{
				// Otherwise, store the no-tight info 
				if( ! isfound(*it,fMuonSelection->_tightLeptons) )
				{
					_notightLeptons->push_back(*it);
					// And continue because the GetNIsoLepton function
					// already take into account to incorporate the notights
					// to the selected collection
					continue;
				}
			}
		}
		else if( it->leptontype() == ELECTRON )
		{
			// --- First check: the leptons was lost: not tight neither no-tight
			if( ! isfound(*it,fElecSelection->_selectedIsoLeptons) )
			{
				continue;
			}

			if( _samplemode == CutManager::FAKEABLESAMPLE )
			{
				// Otherwise, store the no-tight info
				if( ! isfound(*it,fElecSelection->_tightLeptons) )
				{
					_notightLeptons->push_back(*it);
					// And continue because the GetNIsoLepton function
					// already take into account to incorporate the notights
					// to the selected collection
					continue;
				}
			}
		}
		_selectedIsoLeptons->push_back(*it);
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
	}

	// First check is already was run over close Iso muons
	// if not do it
	if( _selectedIsoLeptons == 0)
	{
		this->SelectIsoLeptons();
	}

	// Muons
	fMuonSelection->GetNGoodIdLeptons();
	// Electrons
	fElecSelection->GetNGoodIdLeptons();
	
	//Loop over selected muons
	for(std::vector<LeptonRel>::iterator it = _selectedIsoLeptons->begin(); 
			it != _selectedIsoLeptons->end(); ++it)
	{
		if( it->leptontype() == MUON )
		{
			if( ! isfound(*it,fMuonSelection->_selectedGoodIdLeptons) )
			{
				continue;
			}
		}
		else if( it->leptontype() == ELECTRON )
		{
			if( ! isfound(*it,fElecSelection->_selectedGoodIdLeptons) )
			{
				continue;
			}			
		}
		
		// Update the charges
		it->setcharge(_data->Get<int>(std::string("T_"+it->leptonname()+"_Charge").c_str(),it->index()));
		_selectedGoodIdLeptons->push_back(*it);
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
	
	// Muons
	fMuonSelection->SelectLooseLeptons();
	// Electrons
	fElecSelection->SelectLooseLeptons();

	// leptons to be stored
	std::vector<LeptonRel> tokeep;

	// What muons/electrons were lost?
	for(std::vector<LeptonRel>::iterator it = _selectedbasicLeptons->begin(); 
			it != _selectedbasicLeptons->end(); ++it)
	{
		if( it->leptontype() == MUON )
		{
			if( ! isfound(*it,fMuonSelection->_selectedbasicLeptons) )
			{
				continue;
			}
		}
		else if( it->leptontype() == ELECTRON )
		{
			if( ! isfound(*it,fElecSelection->_selectedbasicLeptons) )
			{
				continue;
			}
		}
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

	std::vector<LeptonRel> *notight = new std::vector<LeptonRel>;
	for(std::vector<LeptonRel>::iterator it = _notightLeptons->begin(); 
			it != _notightLeptons->end(); ++it)
	{
		CutManager * tmp = 0;
		if( it->leptontype() == MUON )
		{
			tmp = fMuonSelection;
		}
		else
		{
			tmp = fElecSelection;
		}

		if( std::find(tmp->GetNoTightLeptons()->begin(),tmp->GetNoTightLeptons()->end(), *it ) !=
				tmp->GetNoTightLeptons()->end() )
		{
			notight->push_back( *it );
		}		
	}
	_notightLeptons->clear();
	*_notightLeptons = *notight;
	delete notight;
	notight = 0;
	
	std::vector<LeptonRel> *tight = new std::vector<LeptonRel>;
	for(std::vector<LeptonRel>::iterator it = _tightLeptons->begin(); 
			it != _tightLeptons->end(); ++it)
	{
		CutManager * tmp = 0;
		if( it->leptontype() == MUON )
		{
			tmp = fMuonSelection;
		}
		else
		{
			tmp = fElecSelection;
		}
		if( std::find(tmp->GetTightLeptons()->begin(),tmp->GetTightLeptons()->end(), *it ) !=
				tmp->GetTightLeptons()->end() )
		{
			tight->push_back( *it );
		}
	}
	_tightLeptons->clear();
	*_tightLeptons = *tight;
	delete tight;
	tight=0;
}	
