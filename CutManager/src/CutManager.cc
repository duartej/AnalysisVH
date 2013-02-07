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
	_modifypt(false),
	_smu(1.0),
	_sebr(1.0),
	_see(1.0),
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
	if( this->_modifypt )
	{
		for(std::vector<LeptonRel>::iterator it = _selectedbasicLeptons->begin();
				it != _selectedbasicLeptons->end(); ++it)
		{
			double f = 1.0;
			switch(it->leptontype())
			{
				case(MUON):
					f = this->_smu;
					break;
				case(ELECTRON):
					if( fabs(it->getP4().Eta()) < 1.479 )
				        {
						f = this->_sebr;
					}
					else
					{
						f = this->_see;
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
	// If we are at fake mode, at this stage it is need
	// to separate the tight and no tight leptons but
	// the _selectedGoodIdLeptons have tight+notights
	if( this->IsInFakeableMode() )
	{
		// Build the tight
		_tightLeptons = new std::vector<LeptonRel>;
		_registeredcols->push_back(&_tightLeptons);
		for(unsigned int i = 0; i < size; ++i)
		{
			_tightLeptons->push_back( _selectedIsoLeptons->at(i) );
		}
		// Plus the notight (if we're don't using all the nLeptons as tights
		if( _nTights != _nLeptons )
		{
			const unsigned int notightsize = _notightLeptons->size();
			for(unsigned int i = 0; i < notightsize; ++i)
			{
				_selectedIsoLeptons->push_back( _notightLeptons->at(i) );
			}
			// Keep track of the lepton type if proceed (mixing classes)
			// Is at this level when _selected Vector has the tight, notight
			// merged collection
			//this->SyncronizeLeptonType();  //-- TO BE DEPRECATED??
			
			size += notightsize;		
		}
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
	// If we are at fake mode, at this stage it is need
	// to update the tight and no tight collections with the new
	// results 
	if( this->IsInFakeableMode() )
	{
		this->UpdateFakeableCollections(_selectedGoodIdLeptons);
	}

	return size;
}

// Scale factor to be applied to lepton pt (momentum/energy scale systematic)
void CutManager::SetPtSystematicFactor(const double & smu, 
		const double & sebarrel, const double & seendcap)
{
	_modifypt = true;
	_smu = smu;
	_sebr= sebarrel;
	_see = seendcap;
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
		return ( (_tightLeptons->size() == _nTights) &&
				(_notightLeptons->size() == _nFails) );
	}
	else
	{
		return (_selectedGoodIdLeptons->size() == _nLeptons);
	}

}


//
bool CutManager::IspassAtLeastN()
{
	// Note that must be at the stage of Good ID
	assert( (_selectedGoodIdLeptons != 0) && 
			"The CutManager::IspassAtLeastN method must be called AFTER CutManager::GetNGoodIdLeptons" );
	if( this->IsInFakeableMode() )
	{
		return ( (_tightLeptons->size() >= _nTights) &&
			(_notightLeptons->size() >= _nFails) );		
	}
	else
	{
		return (_selectedGoodIdLeptons->size() >= _nLeptons);
	}
}

//Overloaded method: it is allowed any combination of Tight, Loose
bool CutManager::IspassAtLeastN(const unsigned int & nLeptons,const unsigned int & nTights)
{
	unsigned int size = nTights;

	if( _samplemode == CutManager::FAKEABLESAMPLE )
	{
		assert( (_notightLeptons != 0) &&
				"CutManager::IspassAtLeastN called and it doesn't make sense to called it at this stage" );
		size += _notightLeptons->size();
	}

	return size >= nLeptons;
}

// Update the tight and no tight collection, the vector introduced as argument
// contains the final result:  [ tight1,...,tightN,notight1,..., notightN]
void CutManager::UpdateFakeableCollections( const std::vector<LeptonRel> * finalcol)
{
	if( ! this->IsInFakeableMode() )
	{
		return;
	}

	if( finalcol == 0 || _tightLeptons == 0 || _notightLeptons == 0)
	{
		std::cerr << "\033[1;31mCutManager::UpdateFakeableCollections ERROR\033[1;m" 
			<< "Incoherent use of this function" << std::endl;
		exit(-1);
	}

	// Mixing channel: already done
	if( this->WasAlreadyUpdated() )
	{
		return;
	}

	std::vector<LeptonRel> *tight = new std::vector<LeptonRel>;
	std::vector<LeptonRel> *notight = new std::vector<LeptonRel>;
	for(std::vector<LeptonRel>::const_iterator it = finalcol->begin(); it != finalcol->end(); ++it)
	{
		if( std::find(_tightLeptons->begin(),_tightLeptons->end(), *it) != 
				_tightLeptons->end() )
		{
			tight->push_back( *it );
			continue;
		}
		if( std::find(_notightLeptons->begin(),_notightLeptons->end(), *it) != 
				_notightLeptons->end() )
		{
			notight->push_back( *it );
			continue; 
		}
	}

	_tightLeptons->clear();
	*_tightLeptons = *tight;
	
	_notightLeptons->clear();
	*_notightLeptons = *notight;

	delete tight;
	delete notight;
}


