#include<assert.h>
#include<stdlib.h>

#include "CutManager.h"
#include "TreeManager.h"

CutManager::CutManager( TreeManager * data, const int & nTights, const int & nLeptons  ) :
	_data(data), 
	_cuts(0),
	_nLeptons(nLeptons),
	_samplemode(CutManager::NORMALSAMPLE),
	_nTights(-1),
	_nFails(-1),
	_selectedbasicLeptons(0),
	_closeToPVLeptons(0),
	_selectedIsoLeptons(0),
	_selectedGoodIdLeptons(0),
	_notightLeptons(0),
	_tightLeptons(0)
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

	if( _notightLeptons != 0 )
	{
		delete _notightLeptons;
		_notightLeptons = 0;
	}
	
	if( _tightLeptons != 0 )
	{
		delete _tightLeptons;
		_tightLeptons = 0;
	}

	if( _selectedbasicLeptons != 0)
	{
		delete _selectedbasicLeptons;
		_selectedbasicLeptons = 0;
	}
	if( _closeToPVLeptons != 0)
	{
		delete _closeToPVLeptons;
		_closeToPVLeptons = 0;
	}
	if( _selectedIsoLeptons != 0)
	{
		delete _selectedIsoLeptons;
		_selectedIsoLeptons = 0;
	}
	if( _selectedGoodIdLeptons != 0)
	{
		delete _selectedGoodIdLeptons;
		_selectedGoodIdLeptons = 0;
	}
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
	cuts.push_back("MaxIsoMu");  // OBSOLETE--> Now in regions
	//   - Quality and Identification
	cuts.push_back("MinNValidHitsSATrk");
	cuts.push_back("MaxNormChi2GTrk");
	cuts.push_back("MinNumOfMatches");
	cuts.push_back("MinNValidPixelHitsInTrk");
	cuts.push_back("MinNValidHitsInTrk");
	cuts.push_back("MaxDeltaPtMuOverPtMu");
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
	if( _notightLeptons != 0 )
	{
		delete _notightLeptons;
		_notightLeptons = 0;
	}
	
	if( _tightLeptons != 0 )
	{
		delete _tightLeptons;
		_tightLeptons = 0;
	}

	if( _selectedbasicLeptons != 0)
	{
		delete _selectedbasicLeptons;
		_selectedbasicLeptons = 0;
	}
	if( _closeToPVLeptons != 0)
	{
		delete _closeToPVLeptons;
		_closeToPVLeptons = 0;
	}
	if( _selectedIsoLeptons != 0)
	{
		delete _selectedIsoLeptons;
		_selectedIsoLeptons = 0;
	}
	if( _selectedGoodIdLeptons != 0)
	{
		delete _selectedGoodIdLeptons;
		_selectedGoodIdLeptons = 0;
	}
}

//
unsigned int CutManager::GetNBasicLeptons()
{
	int size = 0;
	if( _selectedbasicLeptons == 0)
	{
		size = this->SelectBasicLeptons();
	}
	else
	{
		size = _selectedbasicLeptons->size();
	}

	// fake mode: the _selectedbasicLeptons now
	// are loose leptons
	if( _samplemode == CutManager::FAKEABLESAMPLE )
	{
		size = this->SelectLooseLeptons();
	}

	return size;
}

unsigned int CutManager::GetNLeptonsCloseToPV()
{
	int size = 0;
	if( _closeToPVLeptons == 0)
	{
		size = this->SelectLeptonsCloseToPV();
	}
	else
	{
		size = _closeToPVLeptons->size();
	}

	return size;
}

unsigned int CutManager::GetNIsoLeptons()
{
	int size = 0;
	if( _selectedIsoLeptons == 0)
	{
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
		size = this->SelectGoodIdLeptons();
	}
	else
	{
		size = _selectedGoodIdLeptons->size();
	}
	// If we are at fake mode, at this stage it is need
	// to separate the tight and no tight leptons but
	// the _selectedGoodIdLeptons have tight+notights
	if( this->IsInFakeableMode() )
	{
		// Build the tight
		_tightLeptons = new std::vector<int>;
		for(unsigned int i = 0; i < size; ++i)
		{
			_tightLeptons->push_back( _selectedGoodIdLeptons->at(i) );
		}
		// Plus the notight
		const unsigned int notightsize = _notightLeptons->size();
		for(unsigned int i = 0; i < notightsize; ++i)
		{
			_selectedGoodIdLeptons->push_back( _notightLeptons->at(i) );
		}
		// Keep track of the lepton type if proceed (mixing classes)
		this->SyncronizeLeptonType();
		// the total size is
		size += notightsize;		
	}

	return size;
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

// Extract the Index (in the data) of the i-essim no Tight lepton
const unsigned int CutManager::GetNoTightIndex(const unsigned int & i) const
{
	if( _samplemode != CutManager::FAKEABLESAMPLE )
	{
		std::cerr << "\033[1;31mCutManager::GetNoTightPt ERROR\033[1;m Incoherent use of"
			<< " this function because it cannot be called in NORMALSAMPLE mode."
			<< " Check the client of this function why has been made this call"
			<< std::endl;
		exit(-1);
	}

	if( _notightLeptons->size() <= i )
	{
		std::cerr << "\033[1;31mCutManager::GetNoTightPt ERROR\033[1;m Overbounded, "
			<< "there are '" << _notightLeptons->size() << "' noTight leptons."
			<< std::endl;
		exit(-1);
	}

	return (*_notightLeptons)[i];
}

