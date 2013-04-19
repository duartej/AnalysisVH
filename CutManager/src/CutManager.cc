#include<assert.h>
#include<stdlib.h>
#include<algorithm>

#include "CutManager.h"
#include "TreeManager.h"

CutManager::CutManager( TreeManager * data, const int & nTights, const int & nLeptons,
		const char * runperiod  ) :
	_data(data), 
	_runperiod(runperiod),
	_cuts(0),
	_nLeptons(nLeptons),
	_samplemode(CutManager::NORMALSAMPLE),
	_nTights(-1),
	_nFails(-1),
	_modifypt(new bool(false)),
	_smu(new double(1.0)),
	_sebr(new double(1.0)),
	_see(new double (1.0)),
	_selectedbasicLeptons(0),
	_closeToPVLeptons(0),
	_selectedIsoLeptons(0),
	_selectedGoodIdLeptons(0),
	_notightLeptons(0),
	_tightLeptons(0),
	_registeredcols(new std::vector<std::vector<LeptonRel> **>)
{
	_cuts = new std::map<std::string,double>;

	// Fakeable mode
	if( nTights >= 0 )
	{
		_samplemode = CutManager::FAKEABLESAMPLE;
		_nTights = nTights;
		_nFails = _nLeptons - _nTights;
		// Coherent check
		assert( (_nTights <= _nLeptons) && "The number of tights leptons MUST BE lesser"
				" or equal than the number of total leptons" );
	}
}

CutManager::~CutManager()
{
	// Freeing memory
	if( _cuts != 0 )
	{
		delete _cuts;
		_cuts = 0;
	}
	
	// All the others are subsets of this one
	for(std::vector<std::vector<LeptonRel> **>::iterator it = _registeredcols->begin();
			it != _registeredcols->end(); ++it)
	{
		if( *(*it) != 0 )
		{
			delete *(*it);
			*(*it) = 0;
		}
	}

	delete _registeredcols;
	_registeredcols = 0;

	delete _modifypt;
	_modifypt = 0;

	delete _smu;
	_smu = 0;

	delete _sebr;
	_sebr = 0;

	delete _see;
	_see = 0;
}


// Es una funcion de esta classe quien se deberia encargar de esto o es
// mejor que lo haga la clase analisis? Depende de lo generales que 
// queremos que sean los corte...
void CutManager::InitialiseCuts(const std::map<LeptonTypes,InputParameters*> & ipmap)
{
	//---- FIXME: Recupera las explicaciones
	std::vector<std::string> cuts;
	//   - Pt and Eta of muons
	cuts.push_back("MinMuPt1");
	cuts.push_back("MinMuPt2");
	cuts.push_back("MinMuPt3");
	cuts.push_back("MaxAbsEta");
	//   - IP and DeltaZ of track associated with muon w.r.t PV
	cuts.push_back("MaxMuIP2DInTrackR1");
	cuts.push_back("MaxMuIP2DInTrackR2");
	cuts.push_back("MaxDeltaZMu") ;
  	//   - Isolation: (PTtraks + ETcalo)/PTmuon: different regions
	cuts.push_back("MaxPTIsolationR1");
	cuts.push_back("MaxPTIsolationR2");
	cuts.push_back("MaxPTIsolationR3");
	cuts.push_back("MaxPTIsolationR4");
	//   - Quality and Identification
	cuts.push_back("MinNValidHitsSATrk");
	cuts.push_back("MaxNormChi2GTrk");
	cuts.push_back("MinNumOfMatches");
	cuts.push_back("MinNValidPixelHitsInTrk");
	cuts.push_back("MinNValidHitsInTrk");
	cuts.push_back("MinNLayers");
	cuts.push_back("MaxDeltaPtMuOverPtMu");
	//   - BDT electron
	cuts.push_back("MinMVAValueR1");
	cuts.push_back("MinMVAValueR2");
	cuts.push_back("MinMVAValueR3");
	cuts.push_back("MinMVAValueR4");
	cuts.push_back("MinMVAValueR5");
	cuts.push_back("MinMVAValueR6");
	// Loose electrons (trigger id and iso)
	cuts.push_back("MaxSigmaietaietaR1");  
	cuts.push_back("MaxdeltaPhiInR1"); 
	cuts.push_back("MaxdeltaEtaInR1"); 
	cuts.push_back("MaxHtoER1"); 
	cuts.push_back("MaxSigmaietaietaR2"); 
	cuts.push_back("MaxdeltaPhiInR2"); 
	cuts.push_back("MaxdeltaEtaInR2"); 
	cuts.push_back("MaxHtoER2"); 
	cuts.push_back("Maxdr03TkSumPtOverPt"); 
	cuts.push_back("Maxdr03EcalSumPtOverPt"); 
	cuts.push_back("Maxdr03HcalSumPtOverPt"); 

	//   - Max DeltaR between muons
	cuts.push_back("MaxDeltaRMuMu");
  	//   - Min MET of the event
	cuts.push_back("MinMET");

	// Fakeable mode
	if( this->IsInFakeableMode() )
	{
		cuts.push_back( "MaxLoosed0" );
		cuts.push_back( "MaxLooseIso" );
	}

	// Now including all the cuts to the manager
	this->LockCuts(ipmap,cuts);
}

// Method to be called each time finalize a entry
void CutManager::Reset()
{
	for(std::vector<std::vector<LeptonRel> **>::iterator it = _registeredcols->begin();
			it != _registeredcols->end(); ++it)
	{
		if( *(*it) != 0 )
		{
			delete *(*it);
			*(*it) = 0;
		}
	}

	_registeredcols->clear();
}

//
unsigned int CutManager::GetNBasicLeptons()
{
	int size = 0;
	if( _selectedbasicLeptons == 0)
	{
		_selectedbasicLeptons = new std::vector<LeptonRel>;
		_registeredcols->push_back(&_selectedbasicLeptons);
		size = this->SelectBasicLeptons();
	}
	else
	{
		size = _selectedbasicLeptons->size();
	}

	// fake mode: the _selectedbasicLeptons now
	// are loose leptons
	// AND (_nTights != _nLeptons): Patch to fix different 
	// behaviour when dealing with a regular analysis and 
	// a data-driven -F N,N  i.e. the same number of tights
	// than the number of leptons analyzed (see issue #56 
	// (https://github.com/duartej/AnalysisVH/issues/56)
	if( _samplemode == CutManager::FAKEABLESAMPLE && (_nTights != _nLeptons) )
	{
		size = this->SelectLooseLeptons();
	}

	// Putting the pt-scale when systematics
	if( *(this->_modifypt) )
	{
		for(std::vector<LeptonRel>::iterator it = _selectedbasicLeptons->begin();
				it != _selectedbasicLeptons->end(); ++it)
		{
			double f = 1.0;
			switch(it->leptontype())
			{
				case(MUON):
					f = *(this->_smu);
					break;
				case(ELECTRON):
					if( fabs(it->getP4().Eta()) < 1.479 )
				        {
						f = *(this->_sebr);
					}
					else
					{
						f = *(this->_see);
					}
					break;
				default:
					std::cerr << "\033[1;31mCutManager::GetNBasicLeptons ERROR\033[1;m" 
						<< "The leptontype is not known! Exiting..." << std::endl;
					exit(-1);
			}
			it->setScale(f);
		}
	}

	return size;
}

unsigned int CutManager::GetNLeptonsCloseToPV()
{
	int size = 0;
	if( _closeToPVLeptons == 0)
	{
		_closeToPVLeptons = new std::vector<LeptonRel>;
		_registeredcols->push_back(&_closeToPVLeptons);
		size = this->SelectLeptonsCloseToPV();
	}
	else
	{
		size = _closeToPVLeptons->size();
	}

	return size;
}

// Return tight + no tight (if Proceed)
unsigned int CutManager::GetNIsoLeptons()
{
	unsigned int size = 0;
	if( _selectedIsoLeptons == 0)
	{
		_selectedIsoLeptons = new std::vector<LeptonRel>;
		_registeredcols->push_back(&_selectedIsoLeptons);
		size = this->SelectIsoLeptons();
	}
	else
	{
		size = _selectedIsoLeptons->size();
	}

	return size;
}

unsigned int CutManager::GetNGoodIdLeptons()
{
	unsigned int size = 0;
	if( _selectedGoodIdLeptons == 0)
	{
		_selectedGoodIdLeptons = new std::vector<LeptonRel>;
		_registeredcols->push_back(&_selectedGoodIdLeptons);
		size = this->SelectGoodIdLeptons();
	}
	else
	{
		size = _selectedGoodIdLeptons->size();
	}

	return size;
}

//! Get the Tight leptons (passing the PV, Iso and ID cuts)
std::vector<LeptonRel> * CutManager::GetTightLeptons()
{
	// Not in fakeable mode, tight are the total leptons
	// of the analysis
	if( ! this->IsInFakeableMode() )
	{
		return _selectedGoodIdLeptons;
	}

	// Check we have the selected good, it has no sense instead
	if( _selectedGoodIdLeptons == 0 )
	{
		std::cerr << "\033[1;31mCutManager::GetTightLeptons ERROR\033[1;m" 
			<< " Function called before selected the Good Id leptons" 
			<< " Exiting..." << std::endl;
		exit(-1);
	}

	if( _tightLeptons != 0 )
	{
		return _tightLeptons;
	}
	
	// Built the vector the first time you call this function
	_tightLeptons = new std::vector<LeptonRel>;
	_registeredcols->push_back(&_tightLeptons);

	for(std::vector<LeptonRel>::const_iterator it = _selectedGoodIdLeptons->begin();
			it != _selectedGoodIdLeptons->end(); ++it)
	{
		if( it->category() != LeptonRel::TIGHT )
		{
			continue;
		}
		_tightLeptons->push_back(*it);
	}

	return _tightLeptons;
}

//! Get the noTight leptons (not passing the PV, Iso and ID cuts)
std::vector<LeptonRel> * CutManager::GetNoTightLeptons()
{
	// Not in fakeable mode, no sense calling this function
	if( ! this->IsInFakeableMode() )
	{
		std::cerr << "\033[1;31mCutManager::GetNoTightLeptons ERROR\033[1;m" 
			<< " No in Fakeable mode, so there are no loose defined." 
			<< " Exiting..." << std::endl;
		exit(-1);
	}
	// Check we have the selected good, it has no sense instead
	if( _selectedGoodIdLeptons == 0 )
	{
		std::cerr << "\033[1;31mCutManager::GetNoTightLeptons ERROR\033[1;m" 
			<< " Function called before selected the Good Id leptons" 
			<< " Exiting..." << std::endl;
		exit(-1);
	}

	if( _notightLeptons != 0 )
	{
		return _notightLeptons;
	}

	// Built the vector the first time you call this function
	_notightLeptons = new std::vector<LeptonRel>;
	_registeredcols->push_back(&_notightLeptons);

	for(std::vector<LeptonRel>::const_iterator it = _selectedGoodIdLeptons->begin();
			it != _selectedGoodIdLeptons->end(); ++it)
	{
		if( it->category() != LeptonRel::FAIL )
		{
			continue;
		}
		_notightLeptons->push_back(*it);
	}

	return _notightLeptons;
}


// Scale factor to be applied to lepton pt (momentum/energy scale systematic)
void CutManager::SetPtSystematicFactor(const double & smu, 
		const double & sebarrel, const double & seendcap)
{
	*(this->_modifypt) = true;
	*(this->_smu) = smu;
	*(this->_sebr)= sebarrel;
	*(this->_see) = seendcap;
}


// Setters
void CutManager::SetCut(const std::string & cutname, const double & value)
{
	//FIXME: Check the field is already not used
	(*_cuts)[cutname] = value;
}


// 
bool CutManager::IspassExactlyN()
{
	// Note that must be at the stage of Good ID
	assert( (_selectedGoodIdLeptons != 0) && "\033[1;31mThe CutManager::IspassExactlyN method"
			" must be called AFTER CutManager::GetNGoodIdLeptons" );

	if( this->IsInFakeableMode() )
	{
		return ( (this->GetNTightMeasured() == _nTights) &&
				(this->GetNFailMeasured() == _nFails) );
	}
	else
	{
		return (_selectedGoodIdLeptons->size() == _nLeptons);
	}

}


//Check it is pass at least the wanted leptons, but after selectedGoodIdLeptons
bool CutManager::IspassAtLeastN()
{
	// Note that must be at the stage of Good ID
	assert( (_selectedGoodIdLeptons != 0) && 
			"The CutManager::IspassAtLeastN method must be called AFTER CutManager::GetNGoodIdLeptons" );
	if( this->IsInFakeableMode() )
	{
		return ( (this->GetNTightMeasured() >= _nTights) &&
			(this->GetNFailMeasured() >= _nFails) );		
	}
	else
	{
		return (_selectedGoodIdLeptons->size() >= _nLeptons);
	}
}

//Overloaded method: TO BE DEPRECATED: Now the _selected* collections
//contain the full number of (tight+notight) lepton, so do not need this function
bool CutManager::IspassAtLeastN(const unsigned int & nWantedPass,const unsigned int & nCheckToPass)
{
	return nCheckToPass >= nWantedPass;
}

